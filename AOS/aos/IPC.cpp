#include "IPC.h"
#include "String.h"

#include "ace/Get_Opt.h"
#include "ace/Process_Manager.h"

namespace aos {
namespace ipc {

std::string
real_local_addr(const char* ipc_addr, const char* ipc_path)
{
	std::string addr(ipc_addr);
	
	#ifdef ACE_WIN32 // (ACE_SPIPE) _WIN32 || Unix
		do {} while(0); // do nothing
	#else // (ACE_LSOCK) linux || Unix
		if ( ipc_path )
		{
			std::string path(ipc_path);
			path += ACE_DIRECTORY_SEPARATOR_CHAR;
			addr = path + addr;
		}
		else
			addr = "/tmp/" + addr;
	#endif

	return addr;
}

/// class

ssize_t
Local_Stream::send_cstr(const char* buf, size_t len)
{
	const char* left = buf;
	size_t n_left = len;

	while( n_left > 0 )
	{
		size_t n_size = (n_left>BUF_SIZE)?BUF_SIZE:n_left;
		ssize_t n_send = this->send(left, n_size);
		if ( n_send > 0 )
		{
			left += n_send;
			n_left -= n_send;
		}
		else
		{
			break;
		}
	}

	return (n_left==0)?(ssize_t) len:-1;
}

ssize_t
Local_Stream::recv_cstr(char delimiter)
{
	ssize_t n_total = 0;
	char buf[BUF_SIZE];

	buf_.resize(0);
	while(true)
	{
		ssize_t n_recv = this->recv(buf, BUF_SIZE);
		if ( n_recv > 0 )
		{
			n_total += n_recv;
			buf_.append(buf, n_recv);
			if ( buf[n_recv-1] == delimiter )
				break;
		}
		else
		{
			n_total = -1;
			break;
		}
	}

	return n_total;
}

int
Local_Connector::connect(Local_Stream& stream, const char* ipc_addr, const char* ipc_path, ACE_Time_Value* timeout)
{
	Local_Addr addr(real_local_addr(ipc_addr, ipc_path).c_str());
	return connector_.connect(stream, addr, timeout);
}

/// class

Local_Acceptor::Local_Acceptor()
//:
//mutex_(0)
{
}

Local_Acceptor::~Local_Acceptor()
{
	this->close();
}

int
Local_Acceptor::open(const char* ipc_addr, const char* ipc_path)
{
	//if ( mutex_ ) return -1; //? this->close();

	//mutex_ = new (std::nothrow) ACE_Process_Mutex(real_local_addr(ipc_addr, ipc_path).c_str());
	//if ( mutex_->tryacquire() != 0 )
	//{
	//	this->close();
	//	return -1;
	//}

	// only invoked on the IPC server-side, can be implemented in Local_Acceptor::open()
	#ifdef ACE_WIN32 // (ACE_SPIPE) _WIN32 || Unix
		do {} while(0); // do nothing
	#else // (ACE_LSOCK) linux || Unix
		ACE_OS::unlink(real_local_addr(ipc_addr, ipc_path).c_str()); // for ACE_LSOCK
	#endif

	Local_Addr addr(real_local_addr(ipc_addr, ipc_path).c_str());
	return acceptor_.open(addr);
}

int
Local_Acceptor::close()
{
	//if ( mutex_ )
	//{
	//	delete mutex_;
	//	mutex_ = 0;
	//}

	return acceptor_.close();
}

int
Local_Acceptor::accept(Local_Stream& stream, Local_Addr* remote_addr, ACE_Time_Value* timeout)
{
	return acceptor_.accept(stream, remote_addr, timeout);
}

/// class

Service_Process::Service_Process(int argc, ACE_TCHAR* argv[], const char* version, const char* build, int flags)
:
pid_(ACE_OS::getpid()),
since_(ACE_Time_Value::zero),
build_(ACE_Time_Value::zero),
mutex_(0)
{
	if ( version ) this->version_ = version;
	if ( build ) 
	{
		bool is_valid = true;
		struct tm tm_val;
		ACE_OS::memset(&tm_val, 0, sizeof(tm));

		// on Linux, use ACE_OS::strptime(build, "%b %d %Y %H:%M:%S", &tm_val);

		char buf[64];
		ACE_OS::strncpy(buf, build, 63);

		const char* sep = " :\t";
		char* token = ACE_OS::strtok(buf, sep);
		if ( ACE_OS::strcasecmp("Jan", token) == 0 )
			tm_val.tm_mon = 0;
		else if ( ACE_OS::strcasecmp("Feb", token) == 0 )
			tm_val.tm_mon = 1;
		else if ( ACE_OS::strcasecmp("Mar", token) == 0 )
			tm_val.tm_mon = 2;
		else if ( ACE_OS::strcasecmp("Apr", token) == 0 )
			tm_val.tm_mon = 3;
		else if ( ACE_OS::strcasecmp("May", token) == 0 )
			tm_val.tm_mon = 4;
		else if ( ACE_OS::strcasecmp("Jun", token) == 0 )
			tm_val.tm_mon = 5;
		else if ( ACE_OS::strcasecmp("Jul", token) == 0 )
			tm_val.tm_mon = 6;
		else if ( ACE_OS::strcasecmp("Aug", token) == 0 )
			tm_val.tm_mon = 7;
		else if ( ACE_OS::strcasecmp("Sep", token) == 0 )
			tm_val.tm_mon = 8;
		else if ( ACE_OS::strcasecmp("Oct", token) == 0 )
			tm_val.tm_mon = 9;
		else if ( ACE_OS::strcasecmp("Nov", token) == 0 )
			tm_val.tm_mon = 10;
		else if ( ACE_OS::strcasecmp("Dec", token) == 0 )
			tm_val.tm_mon = 11;
		else
			is_valid = false;

		int c = 0;
		while( is_valid )
		{
			token = ACE_OS::strtok(0, sep);
			if ( token == 0 )
				break;
			int val = ::atoi(token);
			
			switch(c)
			{
			case 0:
				tm_val.tm_mday = val;
				break;
			case 1:
				tm_val.tm_year = val - 1900;
				break;
			case 2:
				tm_val.tm_hour = val;
				break;
			case 3:
				tm_val.tm_min = val;
				break;
			case 4:
				tm_val.tm_sec = val;
				break;
			default:
				is_valid = false;
			};

			++c;
		}

		if ( is_valid )
		{
			tm_val.tm_isdst = -1;
			time_t btime = ACE_OS::mktime(&tm_val);
			if ( btime != -1 )
			{
				this->build_.sec(btime);
			}
		}
	}

	// Get_Opt
	ACE_Get_Opt cmd(argc, argv);
	cmd.long_option(ACE_TEXT("as"), ACE_Get_Opt::ARG_REQUIRED);
	cmd.long_option(ACE_TEXT("version"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("build_timestamp"), ACE_Get_Opt::NO_ARG);
	//+ cmd.long_option(ACE_TEXT("image_hash"), ACE_Get_Opt::ARG_OPTIONAL);
	//cmd.long_option(ACE_TEXT("svc"), 's', ACE_Get_Opt::ARG_REQUIRED);
	std::string svc_arg;

	int ch;
	while( (ch = cmd()) != EOF )
	{
		//ACE_OS::printf("index:%d=%c\n", cmd.opt_ind(), ch); //@
		switch(ch)
		{
		case 0:
			if ( ACE_OS::strcasecmp(cmd.last_option(), "as") == 0 )
			{
				svc_arg = cmd.opt_arg();
			}
			//else if ( cmd.argc() == 2 && ACE_OS::strcasecmp(cmd.last_option(), "version") == 0 )
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "version") == 0 )
			{
				ACE_OS::printf("%s\n", (!this->version_.empty())?this->version():"");
				ACE_OS::exit(0);
			}
			//else if ( cmd.argc() == 2 && ACE_OS::strcasecmp(cmd.last_option(), "build_timestamp") == 0 )
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "build_timestamp") == 0 )
			{
				char date[64];
				ACE_OS::strcpy(date, "");
				if ( this->build_ != ACE_Time_Value::zero )
				{
					time_t build = this->build_timestamp().sec();
					struct tm tm_val;
					ACE_OS::localtime_r(&build, &tm_val);
					size_t n_buf = ACE_OS::strftime(date, 63, "%Y-%m-%d %H:%M:%S", &tm_val);
				}
				ACE_OS::printf("%s\n", date);
				ACE_OS::exit(0);
			}
			break;
		//case 's':
		//	if ( cmd.opt_arg() )
		//		svc_arg = cmd.opt_arg();
		//	break;
		}
	}
	// if svc_arg not found, use program's name without file extenstion
	if ( svc_arg.empty() )
	{
		svc_arg = ACE::basename(cmd.argv()[0]);
		size_t last_dot = svc_arg.rfind('.');
		if ( last_dot != std::string::npos )
			svc_arg.erase(last_dot);
	}

	// set service name
	name_ = svc_arg;

	// make sure only one instance
	mutex_ = new (std::nothrow) ACE_Process_Mutex(this->name());
	if ( mutex_ == 0 )
	{
		ACE_OS::printf("'%s' failed to create process lock!\n", this->name());
		ACE_OS::exit(-1);
	}
	if ( mutex_->tryacquire() != 0 )
	{
		ACE_OS::printf("'%s' process lock exists!\n", this->name());
		ACE_OS::exit(-1);
	}

	// exit if pid file exits
	if ( (flags & EXIT_IF_PID_EXISTS) == EXIT_IF_PID_EXISTS )
	{
		ACE_stat stat;
		if ( ACE_OS::lstat(file_.c_str(), &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFREG )
		{
			ACE_OS::exit(-1);
		}
	}

	// write pid file
	if ( (flags & CREATE_PID) == CREATE_PID )
	{
		char path[PATH_MAX+1];
		ACE_OS::realpath(argv[0], path);

		std::string pid_file(ACE::dirname(path));
		pid_file += ACE_DIRECTORY_SEPARATOR_CHAR;
		pid_file += name_;
		pid_file += ".pid";

		ACE_HANDLE fh = ACE_OS::open(pid_file.c_str(), O_CREAT | O_BINARY | O_WRONLY);
		if ( fh != ACE_INVALID_HANDLE )
		{
			char buf[128];
			size_t len = ACE_OS::snprintf(buf, 127, "%d", this->pid());

			ssize_t n_write = ACE_OS::write(fh, buf, len);
			if ( n_write > 0 )
				file_ = pid_file;
			ACE_OS::close(fh);
		}
	}

	// set start-up time
	this->since_ = ACE_OS::gettimeofday();

	// To get start-up time, simply check the creation time of pid file!
}

