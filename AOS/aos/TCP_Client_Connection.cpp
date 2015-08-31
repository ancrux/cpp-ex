#include "aos/TCP_Client_Connection.h"

#include "aos/TCP_Client.h" // friend class

namespace asio {

TCP_Client_Connection::TCP_Client_Connection(TCP_Client& client)
:
strand_(client.ios()),
is_ssl_(false),
timer_(client.ios()),
timeout_(0),
client_(client)
{
	socket_.reset(new ssl_socket(client_.ios(), client_.ssl_context()));
}

TCP_Client_Connection::~TCP_Client_Connection()
{
	//ACE_OS::printf("conn=%p, ssl=%d\n", this, this->is_ssl_); //@
}

void
TCP_Client_Connection::reset_socket()
{
	mb_.reset();

	// reset only if in ssl state.
	SSL* ssl = (*socket_).impl()->ssl; // test if ssl was used
	if ( ssl->type != 0 ) //? if ( is_ssl_ )
	{
		socket_.reset(new ssl_socket(client_.ios(), client_.ssl_context()));
		is_ssl_ = false;
	}
}

//-- override

void
TCP_Client_Connection::on_open(ACE_Message_Block& mb)
{
	//ACE_OS::printf("connection opened!\n"); //@
}

void
TCP_Client_Connection::on_read_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("read completed!\n"); //@
}

void
TCP_Client_Connection::on_write_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("write completed!\n"); //@
}

void
TCP_Client_Connection::on_close()
{
	//ACE_OS::printf("connection closed!\n"); //@
}

void
TCP_Client_Connection::on_socket_timeout(int pending_operation, long timeout)
{
	//ACE_OS::printf("socket timeout! op_code=%d timeout=%d\n", pending_operation, timeout); //@
}

void
TCP_Client_Connection::on_connect_error(const error_code& error)
{
	//ACE_OS::printf("connect error:%s\n", error.message().c_str()); //@
}

void
TCP_Client_Connection::on_ssl_handshake_done(ACE_Message_Block& mb)
{
	//ACE_OS::printf("ssl_handshake done!\n"); //@
}

void
TCP_Client_Connection::on_ssl_handshake_error(const error_code& error)
{
	//ACE_OS::printf("ssl_handshake error:%s\n", error.message().c_str()); //@
}

void
TCP_Client_Connection::on_ssl_shutdown_done(ACE_Message_Block& mb)
{
	//ACE_OS::printf("ssl_shutdown done!\n"); //@
}

void
TCP_Client_Connection::on_ssl_shutdown_error(const error_code& error)
{
	//ACE_OS::printf("ssl_shutdown error:%s\n", error.message().c_str()); //@
}

void
TCP_Client_Connection::on_read_error(const error_code& error)
{
	//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
}

void
TCP_Client_Connection::on_write_error(const error_code& error)
{
	//ACE_OS::printf("write error:%s\n", error.message().c_str()); //@
}

//-- connect

void
TCP_Client_Connection::connect(bool ssl_connect, unsigned short port, const char* ip_addr, long connect_timeout, long socket_timeout, long ssl_handshake_timeout, size_t buf_size)
{
	ip::tcp::endpoint remote_endpoint(ip::address::from_string(ip_addr), port);
	
	this->connect(ssl_connect, remote_endpoint, connect_timeout, socket_timeout, ssl_handshake_timeout, buf_size);
}

void
TCP_Client_Connection::connect(bool ssl_connect, const ip::tcp::endpoint& remote_endpoint, long connect_timeout, long socket_timeout, long ssl_handshake_timeout, size_t buf_size)
{
	this->timeout_ = socket_timeout;

	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( connect_timeout > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(connect_timeout));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_socket_timeout,
				this,
				placeholders::error,
				(int) CONNECT,
				connect_timeout
			)));
	}

	socket().async_connect(
		remote_endpoint,
		make_custom_alloc_handler(allocator_,
		strand_.wrap(boost::bind(
			&TCP_Client_Connection::handle_connect,
			this,
			placeholders::error,
			ssl_connect,
			ssl_handshake_timeout,
			buf_size
		))));
}

