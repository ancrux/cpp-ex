#ifndef _IPC_H_
#define _IPC_H_

#include "aos/Config.h"

#include "ace/OS.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Process_Mutex.h"

// For Unix & Windows
#include "ace/SPIPE_Addr.h"
#include "ace/SPIPE_Connector.h"
#include "ace/SPIPE_Acceptor.h"

// For Unix & Linux
// ACE_HAS_STREAM_PIPES is not defined on Linux
#include "ace/UNIX_Addr.h"
#include "ace/LSOCK_Connector.h"
#include "ace/LSOCK_Acceptor.h"

#include <string>
#include <vector>
//#include <map>

namespace aos {
namespace ipc {

#ifdef ACE_WIN32 // (ACE_SPIPE) _WIN32 || Unix(Solaris)
typedef ACE_SPIPE_Addr Local_IPC_Addr;
typedef ACE_SPIPE_Stream Local_IPC_Stream;
typedef ACE_SPIPE_Connector Local_IPC_Connector;
typedef ACE_SPIPE_Acceptor Local_IPC_Acceptor;
#else // (ACE_LSOCK) linux || Unix(Solaris)
typedef ACE_UNIX_Addr Local_IPC_Addr;
typedef ACE_LSOCK_Stream Local_IPC_Stream;
typedef ACE_LSOCK_Connector Local_IPC_Connector;
typedef ACE_LSOCK_Acceptor Local_IPC_Acceptor;
#endif

AOS_API std::string real_local_addr(const char* ipc_addr, const char* ipc_path = 0);

typedef Local_IPC_Addr Local_Addr;

class AOS_API Local_Stream : public Local_IPC_Stream
{
public:
	static const size_t BUF_SIZE = 4096;

public:
	ssize_t send_cstr(const char* buf, size_t len);
	ssize_t recv_cstr(char delimiter = '\0');
	// to send binary data
	// first, use send_cstr("8192", 5)/recv_cstr() pair functions to get data size
	// then use send/recv(), or send_n()/recv_n() to get data
	//+ can add members send_data()/recv_data() for the above logic
	//+ ssize_t send_data(const void* buf, size_t len);
	//+ ssize_t recv_data(void* buf, size_t len, char delimiter = '\0');
	std::string& buf() { return buf_; };

