#include "System_Stat.h"

#include "statgrab.h"

#ifdef WIN32

#undef UNICODE
#undef _UNICODE

//#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
//#include <stdio.h>

#include <vector>
#include <map>

#endif

System_Stat::System_Stat()
{
	// Initialize statgrab
	sg_init();
	
	// Drop setuid/setgid privileges
	if ( sg_drop_privileges() != 0 )
	{
		perror("Error: Failed to drop privileges");
		return;
	}
	
	// Throw away the first reading as thats averaged over the machines uptime
	sg_cpu_percents* cpu_percent;
	sg_snapshot();

	cpu_percent = sg_get_cpu_percents();
	sg_snapshot();
}

System_Stat::~System_Stat()
{
	// Finalize statgrab
	sg_shutdown();
}

int
System_Stat::get_cpu_info(std::string& cpu)
{
	int n_cpu = 0;

	cpu = "processor_id\tmodel\tphysical_id\tcore_id\tcore_count\n";
	std::string processor, model, physical, core, core_count;

#ifdef linux
	FILE* fp;
	fp = ACE_OS::fopen("/proc/cpuinfo", "r");
	if ( fp == NULL )
		return n_cpu;

	std::string line;
	static const int BUF_SIZE = 1024;
	char buf[BUF_SIZE+1];
	while( ACE_OS::fgets(buf, BUF_SIZE, fp) != NULL )
	{
		line.assign(buf);
		size_t colon;
		if ( (colon = line.find(':')) == std::string::npos )
		{
			++n_cpu;
			cpu += processor; cpu += "\t";
			cpu += model; cpu += "\t";
			cpu += ((physical.empty())?"0":physical); cpu += "\t";
			cpu += ((core.empty())?"0":core); cpu += "\t";
			cpu += ((core_count.empty())?"1":core_count); cpu += "\n";

			processor.resize(0);
			model.resize(0);
			physical.resize(0);
			core.resize(0);
			core_count.resize(0);

			continue;
		}

		std::string key = line.substr(0, colon); aos::trim(key);
		std::string val = line.substr(colon+1, line.size()-colon-1); aos::trim(val);

		//ACE_OS::printf("%s=%s\n", key.c_str(), val.c_str()); //@

		if ( ACE_OS::strncasecmp(key.c_str(), "processor", 9) == 0 )
		{
			processor = val.c_str();
		}
		else if ( ACE_OS::strncasecmp(key.c_str(), "model name", 10) == 0 )
		{
			model = val.c_str();
		}
		else if ( ACE_OS::strncasecmp(key.c_str(), "physical id", 11) == 0 )
		{
			physical = val.c_str();
		}
		else if ( ACE_OS::strncasecmp(key.c_str(), "core id", 7) == 0 )
		{
			core = val.c_str();
		}
		else if ( ACE_OS::strncasecmp(key.c_str(), "cpu cores", 10) == 0 )
		{
			core_count = val.c_str();
		}
	}

	ACE_OS::fclose(fp);
#endif

	return n_cpu;
}

int
System_Stat::get_cpu_stats(float& user, float& kernel, float& iowait, float& idle)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->cpu_lock_, -1);

	sg_cpu_percents* cpu_percent;
	if ( (cpu_percent = sg_get_cpu_percents()) != NULL )
	{
		sg_snapshot();
		user = cpu_percent->user;
		kernel = cpu_percent->kernel;
		iowait = cpu_percent->iowait;
		idle = cpu_percent->idle;
	}

	return (cpu_percent)?0:-1;
}

int
System_Stat::get_mem_stats(long long& total, long long& used, long long& free)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->mem_lock_, -1);

	sg_mem_stats* mem_stats;
	if ( (mem_stats = sg_get_mem_stats()) != NULL )
	{
		total = mem_stats->total;
		used = mem_stats->used;
		free = mem_stats->free;

		return 0;
	}

	return -1;
}

