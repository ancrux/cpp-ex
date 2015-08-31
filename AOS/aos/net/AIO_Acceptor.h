#ifndef _AIO_ACCEPTOR_H_
#define _AIO_ACCEPTOR_H_

#include "AIO_Session.h"
#include "Proactor_TaskPool.h"

/*// Usage: can be inherited or use AIO_Acceptor_T<> directly

//*/

class AIO_Acceptor : public TRB_Asynch_Acceptor< AIO_Session >, public AIO_Session_Manager
{
public:
	static const int MAX_SESSION = 1024;
	static const int INITIAL_ACCEPT = 128;

public:
	AIO_Acceptor(Proactor_TaskPool& task);
	virtual ~AIO_Acceptor();

public:
	Proactor_TaskPool& task() const { return task_; };

public:
	int start(const ACE_INET_Addr& addr, int n_accept = INITIAL_ACCEPT);
	int is_safe_to_delete() { return (this->n_op_a_ == 0); };
	int cancel_all_session(int msec_to_wait = 0);

public:
	virtual int on_validate_address(const ACE_INET_Addr& remote, const ACE_INET_Addr& local);
	//? virtual int on_accept_error();

public:
	virtual AIO_Session* make_session(); // overwite this to create your own session
	virtual void on_session_create(AIO_Session& session); // initialize your own session
	virtual void on_session_destroy(AIO_Session& session);
	int destroy_session(int i);
	int destroy_session(AIO_Session* session);
	int n_session();
	int max_session();
	void max_session(int max_conn);

// inherited from TRB_Asynch_Acceptor
public:
	virtual int cancel(); // cancel all accepts
	virtual AIO_Session* make_handler();
	virtual int should_reissue_accept();
	virtual int validate_connection(const TRB_Asynch_Accept::Result& result, const ACE_INET_Addr& remote, const ACE_INET_Addr& local);

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

	int n_op_a_; // # of pending accept operations
	int is_cancelling_; // 1 for cancelling
	int n_initial_accept_; // # of initial accepts

	long timer_id_;
	ACE_Time_Value timeout_;

	ACE_SYNCH_RECURSIVE_MUTEX lock_;
};

template< class AIO_Session_T >
class AIO_Acceptor_T : public AIO_Acceptor
{
public:
	AIO_Acceptor_T(Proactor_TaskPool& task);
	virtual ~AIO_Acceptor_T();
	virtual AIO_Session_T* make_session();
};

template< class AIO_Session_T >
AIO_Acceptor_T< AIO_Session_T >::AIO_Acceptor_T(Proactor_TaskPool& task)
:
AIO_Acceptor(task)
{
}

template< class AIO_Session_T >
AIO_Acceptor_T< AIO_Session_T >::~AIO_Acceptor_T()
{
}

template< class AIO_Session_T >
AIO_Session_T*
AIO_Acceptor_T< AIO_Session_T >::make_session()
{
	return new AIO_Session_T();
}

#endif // _AIO_ACCEPTOR_H_
