#include "SMTP_Mailer.h"

#include "SMTP_Mailer_Domain_Queue.h"

namespace asio {

SMTP_Mailer::SMTP_Mailer(io_service& ios)
:
//stop_(0),
ios_(ios)
{
}

SMTP_Mailer::~SMTP_Mailer()
{
	this->stop();
}

int
SMTP_Mailer::svc()
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
SMTP_Mailer::start(int n_thread)
{
	//this->stop_ = 0;
	if ( n_thread < 1 )
		n_thread = ACE_OS::num_processors_online(); //? * 4 for default

	this->activate(THR_NEW_LWP | THR_JOINABLE, n_thread);
}

void
SMTP_Mailer::stop()
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

int
SMTP_Mailer::connect(const char* ip_addr, unsigned short port, long timeout, SMTP_Mailer_Domain_Queue* queue)
{
	std::string key(ip_addr);
	SMTP_Mailer_Connection* conn = this->create_connection(key.c_str(), queue);
	if ( conn )
	{
		conn->connect(ip_addr, port, timeout); // connect() must fire after create_connection()
		return 0;
	}

	//ACE_OS::printf("connect() failed!\n"); //@
	return -1;
}

size_t
SMTP_Mailer::n_connection(const char* key)
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
SMTP_Mailer::is_connected_to(const char* key)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	return (pool_.find(key) == pool_.end())?0:1;
}

SMTP_Mailer_Connection*
SMTP_Mailer::create_connection(const char* key, SMTP_Mailer_Domain_Queue* queue)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	// check max connection, 0 for no limit
	if ( MAX_CONNECTION && pool_.size() >= MAX_CONNECTION ) return 0;

	// create connection
	SMTP_Mailer_Connection* conn = new SMTP_Mailer_Connection(this->ios_, this, queue);
	if ( !conn ) return 0;
	
	// insert connection
	pool_.insert(std::make_pair(key, conn));
	if ( conn->queue() ) ++(conn->queue()->n_ref_conn_); // increase domain queue reference count
	
	//ACE_OS::printf("create:%p\n", conn); //@

	return conn;
}

void
SMTP_Mailer::destroy_connection(SMTP_Mailer_Connection* conn)
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
	if ( conn->queue() ) --(conn->queue()->n_ref_conn_); // decrease domain queue reference count
	delete conn;
}

void
SMTP_Mailer::clear_all_connections()
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