int
System_Stat::get_swap_stats(long long& total, long long& used, long long& free)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->swap_lock_, -1);

	sg_swap_stats* swap_stats;
	if ( (swap_stats = sg_get_swap_stats()) != NULL )
	{
		total = swap_stats->total;
		used = swap_stats->used;
		free = swap_stats->free;

		return 0;
	}

	return -1;
}

int
System_Stat::get_fs_stats(std::string& fs)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->fs_lock_, -1);

	sg_fs_stats* fs_stats;
	int n_fs_stats;
	
	fs = "device\ttype\tmount\ttotal\tused\tfree\n";
	fs_stats = sg_get_fs_stats(&n_fs_stats);
	if ( fs_stats )
	{
		for(int i = 0; i < n_fs_stats; ++i, ++fs_stats)
		{
			char buf[PATH_MAX+1];
			fs += (fs_stats->device_name)?fs_stats->device_name:""; fs += "\t";
			fs += (fs_stats->fs_type)?fs_stats->fs_type:""; fs += "\t"; 
			fs += (fs_stats->mnt_point)?fs_stats->mnt_point:""; fs += "\t"; 
			ACE_OS::snprintf(buf, PATH_MAX, "%lld\t", fs_stats->size); fs += buf;
			ACE_OS::snprintf(buf, PATH_MAX, "%lld\t", fs_stats->used); fs += buf;
			ACE_OS::snprintf(buf, PATH_MAX, "%lld\n", fs_stats->avail); fs += buf;
		}

		return n_fs_stats;
	}

	return -1;
}

