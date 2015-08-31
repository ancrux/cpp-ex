#include "SSL_Server_Connection.h"

#include "SSL_Server.h" // friend class

namespace asio {

SSL_Server_Connection::SSL_Server_Connection(SSL_Server& server, long timeout, size_t buf_size)
:
server_(server),
strand_(server.ios_),
socket_(server.ios_, server.ssl_context()),
is_ssl_(false),
timer_(server.ios_),
timeout_(timeout),
mb_((buf_size)?buf_size:BUF_SIZE)
{
}

SSL_Server_Connection::~SSL_Server_Connection()
{
	//ACE_OS::printf("conn=%p, ssl=%d\n", this, this->is_ssl_); //@
}

//-- override

void
SSL_Server_Connection::on_open(ACE_Message_Block& mb)
{
	//ACE_OS::printf("connection opened!\n"); //@
}

void
SSL_Server_Connection::on_read_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("read completed!\n"); //@
}

void
SSL_Server_Connection::on_write_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_OS::printf("write completed!\n"); //@
}

void
SSL_Server_Connection::on_close()
{
	//ACE_OS::printf("connection closed!\n"); //@
}

void
SSL_Server_Connection::on_socket_timeout()
{
	//ACE_OS::printf("socket timeout!\n"); //@
}

void
SSL_Server_Connection::on_read_error(const error_code& error)
{
	//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
}

void
SSL_Server_Connection::on_write_error(const error_code& error)
{
	//ACE_OS::printf("write error:%s\n", error.message().c_str()); //@
}

//-- open

void
SSL_Server_Connection::open(bool ssl_connect, const char* ip_addr, unsigned short port)
{
	//ACE_OS::printf("client from:%s:%d\n", ip_addr, port); //@
	if ( ssl_connect )
	{
		asio::error_code error;
		socket_.handshake(ssl::stream_base::server, error);
		if ( !error )
			is_ssl_ = true;
		else
			return this->close();
	}

	this->on_open(mb_);
}

//-- read/write

void
SSL_Server_Connection::read(ACE_Message_Block& mb)
{
	if ( is_ssl_ )
	{
		socket_.async_read_some(buffer(mb.wr_ptr(), mb.space()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_read,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred)));
	}
	else
	{
		socket().async_read_some(buffer(mb.wr_ptr(), mb.space()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_read,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred)));
	}

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_socket_timeout,
			shared_from_this(), //this,
			placeholders::error)));
	}
}

void
SSL_Server_Connection::handle_read(const error_code& error, size_t bytes_transferred)
{
	if ( error )
	{
		this->on_read_error(error);
		return this->close();
	}
	timer_.cancel();

	mb_.wr_ptr(bytes_transferred);
	this->on_read_some(mb_, bytes_transferred);
}

void
SSL_Server_Connection::write(ACE_Message_Block& mb)
{
	if ( is_ssl_ )
	{
		socket_.async_write_some(buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_write,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred)));
		
		/* asio::async_write(socket_, buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_write,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred))); //*/
	}
	else
	{
		socket().async_write_some(buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_write,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred)));
 
		/* asio::async_write(socket(), buffer(mb.rd_ptr(), mb.length()), strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_write,
			shared_from_this(), //this,
			placeholders::error,
			placeholders::bytes_transferred))); //*/
	}

	if ( timeout_ > 0 )
	{
		timer_.expires_from_now(boost::posix_time::seconds(timeout_));
		timer_.async_wait(strand_.wrap(boost::bind(
			&SSL_Server_Connection::handle_socket_timeout,
			shared_from_this(), //this,
			placeholders::error)));
	}
}

void
SSL_Server_Connection::handle_write(const error_code& error, size_t bytes_transferred)
{
	if ( error )
	{
		this->on_write_error(error);
		return this->close();
	}
	timer_.cancel();

	mb_.rd_ptr(bytes_transferred);
	this->on_write_some(mb_, bytes_transferred);
}

void
SSL_Server_Connection::handle_socket_timeout(const error_code& error)
{
	if ( !error )
	{
		this->on_socket_timeout();

		if ( is_ssl_ )
		{
			asio::error_code error;
			socket_.shutdown(error);
		}
		socket().close();
	}
}

void
SSL_Server_Connection::close()
{
	this->on_close();

	if ( is_ssl_ )
	{
		asio::error_code error;
		socket_.shutdown(error);
	}
	socket().close();

	//ACE_OS::printf("sptr=%p: ref=%d\n", this, shared_from_this().use_count());
	server_.destroy_connection(shared_from_this()); // this
}

} // namespace asio