Service_Process::~Service_Process()
{
	// erase pid file
	if ( !file_.empty() )
	{
		ACE_stat stat;
		if ( ACE_OS::lstat(file_.c_str(), &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFREG )
		{
			ACE_OS::unlink(file_.c_str());
		}
	}

	if ( mutex_ )
	{
		delete mutex_;
		mutex_ = 0;
	}
}

ssize_t
Service_Process::handle_predefined_command(aos::ipc::Local_Stream& stream)
{
	ssize_t n_send = -1;
	int n = 0;

	if ( ACE_OS::strcasecmp(stream.buf().c_str(), "pid") == 0 )
	{
		char buf[256];
		n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\t%s", this->pid(), this->name(), this->pid_file());
		n_send = stream.send_cstr(buf, n+1); // include '\0'
	}
	else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "uptime") == 0 || ACE_OS::strcasecmp(stream.buf().c_str(), "since") == 0 )
	{
		time_t since = this->since().sec();
		struct tm tm_val;
		ACE_OS::localtime_r(&since, &tm_val);
		char date[64];
		size_t n_buf = ACE_OS::strftime(date, 63, "%Y-%m-%d %H:%M:%S", &tm_val);

		char buf[256];
		if ( stream.buf()[0] == 'u' || stream.buf()[0] == 'U' )
			n = ACE_OS::snprintf(buf, 255, "+%lld\t[%s]\t%s", (long long) this->uptime().sec(), this->name(), date);
		else
			n = ACE_OS::snprintf(buf, 255, "+%lld\t[%s]\t%s", (long long) since, this->name(), date);
		n_send = stream.send_cstr(buf, n+1); // include '\0'
	}
	else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "version") == 0 )
	{
		char buf[256];
		if ( !this->version_.empty() )
			n = ACE_OS::snprintf(buf, 255, "+0\t[%s]\t%s", this->name(), this->version());
		else
			n = ACE_OS::snprintf(buf, 255, "-1\t[%s]", this->name());
		n_send = stream.send_cstr(buf, n+1); // include '\0'
	}
	else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "build_timestamp") == 0 )
	{
		char buf[256];
		if ( this->build_ != ACE_Time_Value::zero )
		{
			time_t build = this->build_timestamp().sec();
			struct tm tm_val;
			ACE_OS::localtime_r(&build, &tm_val);
			char date[64];
			size_t n_buf = ACE_OS::strftime(date, 63, "%Y-%m-%d %H:%M:%S", &tm_val);

			n = ACE_OS::snprintf(buf, 255, "+%lld\t[%s]\t%s",(long long) build, this->name(), date);
		}
		else
		{
			n = ACE_OS::snprintf(buf, 255, "-1\t[%s]", this->name());
		}
		n_send = stream.send_cstr(buf, n+1); // include '\0'
	}

	return n_send;
}