int
System_Stat::get_ps_stats_win32(std::string& ps)
{
	typedef std::map<std::string, size_t> PROC_MAP; // process map

	int rc = -1;

#ifdef WIN32

	PDH_STATUS  status;
	LPSTR object = "Process";

	//LPSTR msz_instance = NULL;
	std::string str_instance;
	DWORD len_instance = 0;
	DWORD n_instance = 0;
	
	//LPSTR msz_counter = NULL;
	std::string str_counter;
	DWORD len_counter = 0;
	DWORD n_counter = 0;

	status = ::PdhEnumObjectItems(NULL, NULL, object,
		NULL, &len_counter,
		NULL, &len_instance,
		PERF_DETAIL_WIZARD, 0);
	if ( status != PDH_MORE_DATA && status != ERROR_SUCCESS )
		return -1;

	str_instance.resize(len_instance * sizeof(CHAR) + 1);
	str_counter.resize(len_counter * sizeof(CHAR) + 1);

	status = ::PdhEnumObjectItems(NULL, NULL, object,
		(LPSTR) str_counter.c_str(), &len_counter,
		(LPSTR) str_instance.c_str(), &len_instance,
		PERF_DETAIL_WIZARD, 0);
	if ( status != ERROR_SUCCESS )
		return -1;

	// set counter manually
	str_counter.resize(0); n_counter = 0;
	str_counter.append("ID Process"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Creating Process ID"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("% Processor Time"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Working Set"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Thread Count"); str_counter.append(1, '\0'); ++n_counter;
	str_counter.append("Priority Base"); str_counter.append(1, '\0'); ++n_counter;

	// set instance map
	PROC_MAP map_instance;
	for(LPSTR ptr = (LPSTR) str_instance.c_str(); *ptr; ptr += strlen(ptr)+1)
	{
		//printf("%s\n", ptr);
		++n_instance;
		map_instance[ptr]++;
	}

	/*
	for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
	{
		printf("%s=%d\n", iter->first.c_str(), iter->second);
	}
	//*/

	HQUERY hQuery = NULL;

    // open query
	if ( (status = ::PdhOpenQuery(NULL, 0, &hQuery)) != ERROR_SUCCESS )
		return -1;

	// add counters
	std::vector< HCOUNTER > counter_handles(n_instance * n_counter);
	size_t n_handle = 0;
	for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
	{
		size_t dup_count = iter->second;
		for(size_t n = 0; n < dup_count; ++n)
		{
			const char* ptr = str_counter.c_str();
			for(DWORD c = 0; c < n_counter; ++c, ptr += strlen(ptr)+1)
			{
				std::string counter_path = "\\";
				counter_path += (char*) object;
				counter_path += "(";
				counter_path += iter->first;
				if ( n )
				{
					char buf[32];
					counter_path += "#";
					counter_path += ACE_OS::itoa((int) n, buf, 10);
				}
				counter_path += ")\\";
				counter_path += ptr;
				//printf("%s\n", counter_path.c_str()); //@

				status = ::PdhAddCounter(hQuery, counter_path.c_str(), 0, &counter_handles[n_handle++]);
				//if ( status != ERROR_SUCCESS )
				//{
				//	printf("add counter_handles[%d] error!\n", n_handle-1);
				//}
			}
		}
	}

	// get counter values
	if ( (status = ::PdhCollectQueryData(hQuery)) != ERROR_SUCCESS )
		return -1;

	::Sleep(50);

	if ( (status = ::PdhCollectQueryData(hQuery)) == ERROR_SUCCESS )
	{
		ps = "pid\tcpu\tmem\tthr\tstate\tname\ttitle\n";
		rc = n_instance;

		PDH_FMT_COUNTERVALUE counter_value;

		size_t n_handle = 0;
		for(PROC_MAP::iterator iter = map_instance.begin(); iter != map_instance.end(); ++iter)
		{
			size_t dup_count = iter->second;
			for(size_t n = 0; n < dup_count; ++n)
			{
				std::string counter_path = iter->first;
				if ( n )
				{
					char buf[32];
					counter_path += "#";
					counter_path += ACE_OS::itoa((int) n, buf, 10);
				}

				char buf[PATH_MAX+1];

				// pid
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%d\t", (int) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;

				// ppid
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);

				// cpu
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%f\t", counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;
				double cpu = counter_value.doubleValue;
				
				// mem
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%llu\t", (long long) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;
				
				// thr
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);
				(status == ERROR_SUCCESS)?
					ACE_OS::snprintf(buf, PATH_MAX, "%d\t", (int) counter_value.doubleValue):
					ACE_OS::snprintf(buf, PATH_MAX, " \t");
				ps += buf;

				// priority
				status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++], PDH_FMT_DOUBLE, NULL, &counter_value);

				// state
				(cpu > 0.000001)?
					ACE_OS::snprintf(buf, PATH_MAX, "RUNNING\t"):
					ACE_OS::snprintf(buf, PATH_MAX, "SLEEPING\t");
				ps += buf;

				// name
				ps += iter->first; ps += "\t";

				// title
				ps += counter_path; ps += "\t";

				ps += "\n";

				/*
				//const char* ptr = str_counter.c_str();
				//for(DWORD c = 0; c < n_counter; ++c, ptr += strlen(ptr)+1)
				//{
				//	std::string counter_path = iter->first;
				//	if ( n )
				//	{
				//		char buf[32];
				//		counter_path += "#";
				//		counter_path += ACE_OS::itoa(n, buf, 10);
				//	}
				//	counter_path += "\\";
				//	counter_path += ptr;
				//	//printf("%s\n", counter_path.c_str()); //@

				//	if ( (status = ::PdhGetFormattedCounterValue(counter_handles[n_handle++],
				//		PDH_FMT_DOUBLE,
				//		NULL, &counter_value)) != ERROR_SUCCESS)
				//	{
				//		//fprintf(stderr, "PGFCV failed %08x\n", s);
				//		continue;
				//	}
				//	printf("%s=[%3.3f]\n", counter_path.c_str(), counter_value.doubleValue);
				//}
				//*/
			}
		}
	}

	// remove counters
	for(size_t n = 0, c = counter_handles.size(); n < c; ++n)
		::PdhRemoveCounter(counter_handles[n]);

	// close query
	::PdhCloseQuery(hQuery);

#endif

	return rc;
}

int
System_Stat::get_ps_stats(std::string& ps)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->ps_lock_, -1);

	// on Win32, can use "tasklist" command
#ifdef WIN32

	return get_ps_stats_win32(ps);

