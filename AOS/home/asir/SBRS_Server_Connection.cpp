#include "SBRS_Server_Connection.h"

#include "SBRS_Server.h" // friend class

namespace asio {

SBRS_Server_Connection::SBRS_Server_Connection(io_service& ios, SBRS_Server* server)
:
socket_(ios),
strand_(ios),
timer_(ios),
mb_(MAX_BUF),
server_(server),
state_(READY)
{
	timeout_ = server_->timeout();
	io_.open();
}

SBRS_Server_Connection::~SBRS_Server_Connection()
{
	io_.close();
	//ACE_OS::printf("%p:conn dtor()\n", this); //@
}

//-- open

void
SBRS_Server_Connection::accept(const char* ip_addr, unsigned short port)
{
	ACE_OS::printf("client from:%s:%d\n", ip_addr, port); //@

	io_.read_reset(mb_);
	this->read(mb_, timeout_);
}

//-- read/write

void
SBRS_Server_Connection::read(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_read_some(buffer(mb.wr_ptr(), mb.space()), strand_.wrap(boost::bind(
		&SBRS_Server_Connection::handle_read,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SBRS_Server_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SBRS_Server_Connection::handle_read(const error_code& error, size_t bytes_transferred)
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
		aos::bcstr data = io_.read_request(mb_);
	}
	// keep reading
	if ( io_.read_state() != SBRS_Server_IO::RD_OK )
	{
		mb_.reset();
		this->read(mb_, timeout_);
		return;
	}
	// read completed
	else
	{
		// write command
		//ACE_OS::printf("C:%s", io_.buf().c_str()); //@

		// parse command
		aos::Multi_String params;
		aos::Tokenizer toker(io_.buf().c_str(), io_.buf().size());
		toker.set_separator(" \t\r\n");
		int ch = toker.next();
		if ( toker.size() )
		{
			params.push_back(toker.token(), toker.size());
			ch = toker.next();
			if ( toker.size() )
			{
				params.push_back(toker.token(), toker.size());
			}
		}

		if ( ACE_OS::strncasecmp(params[0], "SRL", 3) == 0 && params.size() == 2 )
		{
			ACE_UINT32 ipv4 = IPv4::to_uint32(params[1]);
			double score = -1.0;

			ACE_OS::printf("ip_int:%u\n", ipv4);
			size_t n_map = server_->maps_.size();
			int idx = ipv4 % n_map;
			SBRS_MAP::iterator iter = (server_->maps_[idx])->find(ipv4);
			if ( iter != (server_->maps_[idx])->end() )
				score = (double) iter->second / 255;

			io_.write_score(mb_, score);
			this->write(mb_, timeout_);
		}
		else
		{
			return this->close();
		}
	}
}

void
SBRS_Server_Connection::write(ACE_Message_Block& mb, long timeout)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	socket_.async_write_some(buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
		&SBRS_Server_Connection::handle_write,
		this,
		placeholders::error,
		placeholders::bytes_transferred)));

	if ( timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SBRS_Server_Connection::handle_socket_timeout,
			this,
			placeholders::error)));
	}
}

void
SBRS_Server_Connection::handle_write(const error_code& error, size_t bytes_transferred)
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
		io_.read_reset(mb_);
		this->read(mb_, timeout_);
	}	
}

void
SBRS_Server_Connection::handle_socket_timeout(const error_code& error)
{
	if ( !error )
	{
		//ACE_OS::printf("socket timeout!\n");
		ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
		socket_.close(); // don't call this->close();
	}
}

void
SBRS_Server_Connection::cancel_timer()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);
	timer_.cancel();
}

void
SBRS_Server_Connection::close()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	timer_.expires_from_now(boost::posix_time::seconds(0));
	timer_.async_wait(strand_.wrap(boost::bind(
		&SBRS_Server_Connection::handle_close,
		this,
		placeholders::error)));
}

void
SBRS_Server_Connection::handle_close(const error_code& error)
{
	if ( server_ )
		server_->destroy_connection(this);
	else
		socket_.close();
}

} // namespace asio

