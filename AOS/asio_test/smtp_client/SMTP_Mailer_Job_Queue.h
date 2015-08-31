#ifndef _ASIO_SMTP_MAILER_JOB_QUEUE_H_
#define _ASIO_SMTP_MAILER_JOB_QUEUE_H_

#include "SMTP_Mailer_Job.h"
#include "SMTP_Mailer_Domain_Queue.h"

#include <map>

namespace asio {

//? or class SMTP_Mailer_Service

class SMTP_Mailer_Job_Queue
{
public:
	typedef std::map< std::string, SMTP_Mailer_Domain_Queue* > DOMAIN_QUEUES;

public:
	SMTP_Mailer_Job_Queue();
	~SMTP_Mailer_Job_Queue();

public:
	//+ void enqueue(SMTP_Mailer_Job* job);

protected:
	int create_domain_queue(const char* domain); // return 0 falied, 1 new, 2 existed 
	void destroy_domain_queue(const char* domain);
	void clear_all_domain_queues();

protected:
	DOMAIN_QUEUES queues_;
	ACE_Thread_Mutex lock_;
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_JOB_QUEUE_H_
