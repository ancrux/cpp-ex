#include "NT_Service.h"

#include "aos/String.h"

#include "ace/Configuration_Import_Export.h"
#include "ace/Process_Manager.h"

NT_Service::NT_Service()
:
console_(0)
{
}

NT_Service::~NT_Service()
{
}

void
NT_Service::handle_control(DWORD control_code)
{
	if (control_code == SERVICE_CONTROL_SHUTDOWN || control_code == SERVICE_CONTROL_STOP )
	{
		this->report_status(SERVICE_STOP_PENDING);

		ACE_DEBUG((LM_INFO, ACE_TEXT ("Service control stop requested\n")));

		this->stop_ = 1;
	}
	else
		ACE_NT_Service::handle_control(control_code);
}

int
NT_Service::svc()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT ("NT_Service_Thread::%t\n")));

	// As an NT service, we come in here in a different thread than the
	// one which created the reactor.  So in order to do anything, we
	// need to own the reactor. If we are not a service, report_status
	// will return -1.
	//if (report_status (SERVICE_RUNNING) == 0)
	//	reactor ()->owner (ACE_Thread::self());

	if ( this->report_status(SERVICE_RUNNING) != 0 && console_ == 0 )
		return 0;

	char path[PATH_MAX+1];
	std::string cmd;
	std::string cmd_stop;

	// read ini file
	std::string ini_file("./run.ini");

	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	iniIO.import_config(ini_file.c_str());

	ACE_Configuration_Section_Key sec;
	ACE_TString key;
	ACE_TString val;
	ACE_Configuration::VALUETYPE val_type;

	config.expand_path(config.root_section(), ACE_TEXT("Run"), sec);

	config.get_string_value(sec, ACE_TEXT("StartCommand"), val);
	cmd = val.c_str();

	config.get_string_value(sec, ACE_TEXT("StopCommand"), val);
	cmd_stop = val.c_str();

///*
	//cmd = "httpd"; cmd += ACE_DIRECTORY_SEPARATOR_CHAR; cmd += "nginx.exe";

	ACE_Process_Options opt;
	opt.command_line(cmd.c_str());
	char* work_dir = ACE_OS::realpath(opt.process_name(), path);
	if ( work_dir )
		opt.working_directory(ACE::dirname(path));

#ifdef ACE_WIN32
	opt.creation_flags(DETACHED_PROCESS); //Win32 use DETACHED_PROCESS, or CREATE_NEW_CONSOLE
	//opt.creation_flags(CREATE_NO_WINDOW);
#endif

	ACE_Process p;
	pid_t pid = p.spawn(opt);
//*/

	//cmd = "svc"; cmd += ACE_DIRECTORY_SEPARATOR_CHAR; cmd += "svc * start 0";
	//ACE_OS::system(cmd.c_str());

	//FILE * fp = ACE_OS::fopen("c:\\service.log", "wb"); //@
	//char* msg = "OK\n"; //@
	//size_t n_msg = ACE_OS::strlen(msg); //@

	ACE_Time_Value sleep_tv; sleep_tv.set(0.1);
	this->stop_ = 0;
	while( !this->stop_ )
	{
		//ACE_OS::fwrite(msg, n_msg, 1, fp); //@
		//ACE_OS::fflush(fp); //@

		ACE_OS::sleep(sleep_tv);
	}

	//ACE_OS::fclose(fp); //@
	//ACE_OS::unlink("c:\\service.log"); //@

	//cmd = "svc"; cmd += ACE_DIRECTORY_SEPARATOR_CHAR; cmd += "svc * stop";
	//ACE_OS::system(cmd.c_str());

	if ( cmd_stop.empty() )
	{
		//ACE::terminate_process(pid);
		p.terminate();
	}
	else
	{
		ACE_Process_Options opt;
		opt.command_line(cmd_stop.c_str());
		char* work_dir = ACE_OS::realpath(opt.process_name(), path);
		if ( work_dir )
			opt.working_directory(ACE::dirname(path));

	#ifdef ACE_WIN32
		opt.creation_flags(DETACHED_PROCESS); //Win32 use DETACHED_PROCESS, or CREATE_NEW_CONSOLE
		//opt.creation_flags(CREATE_NO_WINDOW);
	#endif

		ACE_Process p;
		pid_t pid = p.spawn(opt);

		ACE_exitcode ret = 0;
		//pm->wait(pid, &ret);
		pid = p.wait(&ret);
	}

	return 0;
}

