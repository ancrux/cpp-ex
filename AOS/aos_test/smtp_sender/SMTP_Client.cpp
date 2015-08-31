#include "SMTP_Client.h"

#include "aos/net/smtp/SMTP_Client_IO.h"

/*
//namespace aos {

HTTP_Server::HTTP_Server()
:
max_conn_(MAX_CONN+1),
shutdown_(0)
{
}

HTTP_Server::~HTTP_Server()
{
}

int
HTTP_Server::open(const ACE_Addr& addr)
{
	this->enable();
	if ( acceptor_.open(addr) == -1 )
		return -1; // cannot bind on the server_addr

	return 0;
}

int
HTTP_Server::close()
{
	this->disable();
	acceptor_.close();

	return 0;
}

int
HTTP_Server::start()
{
	this->enable();
	this->activate(THR_NEW_LWP | THR_DETACHED, 1, 0); // create detached 

	return 0;
}

int
HTTP_Server::stop(const ACE_Time_Value* timeout)
{
	this->disable();
	this->thr_mgr()->wait(timeout);

	return 0;
}

int
HTTP_Server::svc()
{
	//ACE_OS::printf("(%d) started...\n", ACE_OS::thr_self());
	if ( shutdown_ == 1 )
		return 0;

#if USE_SSL == 1
	aos::SSL_Socket_Stream stream; //ACE_SSL_SOCK_Stream stream;
#else
	aos::Socket_Stream stream; //ACE_SOCK_Stream stream;
#endif
	ACE_Time_Value timeout(3);
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
	HTTP_Server_IO io;
	io.open();

	ssize_t n_recv = -1;
	ssize_t n_send = -1;
	int support_keep_alive = 1;

	do
	{
		// read request
		io.read_reset(mb);
		while( io.read_state() < HTTP_Server_IO::RD_OK )
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
				if ( io.read_state() != HTTP_Server_IO::RD_DATA )
				{
					aos::bcstr header = io.read_header(mb);
					//ACE_OS::printf("%s", io.buf().c_str());
					if ( io.read_state() > HTTP_Server_IO::RD_HEADER )
					{
						//io.dump_headers();
						//ACE_OS::printf("(%d) is_keep_alive(): %d\n", ACE_OS::thr_self(), io.is_keep_alive());
					}
				}
				else
				{
					aos::bcstr data = io.read_data(mb);
					//std::string in_data(data.buf, data.len);
					//ACE_OS::printf("%s", in_data.c_str());
				}
			}
		}
		if ( n_recv < 1 ) break; // read failed, close connection
		//ACE_OS::printf("\n\n");

		
		// write user-defined response
		std::string content; content.reserve(1024);
		int n = ACE_OS::snprintf(&content[0], content.capacity(), "%d", ACE_OS::time());
		content.resize(n);
		content += "\r\n";

		mb.reset();
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "HTTP/%.1f %d %s\r\n", 1.1, 200, "OK"));
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Content-Type: %s\r\n", "text/plain"));
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Content-Length: %d\r\n", content.size()));
		if ( support_keep_alive && io.keep_alive() ) 
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Connection: %s\r\n", "Keep-Alive"));
		else
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Connection: %s\r\n", "close"));
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));
		mb.copy(content.c_str(), content.size());

		// server write
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
		if ( n_send < 1 ) break; // write failed, close connection

		// write file response
		const char* root = "d:\\reporter2";
		std::string filename(root);
		std::string url;
		io.get_request(0, &url);
		filename += url;
		//should also check if filename begins with web_root
		
		if ( !support_keep_alive ) io.keep_alive(0);
		n_send = io.write_file(stream, flags, &timeout, mb, filename.c_str(), "text/html");
		if ( n_send < 0 ) break; // write failed, close connection

	}
	while( support_keep_alive && io.keep_alive() );

	io.close();
	stream.close();
		
	return 0;
}

// } // namespace aos
//*/

