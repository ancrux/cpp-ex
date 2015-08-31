#ifndef _ASIO_SMTP_LISTENER_CONNECTION_H_
#define _ASIO_SMTP_LISTENER_CONNECTION_H_

#include "aos/net/smtp/SMTP_Server_IO.h"

#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include <boost/bind.hpp>
#include "asio/asio.hpp"

namespace asio {

class SMTP_Listener;

class SMTP_Listener_Connection
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
	SMTP_Listener_Connection(io_service& ios, SMTP_Listener* listener = 0);
	~SMTP_Listener_Connection();

public:
	SMTP_Listener* listener() const { return listener_; };
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
	ACE_Thread_Mutex lock_; // mutex
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> n_async_op_;

	ACE_Message_Block mb_; // buffer for read/write
	SMTP_Server_IO io_;

	SMTP_Listener* listener_;

	int state_;
	int count_;
};

} // namespace asio

#endif // _ASIO_SMTP_LISTENER_CONNECTION_H_
