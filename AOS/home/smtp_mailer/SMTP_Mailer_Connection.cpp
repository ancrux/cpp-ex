#include "SMTP_Mailer_Connection.h"

#include "SMTP_Mailer_Job.h"
#include "SMTP_Mailer.h" // friend class
//#include "SMTP_Mailer_Domain_Queue.h"

namespace asio {

SMTP_Mailer_Connection::SMTP_Mailer_Connection(io_service& ios, SMTP_Mailer* mailer, SMTP_Mailer_Domain_Queue* queue)
:
socket_(ios),
strand_(ios),
timer_(ios),
mb_(MAX_BUF),
job_(0),
mailer_(mailer),
queue_(queue),
state_(INACTIVE)
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
SMTP_Mailer_Connection::connect(const char* ip_addr, unsigned short port, long connect_timeout, long socket_timeout, SMTP_Mailer_Job* job)
{
	timeout_ = socket_timeout;
	job_ = job;
	state_ = TRY_CONNECT;

	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	ip::tcp::endpoint remote_addr(ip::address::from_string(ip_addr), port);
	socket_.async_connect(remote_addr, strand_.wrap(boost::bind(
		&SMTP_Mailer_Connection::handle_connect,
		this,
		placeholders::error)));

	/*
	try { socket_.set_option(ip::tcp::no_delay(true)); }
	catch(asio::system_error& e) {}
	{
		//ACE_OS::printf("set_option_err:%s\n", e.what()); //@
	}
	//*/

	if ( connect_timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(connect_timeout));
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

	// connection established
	this->on_connect_completed();
}

void
SMTP_Mailer_Connection::on_connect_completed()
{
	io_.read_reset(mb_);
	this->read(mb_, timeout_);
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
		//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
		return this->close();
	}

	//ACE_OS::printf("read completed: %d\n", bytes_transferred); //@
	mb_.wr_ptr(bytes_transferred);
	// read response
	while( mb_.length() > 0 )
	{
		aos::bcstr line = io_.read_line(mb_);
	}
	// keep reading
	if ( io_.read_state() != SMTP_Client_IO::RD_OK )
	{
		mb_.reset();
		this->read(mb_, timeout_);
		return;
	}
	// read completed
	else
	{
		this->on_read_completed();
	}
}

void
SMTP_Mailer_Connection::on_read_completed()
{
	ACE_OS::printf("%s", io_.buf().c_str()); //@
	// write command
	switch(state_)
	{
	case TRY_CONNECT:
		// if ok, helo
		state_ = TRY_HELO;
		io_.cmd_helo(mb_, "smtp_mailer");
		this->write(mb_, timeout_);
		break;
	case TRY_HELO:
		// if ok, rset
		state_ = TRY_RSET;
		io_.cmd_rset(mb_);
		this->write(mb_, timeout_);
		break;
	case TRY_RSET:
		// if ok, do mail job
		if ( job_ )
		{
			// try mail_from
			state_ = TRY_MAIL_FROM;
			io_.cmd_mail_from(mb_, (job_->from).c_str());
			this->write(mb_, timeout_);
		}
		else
		{
			// no job, get one... or quit for now
			state_ = TRY_QUIT;
			io_.cmd_quit(mb_);
			this->write(mb_, timeout_);
		}
		break;
	case TRY_MAIL_FROM:
		// if ok, try rcpt_to
		state_ = TRY_RCPT_TO;
		io_.cmd_rcpt_to(mb_, (job_->to).c_str());
		this->write(mb_, timeout_);
		break;
	case TRY_RCPT_TO:
		// if ok, try data
		state_ = TRY_DATA;
		io_.cmd_data(mb_);
		this->write(mb_, timeout_);
		break;
	case TRY_DATA:
		{
		// if ok, try data_transfer
		state_ = TRY_DATA_TRANSFER;
		mb_.reset();
		mb_.copy((job_->eml).c_str(), (job_->eml).size());
		ACE_Message_Block mb(32);
		io_.cmd_data_completed(mb);
		mb_.copy(mb.rd_ptr(), mb.length());
		this->write(mb_, timeout_);
		break;
		}
	case TRY_DATA_TRANSFER:
		// if ok, quit
		state_ = TRY_QUIT;
		io_.cmd_quit(mb_);
		this->write(mb_, timeout_);
		break;
	case TRY_QUIT:
	default:
		// if quit or unknown, close connection
		state_ = INACTIVE;
		this->close();
		break;
	}
/*
// wait here...
	case TRY_RSET:
		// if ok, wait
		state_ = TRY_WAIT;
		timer_.expires_from_now(boost::posix_time::millisec(500*6));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SMTP_Mailer_Connection::on_read_completed,
			this)));
		break;
// handle wait here...
	case TRY_WAIT:
		// if ok, quit
		state_ = TRY_QUIT;
		io_.cmd_quit(mb_);
		this->write(mb_, timeout_);
		break;
//*/
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
		this->write(mb_, timeout_);
		return;
	}
	// write completed
	else
	{
		this->on_write_completed();
	}	
}

void
SMTP_Mailer_Connection::on_write_completed()
{
	io_.read_reset(mb_);
	this->read(mb_, timeout_);
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
