#ifndef _AIO_CONNECTOR_H_
#define _AIO_CONNECTOR_H_

#include "Proactor_TaskPool.h"
#include "AIO_Session.h"

class AIO_Connector : public TRB_Asynch_Connector< AIO_Session >, public AIO_Session_Manager
{
public:
	static const int MAX_SESSION = 64;

public:
	AIO_Connector(Proactor_TaskPool& task);
	virtual ~AIO_Connector();

public:
	Proactor_TaskPool& task() const { return task_; };

public:
	int start();
	int connect(const ACE_INET_Addr& addr, int n_connect = 1);
	int is_safe_to_delete() { return (this->n_op_c_ == 0); }
	int cancel_all_session(int msec_to_wait = 0);
	virtual AIO_Session* make_session();
	virtual void on_session_create(AIO_Session& session); // initialize your own session
	virtual void on_session_destroy(AIO_Session& session);
	int destroy_session(int i);
	int destroy_session(AIO_Session* session);
	int n_session();
	int max_session();
	void max_session(int max_conn);

// inherited from TRB_Asynch_Connector
public:
	virtual int cancel(); // cancel all accepts
	virtual int validate_connection(const TRB_Asynch_Connect::Result& result, const ACE_INET_Addr& remote, const ACE_INET_Addr& local);
	virtual AIO_Session* make_handler();

// inherited from TRB_Handler
public:
	void handle_time_out(const ACE_Time_Value& tv, const void* pArg);

public:
	void start_timer();
	void cancel_timer();
	void set_timeout(const ACE_Time_Value& timeout);
	const ACE_Time_Value& get_timeout();

protected:
	Proactor_TaskPool& task_; // proactor task pool to drive server working
	
	AIO_Session** arr_conn_; // array to store all connection
	int curr_conn_; // # of current connection
	int max_conn_; // # of max connection

	int n_op_c_; // # of pending connection
	int is_cancelling_; // 1 for cancelling

	long timer_id_;
	ACE_Time_Value timeout_;

	ACE_SYNCH_RECURSIVE_MUTEX lock_;
};

template< class AIO_Session_T >
class AIO_Connector_T : public AIO_Connector
{
public:
	AIO_Connector_T(Proactor_TaskPool& task);
	virtual ~AIO_Connector_T();
	virtual AIO_Session_T* make_session();
};

template< class AIO_Session_T >
AIO_Connector_T< AIO_Session_T >::AIO_Connector_T(Proactor_TaskPool& task)
:
AIO_Connector(task)
{
}

template< class AIO_Session_T >
AIO_Connector_T< AIO_Session_T >::~AIO_Connector_T()
{
}

template< class AIO_Session_T >
AIO_Session_T*
AIO_Connector_T< AIO_Session_T >::make_session()
{
	return new AIO_Session_T();
}

#endif // _AIO_CONNECTOR_H_