ssize_t
Service_Process::handle_unknown_command(aos::ipc::Local_Stream& stream)
{
	ssize_t n_send = -1;

	char buf[256];
	int n = ACE_OS::snprintf(buf, 255, "-1\t[%s]\tunknown command!", this->name());
	n_send = stream.send_cstr(buf, n+1); // include '\0'

	return n_send;
}

Service_Control::Service_Control(int argc, ACE_TCHAR *argv[])
:
argc_(argc),
argv_(argv)
{
}

Service_Control::~Service_Control()
{
}

std::string
Service_Control::default_ini_file() const
{
	char path[PATH_MAX+1];
	ACE_OS::realpath(argv_[0], path);

	std::string ini_file(path);
	if ( ini_file.size() > 4 && ACE_OS::strcasecmp(ini_file.c_str() + ini_file.size() - 4, ".exe") == 0 )
		ini_file.resize(ini_file.size() - 4);
	ini_file += ".ini";
	//ACE_OS::printf("ini:%s\n", ini_file.c_str()); //@

	return ini_file;
}

int
Service_Control::load(const char* ini_file, int clear)
{
	int rc = 0;

	FILE* fp = ACE_OS::fopen(ini_file, "rb");
	if ( !fp )
	{
		rc = -1;
		return rc;
	}

	// change working directory, so relative paths in INI file can be translated correctly!
	char path[PATH_MAX+1];
	ACE_OS::realpath(ini_file, path);
	ACE_OS::chdir(ACE::dirname(path));

	static const int LINE = 4096;
	char buf[LINE];
	std::string line;
	std::string sec, key, val;

	bool sec_found = false;
	while( ACE_OS::fgets(buf, LINE, fp) )
	{
		line.assign(buf);

		size_t bracket_l = line.find_first_of('[');
		size_t bracket_r = line.find_last_of(']');
		if ( bracket_l != std::string::npos /*&& bracket_l <= 3*/ && bracket_r != std::string::npos && bracket_l < bracket_r )
		{
			sec = line.substr(bracket_l+1, bracket_r-bracket_l-1);
			continue;
		}

		if ( sec != "*" )
		{
			if ( sec_found ) break;
			else continue;
		}
		else
		{
			// section '*' found
			sec_found = true;
		}

		size_t eq = line.find_first_of('=');
		if ( eq == std::string::npos )
			continue;

		key = line.substr(0, eq); aos::trim(key);
		val = line.substr(eq+1); aos::trim(val);

		if ( key.size() && ACE_OS::ace_isalnum(key[0]) )
		{
			size_t colon = key.find_first_of(':');
			if ( colon != std::string::npos )
				key = key.substr(0, colon);

			if ( val.size() >= 2 && val[0] == '"' && val[0] == val[val.size()-1] )
				val = val.substr(1, val.size()-2);

			services_.push_back( // services_.insert(
				std::make_pair(
					key,
					val // was path
					)
				);
			//::printf("key:%s, val:%s\n", key.c_str(), val.c_str()); //@
		}

		key.resize(0);
		val.resize(0);
	}

	ACE_OS::fclose(fp);

	/*
	// load ini file
	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	iniIO.import_config(ini_file);

	// change working directory, so relative paths in INI file can be translated correctly!
	char path[PATH_MAX+1];
	ACE_OS::realpath(ini_file, path);
	ACE_OS::chdir(ACE::dirname(path)); 

	// clear services
	if ( clear )
		services_.clear();

	// read services
	ACE_Configuration_Section_Key sec;
	config.open_section(config.root_section(), ACE_TEXT("*"), 0, sec);

	ACE_TString name;
	ACE_Configuration::VALUETYPE val_type;
	for(int i = 0;
		config.enumerate_values(sec, i, name, val_type) == 0;
		++i)
	{
		ACE_TString value;
		config.get_string_value(sec, name.c_str(), value);

		//ACE_OS::printf("raw: %s\n", value.c_str()); //@

		//// translate to real path
		//char path[PATH_MAX+1];
		//char* exist = ACE_OS::realpath(value.c_str(), path);
		//if ( exist )
		//	value.set(path);

		//ACE_OS::printf("real: %s\n", value.c_str()); //@

		services_.push_back( // services_.insert(
			std::make_pair(
				std::string(name.c_str(), name.length()),
				std::string(value.c_str()) // was path
				)
			);
	}

	// set working directory back to where the program resides
	ACE_OS::realpath(argv_[0], path);
	ACE_OS::chdir(ACE::dirname(path));
	//*/

	return rc;
}

