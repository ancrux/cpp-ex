#include "AIO_Connector.h"

AIO_Connector::AIO_Connector(Proactor_TaskPool& task)
:
task_(task),
n_op_c_(0),
is_cancelling_(0),
arr_conn_(0),
curr_conn_(0),
max_conn_(MAX_SESSION),
timer_id_(-1),
timeout_(ACE_Time_Value::zero)
{
	size_t size = max_conn_ * sizeof(AIO_Session*);
	arr_conn_ = (AIO_Session**) ACE_OS::malloc(size);
	if ( arr_conn_ )
		ACE_OS::memset(arr_conn_, 0, size);
	else
		is_cancelling_ = 1;
}

AIO_Connector::~AIO_Connector()
{
	cancel_all_session(5000);
	ACE_OS::free(arr_conn_);
}

int
AIO_Connector::cancel_all_session(int msec_to_wait)
{
	int rc = -1;
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);

	this->cancel_timer();

	int n_cancel = 0;
	if ( curr_conn_ > 0 )
	{
		AIO_Session** ptr = 0;
		for(int i=0; i < max_conn_; ++i)
		{
			ptr = arr_conn_ + i;
			if ( *ptr )
			{
				(*ptr)->cancel();
				++n_cancel;
			}
		}
		rc = n_cancel;
	}

	int i = 0;
	while( curr_conn_ > 0 && i < msec_to_wait ) // wait for 5 sec
	{
		ACE_Time_Value tv(0, 1000);
		timespec_t t  = (timespec_t) tv;
		ACE_OS::nanosleep(&t);
		++i;
	}

	return rc;
}

int
AIO_Connector::start()
{
	int rc = -1;
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);

	if ( this->is_cancelling_ == 1 ) return rc;

	if ( TRB_Asynch_Connector< AIO_Session >::open (1,  // int pass_addresses = 0,
		task_.get_proactor(0), //TRB_Proactor *proactor = 0,
		1    // int validate_new_connection = 0 );
		) != 0 )
	{
		ACE_ERROR((LM_ERROR, ACE_TEXT ("(%t) Connector %p\n"), ACE_TEXT ("open failed")));
		return rc;
	}

	rc = 0;
	this->start_timer();
	
	return rc;
}

int
AIO_Connector::connect(const ACE_INET_Addr& addr, int n_connect)
{
	int rc = -1;
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);

	if ( this->is_cancelling_ == 1 ) return rc;

	//if ( this->open (1,  // int pass_addresses = 0,
	//	task_.get_proactor(0), //TRB_Proactor *proactor = 0,
	//	1    // int validate_new_connection = 0 );
	//	) != 0 )
	//{
	//	ACE_ERROR((LM_ERROR, ACE_TEXT ("(%t) Connector %p\n"), ACE_TEXT ("open failed")));
	//	return rc;
	//}

	if ( n_connect < 1 ) n_connect = 1;
	if ( n_connect > max_conn_ - curr_conn_ ) n_connect = max_conn_ - curr_conn_;
	for(rc = 0; rc < n_connect; ++rc)
	{
		//int connect (const ACE_INET_Addr &remote_sap,
		//             const ACE_INET_Addr &local_sap =
		//                          ACE_INET_Addr ((u_short)0),
		//                 int reuse_addr = 1,
		//                 const void *act = 0);

		if ( TRB_Asynch_Connector< AIO_Session >::connect(addr,
			ACE_INET_Addr ((u_short)0),
			1,
			(const void*) &rc) != 0)
		{
			//getchar(); //@
			ACE_ERROR((LM_ERROR, ACE_TEXT ("(%t) Connector %p\n"), ACE_TEXT ("connect failed")));
			ACE_DEBUG((LM_ERROR, ACE_TEXT("(%t) AIO_Connector (curr: %d)\n"), this->n_session())); //@
			break;
		}
		++this->n_op_c_;
	}

	return rc;
}

int
AIO_Connector::validate_connection(const TRB_Asynch_Connect::Result& result, const ACE_INET_Addr& remote, const ACE_INET_Addr& local)
{
	int rc = -1;
	ACE_GUARD_RETURN (ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);

	--this->n_op_c_;

	if ( !result.success() )
	{
		//getchar(); //@
		ACE_DEBUG((LM_ERROR, ACE_TEXT("(%t) Connector: Connect failed err=%d\n"), (int) result.error()));
		ACE_DEBUG((LM_ERROR, ACE_TEXT("(%t) AIO_Connector (curr: %d)\n"), this->n_session())); //@
		
		rc = -1;
	}
	else if ( is_cancelling_ )
	{
		rc = -1;
	}
	else
	{
		rc = 0;
	}

	if ( this->is_safe_to_delete() )
		this->task_.signal_main();

	return rc;
}

int
AIO_Connector::cancel()
{
	int rc = -1;
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);

	this->is_cancelling_ = 1;
	// cancel outstanding connects
	return this->TRB_Asynch_Connector< AIO_Session >::cancel();    
}

