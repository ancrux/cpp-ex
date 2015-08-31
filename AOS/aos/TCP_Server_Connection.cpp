#include "aos/TCP_Server_Connection.h"

#include "aos/TCP_Server.h" // friend class

//#include "asio/impl/src.hpp" // ASIO_SEPARATE_COMPILATION

namespace asio {

TCP_Server_Connection::TCP_Server_Connection(TCP_Server& server, long timeout, size_t buf_size)
:
strand_(server.ios()),
is_ssl_(false),
timer_(server.ios()),
timeout_(timeout),
mb_((buf_size)?buf_size:BUF_SIZE),
server_(server)
{
	socket_.reset(new ssl_socket(server_.ios(), server_.ssl_context()));
}

TCP_Server_Connection::~TCP_Server_Connection()
{
	//ACE_OS::printf("conn=%p, ssl=%d\n", this, this->is_ssl_); //@
}

void
TCP_Server_Connection::reset_socket()
{
	mb_.reset();

	// reset only if in ssl state.
	SSL* ssl = (*socket_).impl()->ssl; // test if ssl was used
	if ( ssl->type != 0 ) //? if ( is_ssl_ )
	{
		socket_.reset(new ssl_socket(server_.ios(), server_.ssl_context()));
		is_ssl_ = false;
	}
}

//-- override

void
TCP_Server_Connection::on_open(ACE_Message_Block& mb)
{
	//ACE_OS::printf("connection opened!\n"); //@
}

void
TCP_Server_Connection::on_read_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("read completed!\n"); //@
}

void
TCP_Server_Connection::on_write_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("write completed!\n"); //@
}

void
TCP_Server_Connection::on_close()
{
	//ACE_OS::printf("connection closed!\n"); //@
}

void
TCP_Server_Connection::on_socket_timeout(int pending_operation, long timeout)
{
	//ACE_OS::printf("socket timeout! op_code=%d timeout=%d\n", pending_operation, timeout); //@
}

void
TCP_Server_Connection::on_ssl_handshake_done(ACE_Message_Block& mb)
{
	//ACE_OS::printf("ssl_handshake done!\n"); //@
}

void
TCP_Server_Connection::on_ssl_handshake_error(const error_code& error)
{
	//ACE_OS::printf("ssl_handshake error:%s\n", error.message().c_str()); //@
}

void
TCP_Server_Connection::on_ssl_shutdown_done(ACE_Message_Block& mb)
{
	//ACE_OS::printf("ssl_shutdown done!\n"); //@
}

void
TCP_Server_Connection::on_ssl_shutdown_error(const error_code& error)
{
	//ACE_OS::printf("ssl_shutdown error:%s\n", error.message().c_str()); //@
}

void
TCP_Server_Connection::on_read_error(const error_code& error)
{
	//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
}

void
TCP_Server_Connection::on_write_error(const error_code& error)
{
	//ACE_OS::printf("write error:%s\n", error.message().c_str()); //@
}

//-- open

void
TCP_Server_Connection::open(bool ssl_connect)
{
	// ssl async-handshake
	if ( ssl_connect )
	{
		long ssl_handshake_timeout = this->ssl_handshake_timeout_on_open();
		
		// no matter ssl handshake is done, mark connection in ssl state.
		is_ssl_ = true;

		if ( ssl_handshake_timeout < 0 ) ssl_handshake_timeout = timeout_;
		if ( ssl_handshake_timeout > 0 )
		{
			timer_.expires_from_now(boost::posix_time::seconds(ssl_handshake_timeout));
			timer_.async_wait(
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_HANDSHAKE,
					ssl_handshake_timeout
				)));
		}

		socket_->async_handshake(
			ssl::stream_base::server,
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_ssl_handshake,
				this,
				placeholders::error,
				true
			))));
	}
	else
	{
		this->on_open(mb_);
	}
}

bool
TCP_Server_Connection::ssl_handshake(long ssl_handshake_timeout)
{
	bool was_ssl = is_ssl_;

	if ( !was_ssl )
	{
		// no matter ssl handshake is done, mark connection in ssl state.
		is_ssl_ = true;

		if ( ssl_handshake_timeout < 0 ) ssl_handshake_timeout = timeout_;
		if ( ssl_handshake_timeout > 0 )
		{
			timer_.expires_from_now(boost::posix_time::seconds(ssl_handshake_timeout));
			timer_.async_wait(
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_HANDSHAKE,
					ssl_handshake_timeout
				)));
		}

		socket_->async_handshake(
			ssl::stream_base::server,
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_ssl_handshake,
				this,
				placeholders::error,
				false
			))));
	}

	return was_ssl;
}

