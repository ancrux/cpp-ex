#ifndef _ASIO_SMTP_MAILER_H_
#define _ASIO_SMTP_MAILER_H_

#include "SMTP_Mailer_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>

namespace asio {

class SMTP_Mailer : public ACE_Task_Base
{
friend class SMTP_Mailer_Connection;

public:
	typedef std::multimap< std::string, SMTP_Mailer_Connection* > CONNECTIONS;

public:
	static const size_t MAX_CONNECTION = 10240;

public:
	SMTP_Mailer(io_service& ios);
	~SMTP_Mailer();

public:
	virtual int svc();

public:
	void start(int n_thread = 0);
	void stop();

public:
	int connect(const char* ip_addr, unsigned short port = 25, long connect_timeout = 0, long socket_timeout = 0, SMTP_Mailer_Domain_Queue* queue = 0);
	size_t n_connection(const char* key = 0);
	int is_connected_to(const char* key);
	CONNECTIONS& pool() { return pool_; };
	ACE_Thread_Mutex& lock() { return lock_; };

protected:
	SMTP_Mailer_Connection* create_connection(const char* key, SMTP_Mailer_Domain_Queue* queue = 0);
	void destroy_connection(SMTP_Mailer_Connection* conn);
	void clear_all_connections();

protected:
	io_service& ios_;
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;

	CONNECTIONS pool_; // connection pool
	ACE_Thread_Mutex lock_; // mutex lock

	//SMTP_Mailer_Job_Queue job_queue_;
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_H_
