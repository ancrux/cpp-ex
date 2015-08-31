#ifndef _CELLO_DIRECTORY_UPDATER_H_
#define _CELLO_DIRECTORY_UPDATER_H_

#include "ace/OS.h"
#include "ace/Task.h"
#include "ace/Atomic_Op.h"

#include "System_Stat.h"

class Cello_Directory_Updater : public ACE_Task_Base //public ACE_Task< ACE_MT_SYNCH >
{
public:
	Cello_Directory_Updater(System_Stat& sys);
	~Cello_Directory_Updater();

public:
	void start();
	void stop();

public:
	virtual int svc();
	
protected:
	System_Stat& sys_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;
	//ACE_Thread_Mutex lock_;
};

#endif // _CELLO_DIRECTORY_UPDATER_H_
