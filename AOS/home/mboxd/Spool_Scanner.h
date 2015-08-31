#ifndef _SPOOL_SCANNER_H_
#define _SPOOL_SCANNER_H_

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include "Mail_Processor.h"

class Spool_Scanner : public ACE_Task_Base
{
public:
	Spool_Scanner(Mail_Processor& mp);
	~Spool_Scanner();

public:
	void start();
	void stop();

public:
	virtual int svc();

protected:
	ACE_Atomic_Op<ACE_Thread_Mutex, long> go_;
	Mail_Processor& mp_;
};

#endif // _SPOOL_SCANNER_H_