void
TCP_Server_Connection::handle_ssl_handshake(const error_code& error, bool on_open)
{
	timer_.expires_at(boost::posix_time::pos_infin); //timer_.cancel();
	if ( error )
	{
		this->on_ssl_handshake_error(error);
		return this->close();
	}

	if ( on_open )
		this->on_open(mb_);
	else
		this->on_ssl_handshake_done(mb_);
}

//-- read/write

void
TCP_Server_Connection::read(ACE_Message_Block& mb)
{
	//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_socket_timeout,
				this,
				placeholders::error,
				(int) READ,
				timeout_
			)));
	}

	if ( is_ssl_ )
	{
		socket_->async_read_some(
			buffer(mb.wr_ptr(), mb.space()),
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_read,
				this,
				placeholders::error,
				placeholders::bytes_transferred
			))));
	}
	else
	{
		socket().async_read_some(
			buffer(mb.wr_ptr(), mb.space()),
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_read,
				this,
				placeholders::error,
				placeholders::bytes_transferred
			))));
	}
}

void
TCP_Server_Connection::handle_read(const error_code& error, size_t bytes_transferred)
{
	timer_.expires_at(boost::posix_time::pos_infin); //timer_.cancel();
	if ( error )
	{
		this->on_read_error(error);
		return this->close();
	}

	mb_.wr_ptr(bytes_transferred);
	this->on_read_some(mb_, bytes_transferred);
}

void
TCP_Server_Connection::write(ACE_Message_Block& mb, bool write_all)
{
	//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_socket_timeout,
				this,
				placeholders::error,
				(int) WRITE,
				timeout_
			)));
	}

	if ( is_ssl_ )
	{
		if ( write_all )
			asio::async_write(
				*socket_,
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
		else
			socket_->async_write_some(
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
	}
	else
	{
		if ( write_all )
			asio::async_write(
				socket(),
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
		else
			socket().async_write_some(
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
	}
}

void
TCP_Server_Connection::handle_write(const error_code& error, size_t bytes_transferred)
{
	timer_.expires_at(boost::posix_time::pos_infin); //timer_.cancel();
	if ( error )
	{
		this->on_write_error(error);
		return this->close();
	}

	mb_.rd_ptr(bytes_transferred);
	this->on_write_some(mb_, bytes_transferred);
}

void
TCP_Server_Connection::close(long ssl_shutdown_timeout)
{
	this->on_close();

	//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( is_ssl_ )
	{
		if ( ssl_shutdown_timeout < 0 ) ssl_shutdown_timeout = timeout_;
		if ( ssl_shutdown_timeout > 0 )
		{
			timer_.expires_from_now(boost::posix_time::seconds(ssl_shutdown_timeout));
			timer_.async_wait(
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_SHUTDOWN,
					ssl_shutdown_timeout
				)));
		}

		socket_->async_shutdown(
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_ssl_shutdown,
				this,
				placeholders::error,
				true
			))));
	}
	else
	{
		asio::error_code err;
		socket().close(err);

		timer_.expires_from_now(boost::posix_time::seconds(0));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_destroy,
				this
			)));
	}
}

bool
TCP_Server_Connection::ssl_shutdown(long ssl_shutdown_timeout)
{
	bool was_ssl = is_ssl_;

	if ( was_ssl )
	{
		if ( ssl_shutdown_timeout < 0 ) ssl_shutdown_timeout = timeout_;
		if ( ssl_shutdown_timeout > 0 )
		{
			timer_.expires_from_now(boost::posix_time::seconds(ssl_shutdown_timeout));
			timer_.async_wait(
				strand_.wrap(boost::bind(
					&TCP_Server_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_SHUTDOWN,
					ssl_shutdown_timeout
				)));
		}

		socket_->async_shutdown(
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_ssl_shutdown,
				this,
				placeholders::error,
				false
			))));
	}

	return was_ssl;
}

void
TCP_Server_Connection::handle_ssl_shutdown(const error_code& error, bool destroy_connection)
{
	if ( !error )
	{
		is_ssl_ = false;
		this->on_ssl_shutdown_done(mb_);
	}
	else
		this->on_ssl_shutdown_error(error);

	if ( destroy_connection || error )
	{
		//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

		asio::error_code err;
		socket().close(err);

		timer_.expires_from_now(boost::posix_time::seconds(0));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Server_Connection::handle_destroy,
				this
			)));
	}
}

void
TCP_Server_Connection::handle_socket_timeout(const error_code& error, int pending_operation, long timeout)
{
	// timer handler

	if ( !error )
	{
		this->on_socket_timeout(pending_operation, timeout);

		// pending async operation (read/write) in progress
		// simply close socket when timeout to trigger operation failure on async handler
		asio::error_code err;
		socket().close(err); // socket().cancel();
	}
}


void
TCP_Server_Connection::handle_destroy()
{
	// timer handler

	// always destroy connection in timer handler, ensure execute lastly!
	server_.destroy_connection(this);
}

} // namespace asio

