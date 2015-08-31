#include "SMTP_Mailer_Job_Queue.h"

namespace asio {

SMTP_Mailer_Job_Queue::SMTP_Mailer_Job_Queue()
{
}

SMTP_Mailer_Job_Queue::~SMTP_Mailer_Job_Queue()
{
	this->clear_all_domain_queues();
}

int
SMTP_Mailer_Job_Queue::create_domain_queue(const char* domain)
{
	std::string key(domain); aos::tolower(key);
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, 0);

	DOMAIN_QUEUES::iterator iter = queues_.find(key);
	if ( iter != queues_.end() ) return 2; // already exists

	SMTP_Mailer_Domain_Queue* queue = new SMTP_Mailer_Domain_Queue();
	if ( queue )
	{
		queues_.insert(std::make_pair(key, queue));
		return 1; // create ok
	}

	return 0; // create failed
}

void
SMTP_Mailer_Job_Queue::destroy_domain_queue(const char* domain)
{
	std::string key(domain); aos::tolower(key);
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	DOMAIN_QUEUES::iterator iter = queues_.find(key);
	if ( iter != queues_.end() )
	{
		delete iter->second;
		queues_.erase(iter);
	}
}

void
SMTP_Mailer_Job_Queue::clear_all_domain_queues()
{
	ACE_GUARD(ACE_Thread_Mutex, guard, lock_);

	for(DOMAIN_QUEUES::iterator iter = queues_.begin();
		iter != queues_.end();
		++iter)
	{
		delete iter->second;
	}
	queues_.clear();
}

} // namespace asio
