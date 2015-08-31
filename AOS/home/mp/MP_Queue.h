#ifndef _MP_QUEUE_H_
#define _MP_QUEUE_H_

#include "ace/OS.h"

#include "aos/String.h"

class MP_Queue
{
public:
	MP_Queue();
	~MP_Queue();

public:
	void dump()
	{
		::printf("path:%s, next:%s, interval:%d\n", path.c_str(), next.c_str(), interval);
	};

public:
	std::string path; // queue folder, can be used as queue id
	std::string next; // next queue to move
	std::string mode; // queue mode: all, evp, eml, mem (?int)
	int interval; // queue process interval (in seconds), 0 for always
	ACE_Time_Value last_visited; // referred by queue process interval
};

#endif // _MP_QUEUE_H_