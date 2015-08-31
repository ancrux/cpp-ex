#ifndef _MP_CONFIGURATION_H_
#define _MP_CONFIGURATION_H_

#include "MP_Queue.h"

#include <map>

class MP_Configuration
{
public:
	typedef std::map< std::string, MP_Queue* > QUEUES;

public:
	MP_Configuration();
	~MP_Configuration();

public:
	void load(const char* ini_file);
	QUEUES& queues() { return queues_; };
	void dump()
	{
		for(QUEUES::iterator iter = queues_.begin(); iter != queues_.end(); ++iter)
			(iter->second)->dump();
	};
	
protected:
	void clear_queues();

protected:
	QUEUES queues_;
};

#endif // _MP_CONFIGURATION_H_