#ifndef _ASIO_TCP_CLIENT_H_
#define _ASIO_TCP_CLIENT_H_

#include "aos/Config.h"

#include "aos/TCP_Client_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>
#include <vector>

namespace asio {

class TCP_Client : public ACE_Task_Base
{
friend class TCP_Client_Connection;

public:
	typedef std::multimap< std::string, TCP_Client_Connection* > CONNECTIONS;
	typedef std::vector< TCP_Client_Connection* > REUSE_CONNECTIONS;

public:
	TCP_Client();
	virtual ~TCP_Client();

public:
	int connect(bool ssl_connect, unsigned short port, const char* ip_addr, long connect_timeout = 10, long socket_timeout = 30, long ssl_handshake_timeout = -1, size_t buf_size = 0);
	int connect(bool ssl_connect, const ip::tcp::endpoint& remote_endpoint, long connect_timeout = 10, long socket_timeout = 30, long ssl_handshake_timeout = -1, size_t buf_size = 0);

public:
	virtual TCP_Client_Connection* make_connection(); // allocate connection object
	virtual void unmake_connection(TCP_Client_Connection* conn); // de-allocate connection object
	virtual asio::error_code prepare_ssl_context(ssl::context& ssl_context);
	virtual int svc();

public:
	asio::error_code start(size_t n_thread = 0);
	void stop();
	int resize_thread_pool(size_t n_thread);
	io_service& ios() { return ios_; };
	ssl::context& ssl_context() { return ssl_context_; };
	void max_connection(long n_max_conn) { n_max_conn_ = n_max_conn; };
	long max_connection() { return n_max_conn_.value(); };

public: // connection mananger (can be moved to a separate class)
	virtual size_t n_connection();
	virtual void on_create_connection(TCP_Client_Connection& conn, const ip::tcp::endpoint& remote_endpoint);
	virtual void on_destroy_connection(TCP_Client_Connection& conn);
	virtual void close_all_connections();
	CONNECTIONS& pool() { return pool_; };

public: // reuse connection
	virtual void on_reuse_connection(TCP_Client_Connection& conn);
	size_t n_reuse_connection();
	void max_reuse_connection(size_t n_max_reuse);
	size_t max_reuse_connection();
	void clear_all_reuse_connections();

protected:
	TCP_Client_Connection* create_connection();
	void destroy_connection(TCP_Client_Connection* conn);
	
protected:
	io_service ios_;
	ssl::context ssl_context_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> is_running_;
	ACE_Thread_Mutex client_lock_;

	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_wish_task_;
	long n_task_; // # of thread(task)
	ACE_Thread_Mutex task_lock_;

	CONNECTIONS pool_; // connection pool
	ACE_Thread_Mutex pool_lock_; // connection pool mutex lock
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_max_conn_; // # of max connections

	REUSE_CONNECTIONS reuse_; // reuse connection pool
	ACE_Thread_Mutex reuse_lock_; // reuse pool lock
	size_t n_max_reuse_; // # of max reuse connections
};

} // namespace asio

#endif // _ASIO_TCP_CLIENT_H_
