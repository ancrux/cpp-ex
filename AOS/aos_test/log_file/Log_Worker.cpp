#include "Log_Worker.h"

#include <cassert>

namespace mozart {

Log_Worker::Log_Worker()
:
store_(0),
monitor_(0)
{
}

Log_Worker::~Log_Worker()
{
}

int
Log_Worker::svc()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT ("(%t) Worker starting up \n"))); //@
	assert(this->store_ != 0);
	assert(this->monitor_ != 0);

	std::string* mb;

	while(this->msg_queue()->dequeue_head(mb) != -1)
	{
		//if ( ACE_OS::strlen(mb->base()) == 0 )
		if ( mb->size() == 0 )
		{
			ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Worker shutting down\n"))); //@
			break;
		}

		// log message to log_store
		this->store_->log(*mb);

		// send message to log_monitor
		this->monitor_->msg_queue()->enqueue_tail(mb);
	}

	return 0;
}

} // namespace mozart