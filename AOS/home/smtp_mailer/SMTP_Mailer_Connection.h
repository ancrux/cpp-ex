#ifndef _ASIO_SMTP_MAILER_CONNECTION_H_
#define _ASIO_SMTP_MAILER_CONNECTION_H_

#include "aos/net/smtp/SMTP_Client_IO.h"

#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include <boost/bind.hpp>
#include "asio/asio.hpp"

namespace asio {

class SMTP_Mailer_Job;
class SMTP_Mailer;
class SMTP_Mailer_Domain_Queue;

class SMTP_Mailer_Connection
{
public:
	static const int MAX_BUF = 4096;

public: // connection state
	enum
	{
		INACTIVE = 0,
		TRY_CONNECT,
		TRY_HELO,
		TRY_MAIL_FROM,
		TRY_RCPT_TO,
		TRY_DATA,
		TRY_DATA_TRANSFER,
		TRY_RSET,
		TRY_WAIT,
		TRY_QUIT
	};

public:
	SMTP_Mailer_Connection(io_service& ios, SMTP_Mailer* mailer = 0, SMTP_Mailer_Domain_Queue* queue = 0);
	virtual ~SMTP_Mailer_Connection();

public:
	SMTP_Mailer* mailer() const { return mailer_; };
	SMTP_Mailer_Domain_Queue* queue() const { return queue_; };

public:
	void connect(const char* ip_addr, unsigned short port = 25, long connect_timeout = 0, long socket_timeout = 0, SMTP_Mailer_Job* job = 0);
	void handle_connect(const error_code& error);
	void handle_connect_timeout(const error_code& error);
	
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

public:
	virtual void on_connect_completed();
	virtual void on_read_completed();
	virtual void on_write_completed();

protected:
	ip::tcp::socket socket_; // socket
	strand strand_; // sync object
	deadline_timer timer_; // timer for timeout operation
	long timeout_; // socket timeout in seconds for read/write operation
	ACE_Thread_Mutex lock_; // mutex
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> n_async_op_;

	ACE_Message_Block mb_; // buffer for read/write
	SMTP_Client_IO io_;
	SMTP_Mailer_Job* job_; // mail job

	SMTP_Mailer* mailer_; // optional connection manager
	SMTP_Mailer_Domain_Queue* queue_; // optional mail job queue
	//? file handle to eml or use mailer_job to store file handle

	int state_;
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_CONNECTION_H_