AIO_Session*
AIO_Connector::make_handler()
{
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, 0);
	//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Connector create_session(curr: %d)\n"), this->n_session())); //@

	if ( curr_conn_ >= max_conn_ || is_cancelling_ ) return 0;

	// insert into arr_conn_
	AIO_Session** ptr = 0;
	for(int i=0; i < max_conn_; ++i)
	{
		ptr = arr_conn_ + (curr_conn_ + i) % max_conn_;
		if ( *ptr == 0 )
		{
			AIO_Session* session = this->make_session();
			if ( session == 0 ) return 0;
			
			*ptr = session;
			++curr_conn_;
			// initialize session
			session->manager(this);
			session->index(i);
			session->set_timeout(timeout_);
			this->on_session_create(*session);

			return session;
		}
	}

	return 0;
}

AIO_Session*
AIO_Connector::make_session()
{
	return new AIO_Session();
}

int
AIO_Connector::destroy_session(AIO_Session* session)
{
	int rc = -1;
	if ( session == 0 ) return rc;

	int index = session->index();

	AIO_Session** ptr = arr_conn_ + index;
	if ( *ptr == session )
	{
		rc = destroy_session(index);
	}
	else
	{
		for(int i=0; i < max_conn_; ++i)
		{
			ptr = arr_conn_ + i;

			ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);
			if ( *ptr == session )
			{
				this->on_session_destroy(*session);
				delete session;
				*ptr = 0;
				--curr_conn_;
				rc = 0;

				break;
			}
		}
	}

	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Connector delete_session(curr: %d)\n"), this->n_session())); //@
	return rc;
}

int
AIO_Connector::destroy_session(int i)
{
	int rc = -1;
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, rc);
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Connector delete_session(curr: %d)\n"), this->n_session())); //@

	AIO_Session** ptr = arr_conn_ + i;
	if ( *ptr )
	{
		this->on_session_destroy(**ptr);
		delete *ptr;
		*ptr = 0;
		--curr_conn_;
		rc = 0;
	}

	return rc;
}

int
AIO_Connector::n_session()
{
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, -1);
	return curr_conn_;
}

int
AIO_Connector::max_session()
{
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, -1);
	return max_conn_;
}

void
AIO_Connector::max_session(int max_conn)
{
	ACE_GUARD(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_);

	if ( max_conn > this->max_conn_ )
	{
		AIO_Session** new_arr = (AIO_Session**) ACE_OS::realloc(arr_conn_, max_conn * sizeof(AIO_Session*));
		if ( new_arr )
		{
			arr_conn_ = new_arr;
			ACE_OS::memset(arr_conn_ + this->max_conn_, 0, (max_conn - this->max_conn_) * sizeof(AIO_Session*));
			max_conn_ = max_conn;
		}
	}
	else
	{
		max_conn_ = max_conn;
	}
}

void
AIO_Connector::handle_time_out(const ACE_Time_Value& tv, const void* pArg)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Connector handle_timeout(curr: %d)\n"), this->n_session())); //@
	if ( curr_conn_ > 0 )
	{
		AIO_Session** ptr = 0;
		for(int i=0; i < max_conn_; ++i)
		{
			ptr = arr_conn_ + i;
			ACE_GUARD(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_);
			if ( *ptr && (*ptr)->has_pending_io() ) (*ptr)->check_timeout();
		}
	}

	ACE_GUARD(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_);
	this->timer_id_ = -1;
	this->start_timer();
}

void
AIO_Connector::start_timer()
{
	if ( this->is_cancelling_ )
	{
		if ( this->is_safe_to_delete() )
			this->task().signal_main();
		return;
	}

	if ( this->timer_id_ != -1 || this->timeout_ == ACE_Time_Value::zero )
		return;

	TRB_Proactor* proactor = this->task().get_proactor(0);
	if ( proactor == 0 ) return;

	ACE_Time_Value abs_time = this->timeout_;
 	this->timer_id_ = proactor->schedule_timer(*this, 0, abs_time);
}

void
AIO_Connector::cancel_timer()
{
	if ( timer_id_ != -1 )
	{
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Connector cancel_timer\n"))); //@

		TRB_Proactor* proactor = this->task().get_proactor(0);
		if ( proactor == 0 ) return;

		int rc = proactor->cancel_timer(this->timer_id_);
		if ( rc != 0 ) this->timer_id_ = -1;
	}
}

const ACE_Time_Value&
AIO_Connector::get_timeout()
{
	ACE_GUARD_RETURN(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_, ACE_Time_Value::zero);
	return this->timeout_;
}

void
AIO_Connector::set_timeout(const ACE_Time_Value& timeout)
{
	ACE_GUARD(ACE_SYNCH_RECURSIVE_MUTEX, monitor, this->lock_);

	this->timeout_ = timeout;
	this->start_timer();
}

void
AIO_Connector::on_session_create(AIO_Session& session)
{
}

void
AIO_Connector::on_session_destroy(AIO_Session& session)
{
}
