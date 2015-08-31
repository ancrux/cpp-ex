#ifndef _ASIO_SMTP_MAILER_H_
#define _ASIO_SMTP_MAILER_H_

#include "SMTP_Listener_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>

namespace asio {

class SMTP_Listener : public ACE_Task_Base
{
friend class SMTP_Listener_Connection;

public:
	typedef std::multimap< std::string, SMTP_Listener_Connection* > CONNECTIONS;

public:
	static const size_t MAX_CONNECTION = 0; //10240;

public:
	SMTP_Listener(io_service& ios);
	~SMTP_Listener();

public:
	virtual int svc();

public:
	void start(unsigned short port, const char* ip_addr = 0, int n_thread = 0);
	void stop();

public:
	void handle_accept(const error_code& error, SMTP_Listener_Connection* conn);

public:
	size_t n_connection(const char* key = 0);
	int is_connected_from(const char* key);
	CONNECTIONS& pool() { return pool_; };
	ACE_Thread_Mutex& lock() { return lock_; };

protected:
	SMTP_Listener_Connection* create_connection(SMTP_Listener_Connection* conn, const char* key);
	void destroy_connection(SMTP_Listener_Connection* conn);
	void clear_all_connections();

protected:
	io_service& ios_;
	ip::tcp::acceptor acceptor_;
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;

	CONNECTIONS pool_; // connection pool
	ACE_Thread_Mutex lock_; // mutex lock

};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_H_