void
TCP_Client_Connection::handle_connect(const error_code& error, bool ssl_connect, long ssl_handshake_timeout, size_t buf_size)
{
	timer_.expires_at(boost::posix_time::pos_infin); //timer_.cancel();
	if ( error )
	{
		this->on_connect_error(error);
		return this->close();
	}
	
	// ssl async-handshake
	if ( ssl_connect )
	{
		// no matter ssl handshake is done, mark connection in ssl state.
		is_ssl_ = true;

		if ( ssl_handshake_timeout < 0 ) ssl_handshake_timeout = timeout_;
		if ( ssl_handshake_timeout > 0 )
		{
			timer_.expires_from_now(boost::posix_time::seconds(ssl_handshake_timeout));
			timer_.async_wait(
				strand_.wrap(boost::bind(
					&TCP_Client_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_HANDSHAKE,
					ssl_handshake_timeout
				)));
		}

		socket_->async_handshake(
			ssl::stream_base::client,
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_ssl_handshake,
				this,
				placeholders::error,
				true,
				buf_size
			))));
	}
	else
	{
		// delay buffer allocation when handshake is done.
		if ( mb_.size((buf_size)?buf_size:BUF_SIZE) != 0 )
			return this->close();

		this->on_open(mb_);
	}
}

bool
TCP_Client_Connection::ssl_handshake(long ssl_handshake_timeout)
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
					&TCP_Client_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_HANDSHAKE,
					ssl_handshake_timeout
				)));
		}

		socket_->async_handshake(
			ssl::stream_base::client,
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_ssl_handshake,
				this,
				placeholders::error,
				false,
				mb_.size()
			))));
	}

	return was_ssl;
}

void
TCP_Client_Connection::handle_ssl_handshake(const error_code& error, bool on_open, size_t buf_size)
{
	timer_.expires_at(boost::posix_time::pos_infin); //timer_.cancel();
	if ( error )
	{
		this->on_ssl_handshake_error(error);
		return this->close();
	}

	if ( on_open )
	{
		// delay buffer allocation when handshake is done.
		if ( mb_.size((buf_size)?buf_size:BUF_SIZE) != 0 )
			return this->close();

		this->on_open(mb_);
	}
	else
		this->on_ssl_handshake_done(mb_);
}

//-- read/write

void
TCP_Client_Connection::read(ACE_Message_Block& mb)
{
	//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_socket_timeout,
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
				&TCP_Client_Connection::handle_read,
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
				&TCP_Client_Connection::handle_read,
				this,
				placeholders::error,
				placeholders::bytes_transferred
			))));
	}
}

void
TCP_Client_Connection::handle_read(const error_code& error, size_t bytes_transferred)
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
TCP_Client_Connection::write(ACE_Message_Block& mb, bool write_all)
{
	//ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_socket_timeout,
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
					&TCP_Client_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
		else
			socket_->async_write_some(
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Client_Connection::handle_write,
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
					&TCP_Client_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
		else
			socket().async_write_some(
				buffer(mb.rd_ptr(), mb.length()),
				make_custom_alloc_handler(allocator_,
				strand_.wrap(boost::bind(
					&TCP_Client_Connection::handle_write,
					this,
					placeholders::error,
					placeholders::bytes_transferred
				))));
	}
}

void
TCP_Client_Connection::handle_write(const error_code& error, size_t bytes_transferred)
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
TCP_Client_Connection::close(long ssl_shutdown_timeout)
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
					&TCP_Client_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_SHUTDOWN,
					ssl_shutdown_timeout
				)));
		}

		socket_->async_shutdown(
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_ssl_shutdown,
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
				&TCP_Client_Connection::handle_destroy,
				this
			)));
	}
}

bool
TCP_Client_Connection::ssl_shutdown(long ssl_shutdown_timeout)
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
					&TCP_Client_Connection::handle_socket_timeout,
					this,
					placeholders::error,
					(int) SSL_SHUTDOWN,
					ssl_shutdown_timeout
				)));
		}

		socket_->async_shutdown(
			make_custom_alloc_handler(allocator_,
			strand_.wrap(boost::bind(
				&TCP_Client_Connection::handle_ssl_shutdown,
				this,
				placeholders::error,
				false
			))));
	}

	return was_ssl;
}

void
TCP_Client_Connection::handle_ssl_shutdown(const error_code& error, bool destroy_connection)
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
				&TCP_Client_Connection::handle_destroy,
				this
			)));
	}
}

void
TCP_Client_Connection::handle_socket_timeout(const error_code& error, int pending_operation, long timeout)
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
TCP_Client_Connection::handle_destroy()
{
	// timer handler

	// always destroy connection in timer handler, ensure execute lastly!
	client_.destroy_connection(this);
}

} // namespace asio

