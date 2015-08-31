#include "aos/TCP_Client.h"

namespace asio {

TCP_Client::TCP_Client()
:
ssl_context_(ios_, ssl::context::sslv23),
is_running_(0),
n_task_(0),
n_max_conn_(0),
n_max_reuse_(0)
{
}

TCP_Client::~TCP_Client()
{
	this->stop();
	this->clear_all_reuse_connections();
}

asio::error_code
TCP_Client::prepare_ssl_context(ssl::context& ssl_context)
{
	asio::error_code error;

	// no need to prepare ssl context with client
	/*
	do
	{
		ssl_context.set_options(ssl::context::default_workarounds, error); if ( error ) break;
		ssl_context.use_certificate_file("server.pem", asio::ssl::context::pem, error); if ( error ) break;
		ssl_context.use_private_key_file("server.key", asio::ssl::context::pem, error); if ( error ) break;
	}
	while(0);
	//*/

	return error;
}

int
TCP_Client::svc()
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

TCP_Client_Connection*
TCP_Client::make_connection()
{
	return new (std::nothrow) TCP_Client_Connection(*this);
}

void
TCP_Client::unmake_connection(TCP_Client_Connection* conn)
{
	delete conn;
}

asio::error_code
TCP_Client::start(size_t n_thread)
{
	asio::error_code lock_error(-1, error::system_category);

	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, client_lock_, lock_error);

	asio::error_code error;

	this->prepare_ssl_context(this->ssl_context());

	do
	{
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
TCP_Client::stop()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, client_lock_);

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

	// reset io_service for next run()
	this->ios_.reset();
}

int
TCP_Client::resize_thread_pool(size_t n_thread)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, client_lock_, -1);

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
			this->ios_.post(boost::bind(&TCP_Client::ios, this));
			if ( this->thr_count() <= n_thread )
				break;
		}
	}

	return 0;
}

//-- connect

int
TCP_Client::connect(bool ssl_connect, unsigned short port, const char* ip_addr, long connect_timeout, long socket_timeout, long ssl_handshake_timeout, size_t buf_size)
{
	ip::tcp::endpoint remote_endpoint(ip::address::from_string(ip_addr), port);

	return this->connect(ssl_connect, remote_endpoint, connect_timeout, socket_timeout, ssl_handshake_timeout, buf_size);
}

int
TCP_Client::connect(bool ssl_connect, const ip::tcp::endpoint& remote_endpoint, long connect_timeout, long socket_timeout, long ssl_handshake_timeout, size_t buf_size)
{
	do
	{
		// check max connection, 0 for no limit, -1 for refuse to connect
		long max_conn = this->max_connection();
		if ( max_conn && ((long) this->n_connection()) >= max_conn )
			break;

		TCP_Client_Connection* conn = this->create_connection();
		if ( !conn )
			break;

		// insert connection into pool
		this->on_create_connection(*conn, remote_endpoint);
		// ready to connect
		conn->connect(ssl_connect, remote_endpoint, connect_timeout, socket_timeout, ssl_handshake_timeout, buf_size);
		
		return 0;
	}
	while(0);

	//ACE_OS::printf("connect() failed!\n"); //@
	return -1;
}

size_t
TCP_Client::n_connection()
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
TCP_Client::on_create_connection(TCP_Client_Connection& conn, const ip::tcp::endpoint& remote_endpoint)
{	
	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	// insert connection
	pool_.insert(std::make_pair(remote_endpoint.address().to_string(), &conn));
}

void
TCP_Client::on_destroy_connection(TCP_Client_Connection& conn)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	// remove it from pool
	TCP_Client_Connection* c = &conn;
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
TCP_Client::close_all_connections()
{
	asio::error_code error;

	// close all connections
	ACE_GUARD(ACE_Thread_Mutex, guard, pool_lock_);

	for(CONNECTIONS::iterator iter = pool_.begin();
		iter != pool_.end();
		++iter)
	{
		(iter->second)->socket().close(error);
	}
}

TCP_Client_Connection*
TCP_Client::create_connection()
{
	TCP_Client_Connection* conn = 0;
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
TCP_Client::destroy_connection(TCP_Client_Connection* conn)
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
TCP_Client::on_reuse_connection(TCP_Client_Connection& conn)
{
	//ACE_OS::printf("reuse connection!\n"); //@
}

size_t
TCP_Client::n_reuse_connection()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, reuse_lock_, 0);
	return reuse_.size();
}

void
TCP_Client::max_reuse_connection(size_t n_max_reuse)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, reuse_lock_);
	n_max_reuse_ = n_max_reuse;
}

size_t
TCP_Client::max_reuse_connection()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, reuse_lock_, 0);
	return n_max_reuse_;
}

void
TCP_Client::clear_all_reuse_connections()
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
