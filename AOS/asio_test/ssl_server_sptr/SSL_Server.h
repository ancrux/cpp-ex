#ifndef _ASIO_SSL_SERVER_H_
#define _ASIO_SSL_SERVER_H_

#include "SSL_Server_Connection.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include <map>

namespace asio {

class SSL_Server : public ACE_Task_Base
{
friend class SSL_Server_Connection;

public:
	typedef std::multimap< std::string, SSL_Server_Connection* > CONNECTIONS;

public:
	static const size_t MAX_CONNECTION = 0; //10240;

public:
	SSL_Server();
	virtual ~SSL_Server();

public:
	virtual int svc();
	virtual SSL_Server_Connection* make_connection();

public:
	asio::error_code start(unsigned short port, const char* ip_addr = 0, int n_thread = 0);
	void stop();
	void ssl_connect(bool is_ssl_connect) { is_ssl_connect_ = (is_ssl_connect)?1:0; };
	bool ssl_connect() { return (is_ssl_connect_.value() == 0)?false:true; };
	ssl::context& ssl_context() { return ssl_context_; };
	ACE_Thread_Mutex& lock() { return lock_; };

public:
	void handle_accept(const error_code& error);

public: // can be moved to a connection manager class
	size_t n_connection(const char* key = 0);
	int is_connected_from(const char* key);
	CONNECTIONS& pool() { return pool_; };

protected: // can be moved to a connection manager class
	SSL_Server_Connection* create_connection(SSL_Server_Connection::sptr conn, const char* key);
	void destroy_connection(SSL_Server_Connection::sptr conn);
	void clear_all_connections();

protected:
	asio::error_code prepare_ssl_context();

protected:
	io_service ios_;
	ip::tcp::acceptor acceptor_;
	ssl::context ssl_context_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> is_ssl_connect_; // whether use ssl (do ssl handshake) on accepting a connection

	SSL_Server_Connection::sptr conn_; // new connection
	CONNECTIONS pool_; // connection pool

	ACE_Thread_Mutex lock_; // mutex lock
};

} // namespace asio

#endif // _ASIO_SSL_SERVER_H_