	// protocol notes:
	// '+' OK
	// '-' ERROR
	// '$' SIZE
	// '&' MORE

protected:
	std::string buf_;
};

class AOS_API Local_Connector
{
public:
	int connect(Local_Stream& stream, const char* ipc_addr, const char* ipc_path = 0, ACE_Time_Value* timeout = 0);

protected:
	Local_IPC_Connector connector_;
};

class AOS_API Local_Acceptor
{
public:
	Local_Acceptor();
	~Local_Acceptor();

public:
	int open(const char* ipc_addr, const char* ipc_path = 0);
	int close();
	int accept(Local_Stream& stream, Local_Addr* remote_addr = 0, ACE_Time_Value* timeout = 0);

protected:
	Local_IPC_Acceptor acceptor_;
	//ACE_Process_Mutex* mutex_;
};

static inline int acquire(ACE_Process_Mutex& mutex, ACE_Time_Value &tv, double tick = 0.01)
{
#ifdef ACE_WIN32
	return mutex.acquire(tv);
#else
	int rc = -1;
	ACE_Time_Value sleep_tv; sleep_tv.set(tick);
	while( ACE_OS::gettimeofday() < tv )
	{
		if ( (rc = mutex.tryacquire()) == 0 )
			break;
		ACE_OS::sleep(sleep_tv);
	}
	return rc;
#endif
};

class AOS_API Service_Process
{
public:
	enum
	{
		NO_ACTION = 0x00,
		CREATE_PID = 0x01,
		EXIT_IF_PID_EXISTS = 0x02
	};

public:
	Service_Process(
		int argc,
		ACE_TCHAR* argv[],
		const char* version = 0,
		const char* build = 0,
		int flags = CREATE_PID);
	virtual ~Service_Process();

public:
	pid_t pid() const { return pid_; };
	void pid(pid_t pid) { pid_ = pid; };
	const char* pid_file() const { return file_.c_str(); };
	const char* name() const { return name_.c_str(); };
	ACE_Time_Value since() const { return since_; };
	ACE_Time_Value uptime() const { return ACE_OS::gettimeofday() - since_; };
	const char* version() const { return version_.c_str(); };
	ACE_Time_Value build_timestamp() const { return build_; };
	//+ ACE_UINT32 image_hash32() const; // compute process image hash32 value, static like version, build_timestamp
	//+ ACE_UINT64 image_hash64() const; // compute process image hash64 value, static like version, build_timestamp
	//+ const char* image_hash(const char* hash_name = "FNV32") const; // compute process image hash using specified hash algorithm, e.g. fnv32, fnv64, md5, sha1, etc
	int is_proxy() const { return 0; };

public:
	ssize_t handle_predefined_command(aos::ipc::Local_Stream& stream);
	ssize_t handle_unknown_command(aos::ipc::Local_Stream& stream);

protected:
	pid_t pid_; // service pid
	ACE_Time_Value since_; // start-up time
	std::string name_; // service name
	std::string file_; // pid file full path
	std::string version_;
	ACE_Time_Value build_;
	// Note:
	// on POSIX, ACE_Process_Mutex use ACE_SV_Semaphore_Complex
	// need to change SEMMNI in /proc/sys/kernel/sem
	// e.g. CentOS 4.4 "250 32000 32 128" => "250 32000 32 1280"
	// to change:
	// 1) echo "250 32000 32 1280" > sem
	// 2) sysctl -w kernel.sem="250 32000 32 1280"
	ACE_Process_Mutex* mutex_; // service mutex
};

class AOS_API Service_Process_Proxy : public Service_Process
{
public:
	Service_Process_Proxy(
		int argc,
		ACE_TCHAR* argv[],
		const char* version = 0,
		const char* build = 0,
		int flags = CREATE_PID)
	:
	Service_Process(argc, argv, version, build, flags)
	{
	};
	virtual ~Service_Process_Proxy() {};

public:
	int is_proxy() const { return 1; };
	void proxy_pid(pid_t pid) { pid_ = pid; };
};

class AOS_API Service_Control
{
public:
	typedef std::vector< std::pair< std::string, std::string > > SERVICES;
	//typedef std::map< std::string, std::string > SERVICES;

public:
	Service_Control(int argc, ACE_TCHAR* argv[]);
	~Service_Control();

public:
	std::string default_ini_file() const;
	int load(const char* ini_file, int clear = 1);
	int service_exec(const char* service);

public:
	int exec(const char* service, const char* command, int echo = 1, std::string* str = 0);
	pid_t pid(const char* service);
	int plock(const char* service);
	int stop(const char* service) { return this->exec(service, "stop"); };
	int wait_for_stop(pid_t pid, const char* service, const char* time_to_wait = 0);
	int start(const char* service, const char* path);
	pid_t wait_for_start(const char* service, const char* time_to_wait = 0);

public:
	const SERVICES& services() const { return services_; };
	SERVICES::const_iterator begin() const { return services_.begin(); };
	SERVICES::const_iterator end() const { return services_.end(); };
	SERVICES::const_iterator find(const char* service) const
	{
		SERVICES::const_iterator iter;
		for(iter = services_.begin(); iter != services_.end(); ++iter)
		{
			if ( ACE_OS::strcmp(service, iter->first.c_str()) == 0 )
				break;
		}
		return iter; //return services_.find(service);
	};
	
public:
	void dump_services() const
	{
		for(SERVICES::const_iterator iter = services_.begin();
			iter != services_.end();
			++iter)
		{
			ACE_OS::printf("'%s'=%s\n", iter->first.c_str(), iter->second.c_str());
		}
	}

protected:
	int argc_;
	ACE_TCHAR** argv_;
	SERVICES services_;
};

} // namespace ipc
} // namespace aos

#endif // _IPC_H_

