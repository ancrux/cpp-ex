#include "SMTP_Listener.h"

namespace asio {

SMTP_Listener::SMTP_Listener(io_service& ios)
:
//stop_(0),
ios_(ios),
acceptor_(ios)
{
}

SMTP_Listener::~SMTP_Listener()
{
	this->stop();
}

int
SMTP_Listener::svc()
{
	ACE_OS::printf("thr(%u) start...\n", ACE_OS::thr_self()); //@

	io_service::work work(ios_);

	//ACE_Time_Value sleep_tv; sleep_tv.set(0.001);
	while(1) // while( !stop_.value() )
	{
		ios_.run();
		break;

		//ACE_OS::sleep(sleep_tv);
		//ACE_OS::printf("====================\n");
	}

	ACE_OS::printf("thr(%u) stop...\n", ACE_OS::thr_self()); //@

	return 0;
}

void
SMTP_Listener::start(unsigned short port, const char* ip_addr, int n_thread)
{
	ip::tcp::endpoint local_addr(ip::tcp::v4(), port);
	if ( ip_addr )
		local_addr.address(ip::address::from_string(ip_addr));

	acceptor_.open(local_addr.protocol());
	acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(local_addr);
	int backlog = socket_base::max_connections;
	//ACE_OS::printf("backlog:%d\n", backlog); //@
	acceptor_.listen(backlog);

	// prepare connection for accept()
	SMTP_Listener_Connection* conn = new (std::nothrow) SMTP_Listener_Connection(this->ios_, this);
	acceptor_.async_accept(conn->socket(), boost::bind(
		&SMTP_Listener::handle_accept,
		this,
		placeholders::error,
		conn)); //? no strand wrap for async_accept()?
	
	if ( n_thread < 1 )
		n_thread = ACE_OS::num_processors_online(); //? * 4 for default

	this->activate(THR_NEW_LWP | THR_JOINABLE, n_thread);
}

void
SMTP_Listener::stop()
{
	// get thread count
	//size_t n_thread = this->thr_count();

	// stop all io_service thread(s)
	//this->stop_ = 1;
	this->ios_.stop();

	// wait for thread(s) to terminate
	this->wait();
	//for(size_t i=0; i < n_thread; ++i) this->wait();
	
	// then, close all connections
	this->clear_all_connections();
}

void
SMTP_Listener::handle_accept(const error_code& error, SMTP_Listener_Connection* conn)
{
	if ( conn )
	{
		if ( error ) delete conn;

		// insert new connection
		std::string remote_addr(conn->socket().remote_endpoint().address().to_string());
		unsigned short remote_port = conn->socket().remote_endpoint().port();
		if ( this->create_connection(conn, remote_addr.c_str()) )
			conn->accept(remote_addr.c_str(), remote_port);
		else
			delete conn;

		// prepare connection for accept()
		conn = new (std::nothrow) SMTP_Listener_Connection(this->ios_, this);
		acceptor_.async_accept(conn->socket(), boost::bind(
			&SMTP_Listener::handle_accept,
			this,
			placeholders::error,
			conn));
	}
}

size_t
SMTP_Listener::n_connection(const char* key)
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
SMTP_Listener::is_connected_from(const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	return (pool_.find(key) == pool_.end())?0:1;
}

SMTP_Listener_Connection*
SMTP_Listener::create_connection(SMTP_Listener_Connection* conn, const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	// check max connection, 0 for no limit
	if ( MAX_CONNECTION && pool_.size() >= MAX_CONNECTION ) return 0;
	
	// insert connection
	pool_.insert(std::make_pair(key, conn));
	//ACE_OS::printf("create:%p\n", conn); //@

	return conn;
}

void
SMTP_Listener::destroy_connection(SMTP_Listener_Connection* conn)
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	// remove it from pool
	for(CONNECTIONS::iterator iter = pool_.begin();
		iter != pool_.end();
		++iter)
	{
		if ( conn == iter->second )
		{
			//ACE_OS::printf("destroy:%p\n", iter->second); //@
			pool_.erase(iter);
			break;
		}
	}
	// delete connection
	delete conn;
}

void
SMTP_Listener::clear_all_connections()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	for(CONNECTIONS::iterator iter = pool_.begin();
		iter != pool_.end();
		++iter)
	{
		delete iter->second;
	}
	pool_.clear();
}

} // namespace asio
