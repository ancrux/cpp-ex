#ifndef _ASIO_SBRS_SERVER_CONNECTION_H_
#define _ASIO_SBRS_SERVER_CONNECTION_H_

#include "SBRS_Server_IO.h"

#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include <boost/bind.hpp>
#include "asio/asio.hpp"

namespace asio {

class SBRS_Server;

class SBRS_Server_Connection
{
public:
	static const int MAX_BUF = 4096;

public: // connection state
	enum
	{
		READY = 0,
		QUIT
	};

public:
	SBRS_Server_Connection(io_service& ios, SBRS_Server* server = 0);
	~SBRS_Server_Connection();

public:
	SBRS_Server* server() const { return server_; };
	ip::tcp::socket& socket() { return socket_; };

public:
	void accept(const char* ip_addr, unsigned short port); // connection starting point
	
public:
	void read(ACE_Message_Block& mb, long timeout = 0);
	void handle_read(const error_code& error, size_t bytes_transferred);
	void write(ACE_Message_Block& mb, long timeout = 0);
	void handle_write(const error_code& error, size_t bytes_transferred);
	void handle_socket_timeout(const error_code& error);

public:
	void cancel_timer();
	void close();
	void handle_close(const error_code& error);

protected:
	ip::tcp::socket socket_; // socket
	strand strand_; // sync object
	deadline_timer timer_; // time-out timer
	long timeout_; // socket timeout in seconds for read/write operation
	ACE_Thread_Mutex lock_; // mutex
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> n_async_op_;

	ACE_Message_Block mb_; // buffer for read/write
	SBRS_Server_IO io_;

	SBRS_Server* server_;

	int state_;
	int count_;
};

} // namespace asio

#endif // _ASIO_SBRS_SERVER_CONNECTION_H_

