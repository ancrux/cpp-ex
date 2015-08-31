#ifndef _MP_WORKER_H_
#define _MP_WORKER_H_

#include "ace/OS.h"
#include "ace/Task_Ex_T.h"

#include "MP_Message.h"

#include <iostream>
using namespace std;

class MP_Worker : public ACE_Task_Ex<ACE_MT_SYNCH, MP_Message>
{
public:
	void start(int n_threads = 1, int force_active = 0) { this->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, n_threads, force_active); };
	void stop() { this->msg_queue()->deactivate(); };

public:
	virtual int svc();
};

#endif // _MP_WORKER_H_