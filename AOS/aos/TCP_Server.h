#ifndef _ASIO_TCP_SERVER_H_
#define _ASIO_TCP_SERVER_H_

#include "aos/Config.h"

#include "aos/TCP_Server_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>
#include <vector>

namespace asio {

class TCP_Server : public ACE_Task_Base
{
friend class TCP_Server_Connection;

public:
	typedef std::multimap< std::string, TCP_Server_Connection* > CONNECTIONS;
	typedef std::vector< TCP_Server_Connection* > REUSE_CONNECTIONS;

public:
	TCP_Server();
	virtual ~TCP_Server();

public:
	virtual TCP_Server_Connection* make_connection(); // allocate connection object
	virtual void unmake_connection(TCP_Server_Connection* conn); // de-allocate connection object
	virtual asio::error_code prepare_ssl_context(ssl::context& ssl_context);
	virtual int svc();

public:
	asio::error_code start(unsigned short port, const char* ip_addr = 0, size_t n_thread = 0);
	void stop();
	int resize_thread_pool(size_t n_thread);
	void ssl_connect(bool is_ssl_connect) { is_ssl_connect_ = (is_ssl_connect)?1:0; };
	bool ssl_connect() const { return (is_ssl_connect_.value() == 0)?false:true; };
	io_service& ios() { return ios_; };
	ssl::context& ssl_context() { return ssl_context_; };
	void max_connection(long n_max_conn) { n_max_conn_ = n_max_conn; };
	long max_connection() const { return n_max_conn_.value(); };
	
public: // connection mananger (can be moved to a separate class)
	virtual size_t n_connection();
	virtual void on_create_connection(TCP_Server_Connection& conn, const ip::tcp::endpoint& remote_endpoint);
	virtual void on_destroy_connection(TCP_Server_Connection& conn);
	virtual void close_all_connections();
	CONNECTIONS& pool() { return pool_; };

public: // reuse connection
	virtual void on_reuse_connection(TCP_Server_Connection& conn);
	size_t n_reuse_connection();
	void max_reuse_connection(size_t n_max_reuse);
	size_t max_reuse_connection();
	void clear_all_reuse_connections();

protected:
	void handle_accept(const error_code& error);
	TCP_Server_Connection* create_connection();
	void destroy_connection(TCP_Server_Connection* conn);
	
protected:
	io_service ios_;
	ip::tcp::acceptor acceptor_;
	ssl::context ssl_context_;
	TCP_Server_Connection* conn_; // new connection
	ACE_Atomic_Op<ACE_Thread_Mutex, long> is_ssl_connect_; // whether use ssl (do ssl handshake) on accepting a connection
	ACE_Atomic_Op<ACE_Thread_Mutex, long> is_running_;
	ACE_Thread_Mutex server_lock_;

	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_wish_task_;
	long n_task_;  // # of thread(task)
	ACE_Thread_Mutex task_lock_;

	CONNECTIONS pool_; // connection pool
	ACE_Thread_Mutex pool_lock_; // connection pool mutex lock
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_max_conn_; // # of max connections, 0 for no limits, -1 for refuse new connections

	REUSE_CONNECTIONS reuse_; // reuse connection pool
	ACE_Thread_Mutex reuse_lock_; // reuse pool lock
	size_t n_max_reuse_; // # of max reuse connections
};

} // namespace asio

#endif // _ASIO_TCP_SERVER_H_
