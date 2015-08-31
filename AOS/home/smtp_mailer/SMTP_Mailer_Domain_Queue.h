#ifndef _ASIO_SMTP_MAILER_DOMAIN_QUEUE_H_
#define _ASIO_SMTP_MAILER_DOMAIN_QUEUE_H_

#include "SMTP_Mailer.h"
#include "SMTP_Mailer_Job.h"

#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"
#include "ace/Message_Queue_T.h"

#include <map>

namespace asio {

class SMTP_Mailer_Domain_Queue
{
friend class SMTP_Mailer;
	
public:
	typedef std::multimap< int, std::string > MX_RECORDS; // int: mx_preference, string: ip_address

public:
	SMTP_Mailer_Domain_Queue(SMTP_Mailer* mailer = 0);
	~SMTP_Mailer_Domain_Queue();

public:
	SMTP_Mailer* mailer() const { return mailer_; };

public:
	size_t count() { return queue_.message_count(); };
	int enqueue_head(SMTP_Mailer_Job* job) { return queue_.enqueue_head(job); };
	int enqueue_tail(SMTP_Mailer_Job* job) { return queue_.enqueue_tail(job); };
	int enqueue(SMTP_Mailer_Job* job) { return this->enqueue_tail(job); };
	int dequeue_head(SMTP_Mailer_Job*& job) { return queue_.dequeue_head(job); };
	int dequeue_tail(SMTP_Mailer_Job*& job) { return queue_.dequeue_tail(job); };
	int dequeue(SMTP_Mailer_Job*& job) { return this->dequeue_head(job); };
	
protected:
	void clear_reference_connections();

protected:
	ACE_Message_Queue_Ex< SMTP_Mailer_Job, ACE_MT_SYNCH > queue_;

	SMTP_Mailer* mailer_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_ref_conn_; // reference connection //?? atomic because mailer::destroy_connection() is called by multi-threads

	MX_RECORDS mx_; //mx records
	//n_max_conn_per_domain
	//n_max_rcpt_ok
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> n_max_rcpt_err_; // max # of rcpt error allowed per mail transaction
	//last_access_ // last operation time

	//ACE_Thread_Mutex lock_; //?+ instance lock // Message_Queue already has its own lock
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_DOMAIN_QUEUE_H_
