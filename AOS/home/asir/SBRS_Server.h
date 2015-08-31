#ifndef _ASIO_SBRS_SERVER_H_
#define _ASIO_SBRS_SERVER_H_

#include "SBRS.h"
#include "SBRS_Server_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>

namespace asio {

class SBRS_Server : public ACE_Task_Base
{
friend class SBRS_Server_Connection;

public:
	typedef std::multimap< std::string, SBRS_Server_Connection* > CONNECTIONS;

public:
	static const size_t MAX_CONNECTION = 0; //10240;

public:
	SBRS_Server(io_service& ios, SBRS_MAPS& maps);
	~SBRS_Server();

public:
	virtual int svc();

public:
	void start(unsigned short port, const char* ip_addr = 0, int n_thread = 0);
	void stop();

public:
	void handle_accept(const error_code& error, SBRS_Server_Connection* conn);

public:
	long timeout() const { return timeout_.value(); };
	void timeout(long timeout) { timeout_ = timeout; };

public:
	size_t n_connection(const char* key = 0);
	int is_connected_from(const char* key);
	CONNECTIONS& pool() { return pool_; };
	ACE_Thread_Mutex& lock() { return lock_; };

protected:
	SBRS_Server_Connection* create_connection(SBRS_Server_Connection* conn, const char* key);
	void destroy_connection(SBRS_Server_Connection* conn);
	void clear_all_connections();

protected:
	io_service& ios_;
	SBRS_MAPS& maps_;
	ip::tcp::acceptor acceptor_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> timeout_; // socket timeout in seconds for read/write operation
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;

	CONNECTIONS pool_; // connection pool
	ACE_Thread_Mutex lock_; // mutex lock

};

} // namespace asio

#endif // _ASIO_SBRS_SERVER_H_
