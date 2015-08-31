#ifndef _LOG_WORKER_H_
#define _LOG_WORKER_H_

#include "ace/OS.h"
#include "ace/Task_Ex_T.h"
//#include "ace/Task.h"

#include "Log_Store.h"
#include "Log_Monitor.h"

#include <iostream>
using namespace std;

namespace mozart {

class Log_Worker : public ACE_Task_Ex<ACE_MT_SYNCH, std::string> // : public ACE_Task<ACE_NULL_SYNCH>
{
public:
	Log_Worker();
	virtual ~Log_Worker();

public:
	void log_store(Log_Store* store)
	{
		store_ = store;
	};
	void log_monitor(Log_Monitor* monitor)
	{
		monitor_ = monitor;
	};

public:
	virtual int svc();

protected:
	Log_Store* store_;
	Log_Monitor* monitor_;
};

} // namepsace mozart

#endif // _LOG_WORKER_H_