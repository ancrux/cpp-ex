#include "SSL_Server.h"

#include "ace/OS.h" // ACE_OS::num_processors_online()

namespace asio {

SSL_Server::SSL_Server()
:
acceptor_(ios_),
ssl_context_(ios_, ssl::context::sslv23),
is_ssl_connect_(1)
{
	this->prepare_ssl_context();
}

SSL_Server::~SSL_Server()
{
	this->stop();
}

asio::error_code
SSL_Server::prepare_ssl_context()
{
	asio::error_code error;
	do
	{
		ssl_context_.set_options(ssl::context::default_workarounds, error); if ( error ) break;
		ssl_context_.use_certificate_file("server.pem", asio::ssl::context::pem, error); if ( error ) break;
		ssl_context_.use_private_key_file("server.key", asio::ssl::context::pem, error); if ( error ) break;
	}
	while(0);

	return error;
}

int
SSL_Server::svc()
{
	ACE_OS::printf("thr(%u) start...\n", ACE_OS::thr_self()); //@

	//io_service::work work(ios_);
	ios_.run();

	ACE_OS::printf("thr(%u) stop...\n", ACE_OS::thr_self()); //@

	return 0;
}

SSL_Server_Connection*
SSL_Server::make_connection()
{
	return new (std::nothrow) SSL_Server_Connection(*this);
}


asio::error_code
SSL_Server::start(unsigned short port, const char* ip_addr, int n_thread)
{
	asio::error_code error;

	ip::tcp::endpoint local_addr(ip::tcp::v4(), port);
	if ( ip_addr )
		local_addr.address(ip::address::from_string(ip_addr));

	do
	{
		acceptor_.open(local_addr.protocol(), error); if ( error ) break;
		acceptor_.set_option(ip::tcp::acceptor::reuse_address(true), error); if ( error ) break;
		acceptor_.bind(local_addr, error); if ( error ) break;
		int backlog = socket_base::max_connections;
		acceptor_.listen(backlog, error); if ( error ) break;

		// prepare connection for accept()
		conn_.reset(this->make_connection());
		acceptor_.async_accept(conn_->socket(), boost::bind(
			&SSL_Server::handle_accept,
			this,
			placeholders::error));
		
		if ( n_thread < 1 )
			n_thread = ACE_OS::num_processors_online(); //? * 4 for default

		this->activate(THR_NEW_LWP | THR_JOINABLE, n_thread);
	}
	while(0);

	return error;
}

void
SSL_Server::stop()
{
	// stop all io_service thread(s)
	this->ios_.stop();

	// wait for thread(s) to terminate
	this->wait();
	
	// then, close all connections
	this->clear_all_connections();
}

void
SSL_Server::handle_accept(const error_code& error)
{
	if ( !error )
	{
		std::string remote_addr(conn_->socket().remote_endpoint().address().to_string());
		unsigned short remote_port = conn_->socket().remote_endpoint().port();
		if ( this->create_connection(conn_, remote_addr.c_str()) )
			conn_->open(this->ssl_connect(), remote_addr.c_str(), remote_port);

		// prepare connection for accept()
		conn_.reset(this->make_connection());
		acceptor_.async_accept(conn_->socket(), boost::bind(
			&SSL_Server::handle_accept,
			this,
			placeholders::error));
	}
}

size_t
SSL_Server::n_connection(const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	if ( !key ) return pool_.size();

	size_t n_conn = 0;
	CONNECTIONS::iterator iter = pool_.find(key);
	if ( iter != pool_.end() )
	{
		n_conn = 1;
		++iter;
		while( iter != pool_.end() )
		{
			if ( iter->first != key ) break;
			++n_conn;
			++iter;
		}
	}
	return n_conn;
}

int
SSL_Server::is_connected_from(const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	return (pool_.find(key) == pool_.end())?0:1;
}

SSL_Server_Connection*
SSL_Server::create_connection(SSL_Server_Connection::sptr conn, const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	// check max connection, 0 for no limit
	if ( MAX_CONNECTION && pool_.size() >= MAX_CONNECTION ) return 0;
	
	// insert connection
	SSL_Server_Connection* c = conn.get();
	pool_.insert(std::make_pair(key, c));

	return c;
}

void
SSL_Server::destroy_connection(SSL_Server_Connection::sptr conn)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	// remove it from pool
	SSL_Server_Connection* c = conn.get();
	for(CONNECTIONS::iterator iter = pool_.begin();
		iter != pool_.end();
		++iter)
	{
		if ( c == iter->second )
		{
			//ACE_OS::printf("destroy:%p\n", iter->second); //@
			pool_.erase(iter);
			break;
		}
	}
}

void
SSL_Server::clear_all_connections()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	pool_.clear();
}

} // namespace asio


