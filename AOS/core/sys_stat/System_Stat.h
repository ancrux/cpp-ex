#ifndef _SYSTEM_STAT_H_
#define _SYSTEM_STAT_H_

#include "ace/OS.h"
#include "ace/Atomic_Op.h"

#include "aos/String.h"

class System_Stat
{
public:
	System_Stat();
	~System_Stat();

public:
	int get_cpu_info(std::string& cpu); // read /proc/cpuinfo on linux
	int get_cpu_stats(float& user, float& kernel, float& iowait, float& idle);
	int get_mem_stats(long long& total, long long& used, long long& free);
	int get_swap_stats(long long& total, long long& used, long long& free);
	int get_fs_stats(std::string& fs);
	int get_ps_stats(std::string& ps);
	int get_os_stats(std::string& os);
	int get_net_if_stats(std::string& net);

protected:
	int get_ps_stats_win32(std::string& ps);

protected:
	ACE_Thread_Mutex cpu_lock_;
	ACE_Thread_Mutex mem_lock_;
	ACE_Thread_Mutex swap_lock_;
	ACE_Thread_Mutex fs_lock_;
	ACE_Thread_Mutex ps_lock_;
	ACE_Thread_Mutex os_lock_;
	ACE_Thread_Mutex net_lock_;
};

#endif // _SYSTEM_STAT_H_
