#include "Log_Monitor.h"

#include <cassert>

namespace mozart {

int
Log_Monitor::svc()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT ("(%t) Monitor starting up \n"))); //@

	std::string* mb;

	while(this->msg_queue()->dequeue_head(mb) != -1)
	{
		//if ( ACE_OS::strlen(mb->base()) == 0 )
		if ( mb->size() == 0 )
		{
			ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Monitor shutting down\n"))); //@
			break;
		}

		// process message

		
		// delete log message after processing
		delete mb;
	}

	return 0;
}


int
Log_Recycler::svc()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT ("(%t) Recycler starting up \n"))); //@

	pool_.high_water_mark(-1);

	ACE_Message_Block* mb;

	while(1)
	{
		//ACE_Time_Value tv = ACE_OS::gettimeofday();
		//time_t tt = ACE_OS::time();

		//ACE_Message_Block* mb = new ACE_Message_Block(128);
		//char* cstr = new char[40];
		//std::string* str = new std::string(128, 0);

		//ACE_Message_Block* mb;// = recycler.get_mb();
		////if ( mb == 0 ) 
		//	mb = new ACE_Message_Block(10);
		//recycler.msg_queue()->enqueue_tail(mb);

		//ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, mtx, -1);
		//ACE_GUARD(ACE_Thread_Mutex, ace_mon, mtx);

		queue_.enqueue(0);
	}

	while(this->msg_queue()->dequeue_head(mb) != -1)
	{
		//if ( ACE_OS::strlen(mb->base()) == 0 )
		if ( mb->size() == 0 )
		{
			ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Recycler shutting down\n"))); //@
			break;
		}

		// process message
		queue_.enqueue(0);
		
		// delete log message after processing
		delete mb;

		//if ( pool_.message_count() >= 10000 )
		//	delete mb;
		//else
		//	pool_.enqueue_tail(mb);
	}

	return 0;
}

size_t
MyQueue::size()
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, mon, lock_, 0);
	size_t cnt = list_.size();
	return cnt;
}

void
MyQueue::enqueue(std::string* str)
{
	ACE_GUARD(ACE_Thread_Mutex, mon, lock_);

	list_.insert(list_.begin(), str);
}

} // namespace mozart