#else

	sg_process_stats* ps_stats;
	int n_ps_stats;
	char* state = "";
	
	ps = "pid\tcpu\tmem\tthr\tstate\tname\ttitle\n";
	ps_stats = sg_get_process_stats(&n_ps_stats);
	if ( ps_stats )
	{
		for(int i = 0; i < n_ps_stats; ++i, ++ps_stats)
		{
			char buf[PATH_MAX+1];
			ACE_OS::snprintf(buf, PATH_MAX, "%d\t", ps_stats->pid); ps += buf;
			ACE_OS::snprintf(buf, PATH_MAX, "%f\t", ps_stats->cpu_percent); ps += buf;
			ACE_OS::snprintf(buf, PATH_MAX, "%llu\t", ps_stats->proc_resident); ps += buf;
			ACE_OS::snprintf(buf, PATH_MAX, "%d\t", ps_stats->num_threads); ps += buf;
			switch(ps_stats->state)
			{
			case SG_PROCESS_STATE_RUNNING:
				state = "RUNNING";
				break;
			case SG_PROCESS_STATE_SLEEPING:
				state = "SLEEPING";
				break;
			case SG_PROCESS_STATE_STOPPED:
				state = "STOPPED";
				break;
			case SG_PROCESS_STATE_ZOMBIE:
				state = "ZOMBIE";
				break;
			case SG_PROCESS_STATE_UNKNOWN:
			default:
				state = "UNKNOWN";
				break;
			}
			ps += state; ps += "\t";
			//ACE_OS::snprintf(buf, PATH_MAX, "%d\t", ps_stats->state); ps += buf;
			//ACE_OS::snprintf(buf, PATH_MAX, "%d\t", (int) ps_stats->time_spent); time += buf;
			ps += (ps_stats->process_name)?ps_stats->process_name:""; ps += "\t";
			ps += (ps_stats->proctitle)?ps_stats->proctitle:""; ps += "\n";
		}

		return n_ps_stats;
	}

	return -1;

#endif
}

int
System_Stat::get_os_stats(std::string& os)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->os_lock_, -1);

	sg_host_info* os_stats;
	int n_os_stats = 0;
	
	os = "os_name\tos_release\tos_version\tplatform\thost_name\tuptime\n";
	os_stats = sg_get_host_info();
	if ( os_stats )
	{
		n_os_stats = 1;
		for(int i = 0; i < n_os_stats; ++i)
		{
			char buf[PATH_MAX+1];
			os += (os_stats->os_name)?os_stats->os_name:""; os += "\t";
			os += (os_stats->os_release)?os_stats->os_release:""; os += "\t"; 
			os += (os_stats->os_version)?os_stats->os_version:""; os += "\t";
			os += (os_stats->platform)?os_stats->platform:""; os += "\t";
			os += (os_stats->hostname)?os_stats->hostname:""; os += "\t";
			ACE_OS::snprintf(buf, PATH_MAX, "%lld\n", (long long) os_stats->uptime); os += buf;
		}
	}

	return n_os_stats;
}

int
System_Stat::get_net_if_stats(std::string& net)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->net_lock_, -1);

	sg_network_iface_stats* net_if_stats;
	int n_net_if = 0;
	char* duplex = "";
	
	net = "if_name\tif_speed\tif_duplex\n";
	net_if_stats = sg_get_network_iface_stats(&n_net_if);
	if ( net_if_stats )
	{
		for(int i = 0; i < n_net_if; ++i, ++net_if_stats)
		{
			char buf[PATH_MAX+1];
			net += (net_if_stats->interface_name)?net_if_stats->interface_name:""; net += "\t";
			ACE_OS::snprintf(buf, PATH_MAX, "%d\t", net_if_stats->speed); net += buf;
			switch(net_if_stats->duplex)
			{
			case SG_IFACE_DUPLEX_FULL:
				duplex = "FULL";
				break;
			case SG_IFACE_DUPLEX_HALF:
				duplex = "HALF";
				break;
			default:
				duplex = "UNKNOWN";
				break;
			}
			net += duplex; net += "\n";
		}
	}

	return n_net_if;
}

