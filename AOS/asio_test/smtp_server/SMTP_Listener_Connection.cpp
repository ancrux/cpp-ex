#include "SMTP_Listener_Connection.h"

#include "SMTP_Listener.h" // friend class

namespace asio {

SMTP_Listener_Connection::SMTP_Listener_Connection(io_service& ios, SMTP_Listener* listener)
:
socket_(ios),
strand_(ios),
timer_(ios),
mb_(MAX_BUF),
listener_(listener),
state_(READY)
{
	io_.open();
}

SMTP_Listener_Connection::~SMTP_Listener_Connection()
{
	io_.close();
	//ACE_OS::printf("%p:conn dtor()\n", this); //@
}

//-- open

void
SMTP_Listener_Connection::accept(const char* ip_addr, unsigned short port)
{
	//ACE_OS::printf("client from:%s:%d\n", ip_addr, port); //@

	io_.greetings(mb_, "domain.com ESMTP product_name");
	this->write(mb_, 5);
}

//-- read/write

void
SMTP_Listener_Connection::read(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_read_some(buffer(mb.wr_ptr(), mb.space()), strand_.wrap(boost::bind(
		&SMTP_Listener_Connection::handle_read,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Listener_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SMTP_Listener_Connection::handle_read(const error_code& error, size_t bytes_transferred)
{
	this->cancel_timer();
	if ( error )
	{
		//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
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
	if ( io_.read_state() != SMTP_Server_IO::RD_OK )
	{
		mb_.reset();
		this->read(mb_, 5);
		return;
	}
	// read completed
	else
	{
		// write command
		//ACE_OS::printf("%s", io_.buf().c_str()); //@

		if ( ACE_OS::strncasecmp(io_.buf().c_str(), "QUIT", 4) == 0 )
		{
			state_ = QUIT;
		}
		io_.write_line(mb_, 200, "OK");
		this->write(mb_, 5);
	}
}

void
SMTP_Listener_Connection::write(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_write_some(buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
		&SMTP_Listener_Connection::handle_write,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Listener_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SMTP_Listener_Connection::handle_write(const error_code& error, size_t bytes_transferred)
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
		if ( QUIT == state_ ) 
			return this->close();

		io_.read_reset(mb_);
		this->read(mb_, 5);
	}	
}

void
SMTP_Listener_Connection::handle_socket_timeout(const error_code& error)
{
	if ( !error )
	{
		//ACE_OS::printf("socket timeout!\n");
		ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
		socket_.close(); // don't call this->close();
	}
}

void
SMTP_Listener_Connection::cancel_timer()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
	timer_.cancel();
}

void
SMTP_Listener_Connection::close()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	timer_.expires_from_now(boost::posix_time::seconds(0));
	timer_.async_wait(strand_.wrap(boost::bind(
		&SMTP_Listener_Connection::handle_close,
		this,
		placeholders::error)));
}

void
SMTP_Listener_Connection::handle_close(const error_code& error)
{
	if ( listener_ )
		listener_->destroy_connection(this);
	else
		socket_.close();
}

} // namespace asio
