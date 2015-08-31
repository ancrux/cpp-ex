#ifndef _MAIL_PROCESSOR_H_
#define _MAIL_PROCESSOR_H_

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include "MBox_Configuration.h"

class Mail_Processor : public ACE_Task< ACE_MT_SYNCH >
{

public:
	void start(int n_thread = 0);
	void stop();

public:
	virtual int svc();

public:
	long busy() const { return n_busy_thread_.value(); }

public: // configuration
	MBox_Configuration cfg;

protected:
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_busy_thread_; // # of BUSY thread
};

template< typename Counter >
class Scope_Counter
{
public:
	Scope_Counter(Counter& counter)
	:
	counter_(counter)
	{
		++counter_;
	};
	~Scope_Counter()
	{
		--counter_;
	};

protected:
	Counter& counter_;
};

// like boost::scoped_ptr
template< typename Pointer >
class Scope_Free
{
public:
	Scope_Free(Pointer ptr)
	:
	ptr_(ptr)
	{
	};
	~Scope_Free()
	{
		//ACE_OS::printf("free ptr: %p\n", ptr_); //@
		delete ptr_;
	};
protected:
	Pointer ptr_;
};

// like boost::scoped_array
template< typename Pointer >
class Scope_Free_Array
{
public:
	Scope_Free_Array(Pointer& ptr)
	:
	ptr_(ptr)
	{
	};
	~Scope_Free_Array()
	{
		//ACE_OS::printf("free ptr: %p\n", ptr_); //@
		delete [] ptr_;
	};
protected:
	Pointer ptr_;
};

#endif // _MAIL_PROCESSOR_H_


