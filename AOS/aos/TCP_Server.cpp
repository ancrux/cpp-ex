#include "aos/TCP_Server.h"

namespace asio {

TCP_Server::TCP_Server()
:
acceptor_(ios_),
ssl_context_(ios_, ssl::context::sslv23),
conn_(0),
is_ssl_connect_(0),
is_running_(0),
n_task_(0),
n_max_conn_(0),
n_max_reuse_(0)
{
}

TCP_Server::~TCP_Server()
{
	this->stop();
	this->clear_all_reuse_connections();
}

asio::error_code
TCP_Server::prepare_ssl_context(ssl::context& ssl_context)
{
	asio::error_code error;
	do
	{
		ssl_context.set_options(ssl::context::default_workarounds, error); if ( error ) break;
		ssl_context.use_certificate_file("server.pem", asio::ssl::context::pem, error); if ( error ) break;
		ssl_context.use_private_key_file("server.key", asio::ssl::context::pem, error); if ( error ) break;
	}
	while(0);

	return error;
}

int
TCP_Server::svc()
{
	long task_id = 0;
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, task_lock_, -1);
		++n_task_;
		task_id = n_task_;
	}
	if ( task_id < 1 ) return -1;
	//ACE_OS::printf("thr(%u) start...=>\t%d\n", ACE_OS::thr_self(), task_id); //@

#if ( USE_NEDMALLOC == 1 )
	//ACE_OS::printf("thr(%u) cleanup nedmalloc threadcache\n", ACE_OS::thr_self());
	//nedalloc::neddisablethreadcache(0); // cause problem in DEBUG mode
#endif

	asio::error_code error;

	io_service::work work(ios_);
	while(true)
	{
		if ( 0 == ios_.run_one(error) )
		{
			//ACE_OS::printf("run_one(): task_id=>\t%d\n", task_id); //@
			break;
		}
		if ( task_id > n_wish_task_.value() )
		{
			//ACE_OS::printf("id > wish: task_id=>\t%d\n", task_id); //@
			break;
		}
	}

#if ( USE_NEDMALLOC == 1 )
	ACE_OS::printf("thr(%u) cleanup nedmalloc threadcache\n", ACE_OS::thr_self());
	nedalloc::neddisablethreadcache(0);
#endif

	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, task_lock_, -1);
		--n_task_;
	}
	//ACE_OS::printf("thr(%u) stop...=>\t%d\n", ACE_OS::thr_self(), task_id); //@

	return 0;
}

TCP_Server_Connection*
TCP_Server::make_connection()
{
	return new (std::nothrow) TCP_Server_Connection(*this);
}

void
TCP_Server::unmake_connection(TCP_Server_Connection* conn)
{
	delete conn;
}

asio::error_code
TCP_Server::start(unsigned short port, const char* ip_addr, size_t n_thread)
{
	asio::error_code lock_error(-1, error::system_category);

	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, server_lock_, lock_error);

	asio::error_code error;

	// initialize ssl context
	this->prepare_ssl_context(this->ssl_context());

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
		conn_ = this->create_connection();
		acceptor_.async_accept(conn_->socket(), boost::bind(
			&TCP_Server::handle_accept,
			this,
			placeholders::error));
		
		if ( n_thread < 1 )
			n_thread = ACE_OS::num_processors_online(); //? * 4 for default

		n_wish_task_ = n_thread;
		if ( this->activate(THR_NEW_LWP | THR_DETACHED, n_thread) != -1 )
			is_running_ = 1;
	}
	while(0);

	return error;
}

void
TCP_Server::handle_accept(const error_code& error)
{
	if ( !error )
	{
		// check max connection, 0 for no limit, -1 for refuse any new connections
		long max_conn = this->max_connection();
		if ( !max_conn || ((long) this->n_connection()) < max_conn )
		{
			// insert connection into pool
			this->on_create_connection(*conn_, (*conn_).socket().remote_endpoint());
			// connection starts here
			conn_->open(this->ssl_connect());
		}
		else
		{
			if ( conn_ )
			{
				delete conn_;
				conn_ = 0;
			}
		}

		// prepare connection for accept()
		conn_ = this->create_connection();
		acceptor_.async_accept(conn_->socket(), boost::bind(
			&TCP_Server::handle_accept,
			this,
			placeholders::error));
	}
	else if ( is_running_.value() )
	{
		// in the following situation, accept() may fail
		// "too many open files" if ulimit -n is too small on linux
		// "I/O operation cancelled" if worker thread is terminated on windows
		//ACE_OS::printf("accept() failed but continued: %s\n", error.message().c_str()); //@

		if ( conn_ )
		{
			delete conn_;
			conn_ = 0;
		}

		// prepare connection for accept()
		conn_ = this->create_connection();
		acceptor_.async_accept(conn_->socket(), boost::bind(
			&TCP_Server::handle_accept,
			this,
			placeholders::error));
	}
	else
	{
		// triggered when acceptor.close()
		// on_accept_error()
		//ACE_OS::printf("accept() failed: %s\n", error.message().c_str()); //@
	}
}

