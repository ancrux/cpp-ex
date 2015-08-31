#ifndef _ASIO_SBRS_LOADER_H_
#define _ASIO_SBRS_LOADER_H_

#include "SBRS.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

class SBRS_Loader : public ACE_Task_Base
{
public:
	SBRS_Loader(SBRS_MAPS& maps);
	~SBRS_Loader();

public:
	virtual int svc();

public:
	void start(int n_thread = 1);
	void stop();


protected:
	ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;
	ACE_Thread_Mutex lock_;

	SBRS_MAPS& maps_;
	int index_;
};

#endif // _ASIO_SBRS_LOADER_H_
