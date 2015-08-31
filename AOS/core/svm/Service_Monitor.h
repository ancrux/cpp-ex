#ifndef _SERVICE_MONITOR_H_
#define _SERVICE_MONITOR_H_

#include "ace/OS.h"
#include "ace/Task.h"
#include "ace/Atomic_Op.h"

#include "aos/String.h"

#include <iostream>
using namespace std;

class Service_Monitor : public ACE_Task_Base
{
public:
	typedef std::map< std::string, size_t > MONITORS;

public:
	Service_Monitor();
	Service_Monitor(int argc, ACE_TCHAR* argv[]);
	~Service_Monitor();
	
public:
	void start();
	void stop();

public:
	virtual int svc();

public:
	int import_svc_ini(const char* ini_file); // import svc-style ini
	int load(const char* ini_file); // load svm-style ini
	int save(const char* ini_file = 0); // save svm-style ini

public: //? should be protected to avoid synchronization problem
	MONITORS& monitors() { return monitors_; };
	MONITORS::iterator begin() { return monitors_.begin(); };
	MONITORS::iterator end() { return monitors_.end(); };
	MONITORS::const_iterator find(const char* service) const { return monitors_.find(service); };
	
public:
	int get_monitor(const char* service) const;
	void set_monitor(const char* service, int n_sec);

public:
	ACE_Thread_Mutex& lock() const { return this->mutex_; };

protected:
	MONITORS monitors_;
	std::string ini_file_; // current loaded ini file
		
	ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;
	mutable ACE_Thread_Mutex mutex_;

	int argc_;
	ACE_TCHAR** argv_;
};

#endif // _SERVICE_MONITOR_H_