int
Service_Control::exec(const char* service, const char* command, int echo, std::string* str)
{
	aos::ipc::Local_Stream stream;
	aos::ipc::Local_Connector connector;
	ACE_Time_Value timeout(1);

	if ( connector.connect(stream, service, 0, &timeout) == -1 )
	{
		//ACE_OS::printf("-1 '%s' is not running! (error: %s)\n", service, ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("connect()");
		return -1;
	}

	ssize_t n_send = stream.send_cstr(command, ACE_OS::strlen(command)+1); // include '\0'
	if ( n_send < 0 ) return -1; // -2

	ssize_t n_recv = stream.recv_cstr();
	if ( n_recv < 0 ) return -1; // -3
	if ( str ) str->append(stream.buf()); //? append "\n" for consistence with echo
	if ( echo ) ACE_OS::printf("%s\n", stream.buf().c_str()); // if ( echo ) ACE_OS::write(ACE_STDOUT, stream.buf().c_str(), stream.buf().size()); // ACE_OS::write() don't support | grep
	
	stream.close();

	return 0;
}

pid_t
Service_Control::pid(const char* service)
{
	std::string response;
	this->exec(service, "pid", 0, &response);

	pid_t pid = -1;
	if ( !response.empty() && *(response.c_str()) == '+' )
		pid = ACE_OS::strtol(response.c_str()+1, 0, 10);

	return pid;
}

