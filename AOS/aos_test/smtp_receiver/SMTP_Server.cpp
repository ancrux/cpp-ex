#include "SMTP_Server.h"

#include "aos/net/smtp/SMTP_Server_IO.h"

//namespace aos {

SMTP_Server::SMTP_Server()
:
max_conn_(MAX_CONN+1),
shutdown_(0)
{
}

SMTP_Server::~SMTP_Server()
{
}

int
SMTP_Server::open(const ACE_Addr& addr)
{
	this->enable();
	if ( acceptor_.open(addr) == -1 )
		return -1; // cannot bind on the server_addr

	return 0;
}

int
SMTP_Server::close()
{
	this->disable();
	acceptor_.close();

	return 0;
}

int
SMTP_Server::start()
{
	this->enable();
	this->activate(THR_NEW_LWP | THR_DETACHED, 1, 0); // create detached 

	return 0;
}

int
SMTP_Server::stop(const ACE_Time_Value* timeout)
{
	this->disable();
	this->thr_mgr()->wait(timeout);

	return 0;
}

int
SMTP_Server::svc()
{
	//ACE_OS::printf("(%d) started...\n", ACE_OS::thr_self());
	if ( shutdown_ == 1 )
		return 0;

#if USE_SSL == 1
	aos::SSL_Socket_Stream stream; //ACE_SSL_SOCK_Stream stream;
#else
	aos::Socket_Stream stream; //ACE_SOCK_Stream stream;
#endif
	ACE_Time_Value timeout(30);
	int flags = 0;

	int n_accept_sec = 1;
	ACE_Time_Value accept_timeout(n_accept_sec, 0);
	ACE_INET_Addr client_addr;
	while( acceptor_.accept(stream, &client_addr, &accept_timeout) == -1 )
	{
		if ( shutdown_ == 1 )
			break;

		accept_timeout.set(n_accept_sec, 0); // must reset accept_timeout for SSL accept()
		int error = ACE_OS::last_error(); // 10060 == ETIMEDOUT, 10035 == EWOULDBLOCK, 10054 == ECONNRESET
		if ( error == ETIMEDOUT )
			continue;
		//ACE_OS::printf("(%d) accept() error: %d\n", ACE_OS::thr_self(),  error);

		// other error, either spawn another listener,
		// or new another SSL_Stream, don't use the SSL_Stream
		this->activate(THR_NEW_LWP | THR_DETACHED, 1, 1);
		return 0;
	}
	if ( shutdown_ == 1 )
	{
		// response server is temporarily unavailable!
		stream.close();
		return 0;
	}

	this->activate(THR_NEW_LWP | THR_DETACHED, 1, 1); // append new detached thread as listener
	//ACE_OS::printf("(%d) new thread created: %d!\n", ACE_OS::thr_self(), this->thr_count());

	//if ( this->thr_count() > MAX_CONN+1 )
	if ( this->thr_count() > max_conn_.value() )
	{
		// response server is temporarily unavailable!
		ACE_OS::printf("(%d) max_connection=%d reached!\n", ACE_OS::thr_self(), this->max_conn());
		stream.close();
		return 0;
	}

	//if ( accept_stream.start_ssl(stream, 1) <= 0 )
	//	stream.stop_ssl();

	//ACE_TCHAR addr[MAXHOSTNAMELEN];
	//client_addr.addr_to_string(addr, MAXHOSTNAMELEN);
	//ACE_OS::printf("%s\n", addr);

	ACE_Message_Block mb(4096); // read/write buffer
	SMTP_Server_IO io;
	io.open();

	ssize_t n_recv = -1;
	ssize_t n_send = -1;

	// write greetings
	io.greetings(mb);
	if ( io.write_state() == SMTP_Server_IO::WR_OK )
	{
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
		if ( n_send < 1 ) ACE_OS::printf("send() failed!\n"); //@
	}
	else ACE_OS::printf("insufficient send buffer!\n"); //@
	
	// enter read/write loop
	if ( n_send > 0 ) // greetings sent!
	{
		do
		{
			//// read command

			io.read_reset(mb);
			while( io.read_state() != SMTP_Server_IO::RD_OK )
			{
				// buffer consumed, read more...
				if ( mb.length() == 0 )
				{
					mb.reset();
					n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
					if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
						break;
					else
						mb.wr_ptr(n_recv);
				}
				// process buffer
				if ( mb.length() > 0 )
				{
					if ( io.state() != SMTP_Server_IO::State::DATA )
					{
						aos::bcstr line = io.read_line(mb);
					}
					else
					{
						aos::bcstr data = io.read_data(mb);
						//std::string out(data.buf, data.len);
						//::printf("%s", out.c_str());
					}
				}
			}
			if ( n_recv < 1 )
			{
				ACE_OS::printf("recv() failed\n"); // read failed, close connection
				break;
			}

			//// write response

			int rc = io.exec_cmd(mb); // parse & exec command
			if ( io.write_state() == SMTP_Server_IO::WR_OK )
			{
				n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
				if ( n_send < 1 )
					ACE_OS::printf("send() failed!\n");

				// rc == -1, QUIT close connection
				if ( rc < 0 )
					break;
			}
			else
			{
				// WR_MORE (insufficient send buffer)
				ACE_OS::printf("insufficient send buffer!\n");
				break;
			}
		}
		while( n_send > 0 );
	}

	io.close();
	stream.close();
		
	return 0;
}

// } // namespace aos