void
TCP_Server::stop()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, server_lock_);

	is_running_ = 0;

	// stop and quit all io_service thread(s)
	this->ios_.stop();
	this->thr_mgr()->wait(); // wait for both detached & joinable thread
	//this->wait(); // only wait for joinable thread
	
	// close all connections
	this->close_all_connections();

	// run io_service again to finish incompleted operations
	//ACE_OS::printf("before closing n_conn: %d\n", this->n_connection()); //@
	asio::error_code error;
	this->ios_.reset();
	while(true)
	{
		if ( 0 == this->ios_.run_one(error) &&
			0 == this->n_connection() )
			break;
	}
	this->ios_.stop();
	this->wait();

	// delete new connection waiting for accept
	if ( conn_ )
	{
		delete conn_;
		conn_ = 0;
	}

	// reset io_service for next run()
	this->ios_.reset();
}

int
TCP_Server::resize_thread_pool(size_t n_thread)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, server_lock_, -1);

	if ( is_running_.value() == 0 || n_thread < 1 )
		return -1;

	n_wish_task_ = n_thread; //ACE_OS::printf("wish_thread:%d\n", n_wish_task_.value()); //@
	size_t n_running = this->thr_count();
	if ( n_thread == n_running )
	{
	}
	else if ( n_thread > n_running )
	{
		int res = this->activate(THR_NEW_LWP | THR_DETACHED, n_thread - n_running, 1);
		if ( -1 == res )
			return -1;
		while(true)
		{
			if ( this->thr_count() >= n_thread )
				break;
		}
	}
	else
	{
		// don't need to wait() if detached thread
		while(true)
		{
			this->ios_.post(boost::bind(&TCP_Server::ios, this));
			if ( this->thr_count() <= n_thread )
				break;
		}
	}

	return 0;
}

size_t
TCP_Server::n_connection()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, pool_lock_, 0);

	return pool_.size();

	/*
	if ( key )
	{
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
	//*/
}

void
TCP_Server::on_create_connection(TCP_Server_Connection& conn, const ip::tcp::endpoint& remote_endpoint)
{	
	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	// insert connection
	pool_.insert(std::make_pair(remote_endpoint.address().to_string(), &conn));
}

void
TCP_Server::on_destroy_connection(TCP_Server_Connection& conn)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	// remove it from pool
	TCP_Server_Connection* c = &conn;
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
TCP_Server::close_all_connections()
{
	asio::error_code error;

	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	// close acceptor
	acceptor_.close(error);

	// close all connections
	for(CONNECTIONS::iterator iter = pool_.begin();
		iter != pool_.end();
		++iter)
	{
		(iter->second)->socket().close(error);
	}
}

TCP_Server_Connection*
TCP_Server::create_connection()
{
	TCP_Server_Connection* conn = 0;
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, reuse_lock_, 0);

		if ( !reuse_.empty() )
		{
			conn = reuse_.back();
			reuse_.pop_back();
		}

		// no mutex required
		if ( conn )
		{
			conn->reset_socket();
			this->on_reuse_connection(*conn);
		}
		else
			conn = this->make_connection();

		return conn;
	}
}

void
TCP_Server::destroy_connection(TCP_Server_Connection* conn)
{
	// remove connection from pool
	this->on_destroy_connection(*conn);

	// delete connection
	{
		ACE_GUARD(ACE_Thread_Mutex, guard, reuse_lock_);
		if ( reuse_.size() < n_max_reuse_ )
		{
			reuse_.push_back(conn);
			conn = 0;
		}

		// no mutex required
		if ( conn )
			this->unmake_connection(conn);
	}
}

void
TCP_Server::on_reuse_connection(TCP_Server_Connection& conn)
{
	//ACE_OS::printf("reuse connection!\n"); //@
}

size_t
TCP_Server::n_reuse_connection()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, reuse_lock_, 0);
	return reuse_.size();
}

void
TCP_Server::max_reuse_connection(size_t n_max_reuse)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, reuse_lock_);
	n_max_reuse_ = n_max_reuse;
}

size_t
TCP_Server::max_reuse_connection()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, reuse_lock_, 0);
	return n_max_reuse_;
}

void
TCP_Server::clear_all_reuse_connections()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, reuse_lock_);
	for(REUSE_CONNECTIONS::iterator iter = reuse_.begin();
		iter != reuse_.end();
		++iter)
	{
		delete *iter;
	}
	reuse_.clear();
}

} // namespace asio
