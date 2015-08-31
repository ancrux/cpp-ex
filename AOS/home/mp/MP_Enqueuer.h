#ifndef _MP_ENQUEUER_H_
#define _MP_ENQUEUER_H_

#include "ace/OS.h"
#include "ace/Task.h"
//#include "ace/Atomic_Op.h"

#include "aos/String.h"

#include "MP_Worker.h"

#include <iostream>
using namespace std;

class MP_Enqueuer : public ACE_Task_Base
{
public:
	MP_Enqueuer(MP_Worker& worker);
	~MP_Enqueuer();

public:
	void start() { stop_ = 0; this->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, 1); };
	void stop() { stop_ = 1; };
	
public:
	virtual int svc();

protected:
	MP_Worker& worker_;
	int stop_;
	//ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;
};

#endif // _MP_ENQUEUER_H_