int
Service_Control::plock(const char* service)
{
	int rc = 0;

	ACE_Process_Mutex mutex(service);
	if ( mutex.tryacquire() == 0 )
		mutex.release();
	else
	{
		rc = 1;
		//ACE_OS::printf("'%s' process lock exists!\n", service);
	}

	return rc;
}

int
Service_Control::start(const char* service, const char* path)
{
	int rc = 0;

	std::string start_cmd(path);
	start_cmd += " --as ";
	start_cmd += service;

	ACE_Process_Options opt;
	opt.command_line(start_cmd.c_str());
	
	//ACE_OS::printf("start_cmd: %s\n", start_cmd.c_str()); //@
	//ACE_OS::printf("+%d\t[%s]\tservice is starting...\n", rc, service);

	char proc[PATH_MAX+1];
	ACE_OS::realpath(opt.process_name(), proc); // return 0 on linux if path doesn't exists
	opt.process_name(proc);
	opt.working_directory(ACE::dirname(opt.process_name()));
	//opt.set_handles(ACE_INVALID_HANDLE);

	/*
	ACE_OS::printf("proc:%s\n", opt.process_name()); //@
	ACE_OS::printf("full:%s\n", opt.command_line_buf()); //@
	//int argc = 0;
	//char* const* argv = opt.command_line_argv();
	//for(int i = 1; argv[i] != 0; ++i)
	//	ACE_OS::printf("[%d]=%s\n", i, argv[i]);
	//*/
	
	ACE_stat stat;
	if ( ACE_OS::lstat(ACE::dirname(opt.process_name()), &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFDIR )	
	{	
		//ACE_OS::printf("+%d\t[%s]\tservice is starting...\n", rc, service);
		ACE_OS::printf("+%d\t[%s]\tservice is starting...\t%s\n", rc, service, opt.process_name());
	}
	else
	{
		rc = -1;
		ACE_OS::printf("%d\t[%s]\tservice directory: '%s' doesn't exist!\n", rc, service, ACE::dirname(opt.process_name()));
		return rc;
	}

#ifdef ACE_WIN32
	ACE_Process_Manager* pm = ACE_Process_Manager::instance();
	opt.creation_flags(DETACHED_PROCESS); //Win32 use DETACHED_PROCESS, or CREATE_NEW_CONSOLE
	pid_t start_pid = pm->spawn(opt);
	//ACE_Process p;
	//pid_t pid = pm->spawn(&p, opt);
	//ACE_OS::printf("pid=%d\n", pid); //@
#else
	///*
	// Method 5: fork() + daemon() + Process_Manager
	pid_t pid = ACE_OS::fork();
	if ( pid > 0 )
	{
		// parent process exits here
		//ACE_OS::printf("pid=%d\n", pid); //@
	}
	if ( pid == 0 )
	{
		// child process continues...
		daemon(0, 0);
		ACE_Process_Manager* pm = ACE_Process_Manager::instance();
		pid_t start_pid = pm->spawn(opt);
	}
	//*/

	/*
	// Method 1: fork() + Process_Manager
	pid_t pid = ACE_OS::fork();
	if ( pid > 0 )
	{
		// parent process exits here
		//ACE_OS::printf("pid=%d\n", pid); //@
	}
	if ( pid == 0 )
	{
		// child process continues...
		
		//for(int i = ACE::max_handles() - 1; i >= 0; i--)
		//	ACE_OS::close(i);

		ACE_OS::setsid(); // become session leader  
		//ACE_OS::chdir("/"); // change working directory  
		ACE_OS::umask(0); // clear file mode creation mask  
		ACE_OS::close(0); // close stdin
		ACE_OS::close(1); // close stdout
		ACE_OS::close(2); // close stderr

		ACE_Process_Manager* pm = ACE_Process_Manager::instance();
		pid_t start_pid = pm->spawn(opt);
	}
	//*/

	/*
	// Method 2: fork() + system() in background &
	start_cmd = opt.command_line_buf();
	ACE_OS::chdir(ACE::dirname(proc));
	ACE_OS::printf("cmd:%s\n", start_cmd.c_str()); //@

	pid_t pid = ACE_OS::fork();
	if ( pid > 0 )
	{
		// parent process exits here
		//ACE_OS::printf("pid=%d\n", pid); //@
	}
	if ( pid == 0 )
	{
		// child process continues...
		for(int i = ACE::max_handles() - 1; i >= 0; i--)
			ACE_OS::close(i);
		start_cmd += " &";
		ACE_OS::system(start_cmd.c_str());
	}
	//*/

	/*
	// Method 3: daemonize() + system() in background &
	start_cmd += " &";
	ACE::daemonize(ACE::dirname(start_cmd.c_str()), true, start_cmd.c_str());
	ACE_OS::system(start_cmd.c_str());
	//*/

	/*
	// Method 4: nohup
	std::string nohup("nohup ");
	nohup += start_cmd;
	nohup += " &";
	ACE_OS::system(nohup.c_str());
	//*/
#endif

	return rc;
}

// 'svc service stop' == stop and wait forever until the service is stopped
// 'svc service stop abc' == stop and wait forever until the service is stopped
// 'svc service stop -1' == stop with no wait
// 'svc service stop 0' == stop instantly by killing process
// 'svc service stop 10' == stop and wait for 10 seconds
int
Service_Control::wait_for_stop(pid_t pid, const char* service, const char* time_to_wait)
{
	//static const int MAX_STOP_WAIT = 10; // default for 10 seconds

	int rc = 0;

	int n_wait = -1; //MAX_STOP_WAIT;
	if ( time_to_wait )
	{
		n_wait = ACE_OS::atoi(time_to_wait);
		//if ( (*time_to_wait >= '0' && *time_to_wait <= '9') || *time_to_wait == '-')
		//{
		//	n_wait = ACE_OS::atoi(time_to_wait);
		//	if ( n_wait == 0 && *time_to_wait == '-' )
		//		n_wait = MAX_STOP_WAIT;
		//}
		if ( n_wait < 0 )
			return rc;
	}

	ACE_Process_Mutex svc_mutex(service);
	if ( !time_to_wait || ( n_wait == 0 && *time_to_wait != '0' ) )
	{
		// wait until the service is stopped.
		////@? is acquire() wait forever?? may cause bug
		//if ( svc_mutex.acquire() == 0 )
		//	svc_mutex.release();
		///*
		while(1)
		{
			if ( svc_mutex.acquire() == 0 )
			{
				svc_mutex.release();
				break;
			}
		}
		//*/
	}
	else
	{
		// wait for time_to_wait. if not stopped, kill the process.
		ACE_Time_Value tv = ACE_OS::gettimeofday();
		tv += n_wait;

		// wait on process mutex to stop
		// can also wait on pid using ACE_OS::wait(pid), but not reliable on Win32 (pass win32 handle is better)
		if ( aos::ipc::acquire(svc_mutex, tv) == 0 )
		{
			svc_mutex.release();
		}
		else
		{
			ACE::terminate_process(pid);
			rc = -1;
		}
	}

	return rc;
}

// 'svc service start' == try to start with no wait
// 'svc service start abc' == try to start with no wait
// 'svc service start -1' == try to start with no wait
// 'svc service start 0' == try to start and wait forever until the service is started
// 'svc service start 10' == try to start and wait for 10 seconds until the service is started
pid_t
Service_Control::wait_for_start(const char* service, const char* time_to_wait)
{
	pid_t pid = -1;

	if ( time_to_wait )
	{
		ACE_Time_Value sleep_tv; sleep_tv.set(0.01);

		int n_wait = ACE_OS::atoi(time_to_wait);
		if ( n_wait > 0 )
		{
			ACE_Time_Value tv = ACE_OS::gettimeofday();
			tv += n_wait;

			while( tv > ACE_OS::gettimeofday() && (pid = this->pid(service)) <= 0 )
				ACE_OS::sleep(sleep_tv);
		}
		else if ( n_wait == 0 && *time_to_wait == '0' )
		{
			while( (pid = this->pid(service)) <= 0 )
				ACE_OS::sleep(sleep_tv);
		}
	}

	return pid;
}

int
Service_Control::service_exec(const char* service)
{
	int rc = 0;

	if ( ACE_OS::strcmp("start", argv_[2]) == 0 )
	{
		// get plock first, then check pid for start completion
		int plock = this->plock(service);
		if ( !plock )
		{
			Service_Control::SERVICES::const_iterator iter = this->find(service);
			if ( iter != this->end() )
			{
				rc = this->start(service, iter->second.c_str());
				const char* time_to_wait = 0;
				if ( argc_ >= 4 )
					time_to_wait = argv_[3];
				pid_t start_pid = this->wait_for_start(service, time_to_wait);
			}
			else
			{
				rc = -1;
				ACE_OS::printf("%d\t[%s]\tservice command is not found!\n", rc, service);
			}
		}
		else
		{
			rc = -1;

			pid_t p_id = this->pid(service);
			if ( p_id < 0 )
			{
				//ACE_OS::printf("-%d\t[%s]\tservice is still running!\n", p_id, service);
				ACE_OS::printf("%d\t[%s]\tservice is still running!\n", rc, service);
			}
			else
			{
				ACE_OS::printf("+%d\t[%s]\tservice is already running!\n", p_id, service);
			}
		}

		/*
		pid_t p_id = this->pid(service);
		if ( p_id < 0 )
		{
			// get plock first, then check pid for start completion
			int plock = this->plock(service);
			if ( !plock )
			{
				Service_Control::SERVICES::const_iterator iter = this->find(service);
				if ( iter != this->end() )
				{
					rc = this->start(service, iter->second.c_str());
					const char* time_to_wait = 0;
					if ( argc_ >= 4 )
						time_to_wait = argv_[3];
					pid_t start_pid = this->wait_for_start(service, time_to_wait);
				}
				else
				{
					rc = -1;
					ACE_OS::printf("%d\t[%s]\tservice command is not found!\n", rc, service);
				}
			}
			else
			{
				rc = -1;
				//ACE_OS::printf("-%d\t[%s]\tservice is still running!\n", p_id, service);
				ACE_OS::printf("%d\t[%s]\tservice is still running!\n", rc, service);
			}
		}
		else
			ACE_OS::printf("+%d\t[%s]\tservice is already running!\n", p_id, service);
		//*/
	}
	else if ( ACE_OS::strcmp("stop", argv_[2]) == 0 )
	{
		// get pid first, then check plock for stop completion
		pid_t p_id = this->pid(service);
		if ( p_id >= 0 )
		{
			rc = this->stop(service);
			const char* time_to_wait = 0;
			if ( argc_ >= 4 )
				time_to_wait = argv_[3];
			rc = this->wait_for_stop(p_id, service, time_to_wait);
		}
		else
			ACE_OS::printf("+%d\t[%s]\tservice is not running!\n", rc, service);
	}
	else if ( ACE_OS::strcmp("restart", argv_[2]) == 0 )
	{
		// stop
		pid_t p_id;
		p_id = this->pid(service);
		if ( p_id >= 0 )
		{
			rc = this->stop(service);
			//ACE_OS::printf("+%d=pid\trc=%d\n", p_id, rc); //@
			const char* time_to_wait = 0;
			if ( argc_ >= 4 )
				time_to_wait = argv_[3];
			rc = this->wait_for_stop(p_id, service, time_to_wait);
		}
		else
			ACE_OS::printf("+%d\t[%s]\tservice is not running!\n", rc, service);

		// start
		int plock = this->plock(service);
		if ( !plock )
		{
			Service_Control::SERVICES::const_iterator iter = this->find(service);
			if ( iter != this->end() )
			{
				rc = this->start(service, iter->second.c_str());
				const char* time_to_wait = 0;
				if ( argc_ >= 5 )
					time_to_wait = argv_[4];
				pid_t start_pid = this->wait_for_start(service, time_to_wait);
			}
			else
			{
				rc = -1;
				ACE_OS::printf("%d\t[%s]\tservice command is not found!\n", rc, service);
			}
		}
		else
		{
			rc = -1;
			//ACE_OS::printf("-%d\t[%s]\tservice is still running!\n", p_id, service);
			ACE_OS::printf("%d\t[%s]\tservice is still running!\n", rc, service);
		}
	}
	else if ( ACE_OS::strcmp("plock", argv_[2]) == 0 )
	{
		int plock = this->plock(service);
		if ( plock )
			ACE_OS::printf("+%d\t[%s]\tprocess lock exists!\n", plock, service);
		else
			ACE_OS::printf("+%d\t[%s]\tprocess lock removed!\n", plock, service);
	}
	else // all other commands
	{
		std::string cmd(argv_[2]);
		for(int i = 3; i < argc_; ++i)
		{
			cmd += '\t';
			cmd += argv_[i];
		}

		rc = this->exec(service, cmd.c_str());
		if ( rc < 0 )
			ACE_OS::printf("%d\t[%s]\tservice is not running!\t(error: %s)\n", rc, service, ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("connect()");
	}

	return rc;
}

} // namespace ipc
} // namespace aos
