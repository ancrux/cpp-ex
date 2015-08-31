#ifndef _ASIO_SSL_SERVER_CONNECTION_H_
#define _ASIO_SSL_SERVER_CONNECTION_H_

#include "ace/Message_Block.h"
#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "asio.hpp"
#include "asio/ssl.hpp"

namespace asio {

class SSL_Server;

class SSL_Server_Connection : public boost::enable_shared_from_this< SSL_Server_Connection >
{
public:
	typedef boost::shared_ptr< SSL_Server_Connection > sptr;
	typedef ssl::stream< ip::tcp::socket > ssl_socket;
	static const int BUF_SIZE = 4096;

public:
	SSL_Server_Connection(SSL_Server& server, long timeout = 0, size_t buf_size = 0);
	virtual ~SSL_Server_Connection();

public:
	SSL_Server& server() const { return server_; };
	ssl_socket::next_layer_type& socket() { return socket_.next_layer(); }; // ip::tcp::socket& socket() { return socket_; };

public:
	virtual void on_open(ACE_Message_Block& mb); // connection starting point
	virtual void on_read_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_write_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_close();

public:
	virtual void on_socket_timeout();
	virtual void on_read_error(const error_code& error);
	virtual void on_write_error(const error_code& error);

public:
	void open(bool ssl_connect, const char* ip_addr, unsigned short port); // connection starting point
	void read(ACE_Message_Block& mb);
	void write(ACE_Message_Block& mb);
	void close();
	
protected:
	void handle_read(const error_code& error, size_t bytes_transferred);
	void handle_write(const error_code& error, size_t bytes_transferred);
	void handle_socket_timeout(const error_code& error);

protected:
	SSL_Server& server_;
	ssl_socket socket_; // ip::tcp::socket socket_; // socket
	strand strand_; // sync object
	bool is_ssl_; //? or use_ssl_

	deadline_timer timer_; // time-out timer
	long timeout_; // socket timeout
	
	ACE_Message_Block mb_; // buffer for read/write
};

} // namespace asio

#endif // _ASIO_SSL_SERVER_CONNECTION_H_


