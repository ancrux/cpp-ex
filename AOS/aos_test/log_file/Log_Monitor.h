#ifndef _LOG_MONITOR_H_
#define _LOG_MONITOR_H_

#include "ace/OS.h"
#include "ace/Task_Ex_T.h"

#include <list>
#include <iostream>
using namespace std;

namespace mozart {

class Log_Monitor : public ACE_Task_Ex<ACE_MT_SYNCH, std::string>
{
public:
	virtual int svc();
};

class MyQueue
{
public:
	size_t size();
	//{
	//	ACE_GUARD_RETURN(ACE_Thread_Mutex, mon, lock_, 0);
	//	size_t cnt = list_.size();
	//	return cnt;
	//};
	void enqueue(std::string* str);

protected:
	std::list< std::string* > list_;
	ACE_Thread_Mutex lock_;
};

class Log_Recycler : public ACE_Task<ACE_MT_SYNCH>
{
public:
	virtual int svc();

public:
	ACE_Message_Block* get_mb()
	{
		if ( !pool_.is_empty() )
		{
			ACE_Message_Block* mb;
			if ( pool_.dequeue_head(mb) != -1 )
				return mb;
		}

		return 0;
	}

	size_t my_size()
	{
		return queue_.size();
	};

protected:
	ACE_Message_Queue<ACE_MT_SYNCH> pool_;
	MyQueue queue_;
};



} // namepsace mozart

#endif // _LOG_MONITOR_H_