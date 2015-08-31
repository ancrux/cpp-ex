#ifndef _ASIO_TCP_SERVER_CONNECTION_H_
#define _ASIO_TCP_SERVER_CONNECTION_H_

#include "aos/Config.h"

#include "ace/Message_Block.h"
#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include <boost/bind.hpp>
#include "asio.hpp"
#include "asio/ssl.hpp"

#include "aos/TCP_Async_Handler_Allocator.h"

namespace asio {

class TCP_Server;

class TCP_Server_Connection
{
public:
	typedef ssl::stream< ip::tcp::socket > ssl_socket;
	static const int BUF_SIZE = 4096;

public: // async-operation 
	enum
	{
		NONE = 0,
		READ,
		WRITE,
		SSL_HANDSHAKE,
		SSL_SHUTDOWN,
		OP_MAX
	};

public:
	TCP_Server_Connection(TCP_Server& server, long timeout = 0, size_t buf_size = 0);
	virtual ~TCP_Server_Connection();

public:
	TCP_Server& server() const { return server_; };
	ssl_socket::next_layer_type& socket() { return socket_->next_layer(); };
	bool is_ssl() const { return is_ssl_; };
	void reset_socket();

public:
	std::string remote_addr() { return socket().remote_endpoint().address().to_string(); };
	unsigned short remote_port() { return socket().remote_endpoint().port(); };
	std::string local_addr() { return socket().local_endpoint().address().to_string(); };
	unsigned short local_port() { return socket().local_endpoint().port(); };

public:
	virtual long ssl_handshake_timeout_on_open() { return -1; };
	virtual void on_open(ACE_Message_Block& mb); // connection starting point
	virtual void on_read_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_write_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_ssl_handshake_done(ACE_Message_Block& mb);
	virtual void on_ssl_shutdown_done(ACE_Message_Block& mb);
	virtual void on_close();
	
public:
	virtual void on_socket_timeout(int pending_operation, long timeout);
	virtual void on_read_error(const error_code& error);
	virtual void on_write_error(const error_code& error);
	virtual void on_ssl_handshake_error(const error_code& error);
	virtual void on_ssl_shutdown_error(const error_code& error);

public:
	void open(bool ssl_connect);
	void read(ACE_Message_Block& mb);
	void write(ACE_Message_Block& mb, bool write_all = false);
	void close(long ssl_shutdown_timeout = 3);
	bool ssl_handshake(long ssl_handshake_timeout = -1);
	bool ssl_shutdown(long ssl_shutdown_timeout = -1);
	
protected:
	void handle_ssl_handshake(const error_code& error, bool on_open);
	void handle_ssl_shutdown(const error_code& error, bool destroy_connection);
	void handle_read(const error_code& error, size_t bytes_transferred);
	void handle_write(const error_code& error, size_t bytes_transferred);
	void handle_socket_timeout(const error_code& error, int pending_operation, long timeout); // timer handler
	void handle_destroy(); // timer handler

protected:
	std::auto_ptr< ssl_socket > socket_;
	strand strand_; // sync object
	bool is_ssl_; // SSL state

	deadline_timer timer_; // time-out timer
	long timeout_; // socket timeout
	
	ACE_Message_Block mb_; // buffer for read/write

	TCP_Server& server_;
	ACE_Thread_Mutex lock_; // mutex

	// The allocator to use for handler-based custom memory allocation.
	handler_allocator allocator_;
};

} // namespace asio

#endif // _ASIO_TCP_SERVER_CONNECTION_H_

