#include "SMTP_Mailer_Connection.h"

#include "SMTP_Mailer.h" // friend class

namespace asio {

SMTP_Mailer_Connection::SMTP_Mailer_Connection(io_service& ios, SMTP_Mailer* mailer, SMTP_Mailer_Domain_Queue* queue)
:
socket_(ios),
strand_(ios),
timer_(ios),
mb_(MAX_BUF),
mailer_(mailer),
queue_(queue),
state_(DISCONNECTED)
{
	io_.open();
}

SMTP_Mailer_Connection::~SMTP_Mailer_Connection()
{
	io_.close();
	//ACE_OS::printf("%p:conn dtor()\n", this); //@
}

//-- connect

void
SMTP_Mailer_Connection::connect(const char* ip_addr, unsigned short port, long timeout)
{
	count_ = 0; //@

	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	ip::tcp::endpoint remote_addr(ip::address::from_string(ip_addr), port);
	socket_.async_connect(remote_addr, strand_.wrap(boost::bind(
		&SMTP_Mailer_Connection::handle_connect,
		this,
		placeholders::error)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Mailer_Connection::handle_connect_timeout,
			this,
			placeholders::error)));
	}
}

void
SMTP_Mailer_Connection::handle_connect(const error_code& error)
{
	this->cancel_timer();
	if ( error )
	{
		//ACE_OS::printf("connect error:%s\n", error.message().c_str()); //@
		return this->close();
	}
	
	//ACE_OS::printf("connected...\n"); //@

	io_.read_reset(mb_);
	this->read(mb_, 5);
}

void
SMTP_Mailer_Connection::handle_connect_timeout(const error_code& error)
{
	if ( !error )
	{
		//ACE_OS::printf("connect timeout!\n");
		ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
		socket_.close(); // don't call this->close();
	}
}

//-- read/write

void
SMTP_Mailer_Connection::read(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_read_some(buffer(mb.wr_ptr(), mb.space()), strand_.wrap(boost::bind(
		&SMTP_Mailer_Connection::handle_read,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Mailer_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SMTP_Mailer_Connection::handle_read(const error_code& error, size_t bytes_transferred)
{
	this->cancel_timer();
	if ( error )
	{
		ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
		return this->close();
	}

	//ACE_OS::printf("read completed: %d\n", bytes_transferred); //@
	mb_.wr_ptr(bytes_transferred);
	// read greetings
	while( mb_.length() > 0 )
	{
		aos::bcstr line = io_.read_line(mb_);
	}
	// keep reading
	if ( io_.read_state() != SMTP_Client_IO::RD_OK )
	{
		mb_.reset();
		this->read(mb_, 5);
		return;
	}
	// read completed
	else
	{
		// write command
		switch(state_)
		{
		case DISCONNECTED:
			state_ = READY;
			ACE_OS::printf("%s", io_.buf().c_str()); //@
			// write noop
			io_.cmd_noop(mb_);
			this->write(mb_, 5);
			break;
		case READY:
			++count_; // # of noop sent
			if ( count_ < 100 )
			{
				// write noop again
				io_.cmd_noop(mb_);
				this->write(mb_, 5);
			}
			else
			{
				// close connection
				this->close();
			}
			break;
		}
	}
}

void
SMTP_Mailer_Connection::write(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_write_some(buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
		&SMTP_Mailer_Connection::handle_write,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Mailer_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SMTP_Mailer_Connection::handle_write(const error_code& error, size_t bytes_transferred)
{
	this->cancel_timer();
	if ( error )
	{
		//ACE_OS::printf("write error:%s\n", error.message().c_str()); //@
		return this->close();
	}

	//ACE_OS::printf("write completed: %d\n", bytes_transferred); //@
	mb_.rd_ptr(bytes_transferred);

	// keep writing
	if ( mb_.length() > 0 )
	{
		this->write(mb_, 5);
		return;
	}
	// write completed
	else
	{
		io_.read_reset(mb_);
		this->read(mb_, 5);
	}	
}

void
SMTP_Mailer_Connection::handle_socket_timeout(const error_code& error)
{
	if ( !error )
	{
		//ACE_OS::printf("socket timeout!\n");
		ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
		socket_.close(); // don't call this->close();
	}
}

void
SMTP_Mailer_Connection::cancel_timer()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
	timer_.cancel();
}

void
SMTP_Mailer_Connection::close()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	timer_.expires_from_now(boost::posix_time::seconds(0));
	timer_.async_wait(strand_.wrap(boost::bind(
		&SMTP_Mailer_Connection::handle_close,
		this,
		placeholders::error)));
}

void
SMTP_Mailer_Connection::handle_close(const error_code& error)
{
	if ( mailer_ )
		mailer_->destroy_connection(this);
	else
		socket_.close();
}

} // namespace asio
