#include "Service_Monitor.h"

#include "ace/Get_Opt.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Process_Manager.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include "SVM_Logger.h"

Service_Monitor::Service_Monitor()
:
argc_(0),
argv_(0)
{
}

Service_Monitor::Service_Monitor(int argc, ACE_TCHAR* argv[])
:
argc_(argc),
argv_(argv)
{
}

Service_Monitor::~Service_Monitor()
{
}

int
Service_Monitor::import_svc_ini(const char* ini_file)
{
	int rc = -1;

	// load ini file
	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	rc = iniIO.import_config(ini_file);
	if ( rc != 0 )
		return rc;

	// clear monitors
	monitors_.clear();
	ini_file_ = "";

	// read ini
	ACE_TString name;
	for(int i = 0;
		config.enumerate_sections(config.root_section(), i, name) == 0;
		++i)
	{
		// not [service] section
		if ( !ACE_OS::ace_isalnum(*(name.c_str())) )
			continue;

		ACE_Configuration_Section_Key sec;
		config.open_section(config.root_section(), name.c_str(), 0, sec);
		
		// read svm
		ACE_TString str_svm;
		config.get_string_value(sec, ACE_TEXT("svm"), str_svm);
		// no svm value
		if ( str_svm.is_empty() )
			continue;

		int monitor_sec = ACE_OS::atoi(str_svm.c_str());
		if ( monitor_sec >= 0 )
			monitors_.insert(std::make_pair(name.c_str(), monitor_sec));
	}

	return rc;
}

int
Service_Monitor::load(const char* ini_file)
{
	int rc = -1;

	// load ini file
	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	rc = iniIO.import_config(ini_file_.c_str());
	if ( rc != 0 )
		return rc;

	// clear monitors
	monitors_.clear();
	ini_file_ = ini_file;

	// read monitors
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
		int monitor_sec = ACE_OS::atoi(value.c_str());

		if ( monitor_sec >= 0 )
			monitors_.insert(std::make_pair(name.c_str(), monitor_sec));
	}

	return rc;
}

int
Service_Monitor::save(const char* ini_file)
{
	int rc = -1;

	if ( ini_file )
		ini_file_ = ini_file;

	FILE* fp = ACE_OS::fopen(ini_file_.c_str(), "wb");
	if ( fp )
	{
		std::string ini_body("[*]\r\n");
		for(MONITORS::const_iterator iter = this->begin();
			iter != this->end();
			++iter)
		{
			char buf[1024]; // n_sec
			ini_body += iter->first;
			ini_body += "=";
			ini_body += ACE_OS::itoa((int) iter->second, buf, 10); 
			ini_body += "\r\n";
		}

		ACE_OS::fwrite(ini_body.c_str(), ini_body.size(), 1, fp);
		ACE_OS::fclose(fp);
		rc = 0;
	}

	return rc;
}

int
Service_Monitor::get_monitor(const char* service) const
{
	int n_sec = -1;
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, this->lock(), n_sec);

		MONITORS::const_iterator iter = this->find(service);
		if ( iter != monitors_.end() )
			n_sec = (int) iter->second;
	}

	return n_sec;
}

void
Service_Monitor::set_monitor(const char* service, int n_sec)
{
	{
		ACE_GUARD(ACE_Thread_Mutex, guard, this->lock());

		monitors_.erase(service);
		if ( n_sec >= 0 )
			monitors_.insert(std::make_pair(service, n_sec));
	}
}

void
Service_Monitor::start()
{
	stop_ = 0;
	this->activate();
}

void
Service_Monitor::stop()
{
	stop_ = 1;
	this->wait();
}

int
Service_Monitor::svc(void)
{
	ACE_OS::printf("(%u) Service_Monitor starting up...\n", ACE_OS::thr_self());

	int use_svm_ini = 0;

	// Get_Opt
	ACE_Get_Opt cmd(this->argc_, this->argv_);
	cmd.long_option(ACE_TEXT("svm"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("svc"), ACE_Get_Opt::NO_ARG);

	int ch;
	while( (ch = cmd()) != EOF )
	{
		switch(ch)
		{
		case 0:
			if ( ACE_OS::strcasecmp(cmd.last_option(), "svm") == 0 )
				use_svm_ini = 1;
			break;
		}
	}

	// load monitors
	( use_svm_ini )?this->load("svm.ini"):this->import_svc_ini("svc.ini");

	Service_Control sc(0, 0);
	sc.load("svc.ini");

	ACE_Process_Manager* pm = ACE_Process_Manager::instance();

	size_t n_count = 0;
	while( !stop_.value() )
	{
		{
			ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, this->lock_, -1);

			for(MONITORS::iterator it = monitors_.begin();
				it != monitors_.end();
				++it)
			{
				if ( it->second > 0 && n_count % it->second == 0 )
				{
					const char* service = it->first.c_str();

					int rc = 0;
					//+ get pid first. if connected, don't call start()
					int plock = sc.plock(service); // pid_t p_id = sc.pid(service);
					if ( !plock ) // if ( p_id < 0 )
					{
						std::string cmd(".");
						cmd += ACE_DIRECTORY_SEPARATOR_CHAR;
						cmd += "svc "; cmd += service; cmd += " start";

						/*
						//?? better to wake up service by system()
						ACE_OS::system(cmd.c_str());
						//*/

						ACE_Process_Options opt;
						opt.command_line(cmd.c_str());
#ifdef ACE_WIN32
						opt.creation_flags(CREATE_NO_WINDOW);
#endif
						///*
						pid_t start_pid = pm->spawn(opt);
						if ( start_pid != ACE_INVALID_PID )
							pm->wait(start_pid);
						//*/

						// log: service is starting again
						char buf[256];
						int n_buf = ACE_OS::snprintf(buf, 255, "[%s] is starting again!\n", service);
						SVM_LOG->log(buf, n_buf);

						//ACE_OS::printf("%s", buf); //@

						/*
						Service_Control::SERVICES::const_iterator iter = sc.find(service);
						if ( iter != sc.end() )
						{
							rc = sc.start(service, iter->second.c_str());
							const char* time_to_wait = 0;
							pid_t start_pid = sc.wait_for_start(service, time_to_wait);

							// log: service is starting again
							//+ log restart event here!!
							char buf[256];
							int n_buf = ACE_OS::snprintf(buf, 255, "[%s] is starting again!\n", service);
							SVM_LOG->log(buf, n_buf);
							//SVM_LOGGER()->log(buf, n_buf, &ACE_OS::gettimeofday());
						}
						else
						{
							rc = -1;
							//+ log: service is not found // don't log??
							ACE_OS::printf("%d\t[%s]\tservice command is not found!\n", rc, service); //@
						}
						//*/
					}
					else
					{
						//+ log: service is already running // don't log??
						ACE_OS::printf("+%d\t[%s]\tservice is still running!\n", rc, service); //@
					}
				}
			}
		}

		++n_count;
		
		ACE_Time_Value sleep_tv; sleep_tv.set(0.1);
		for(int i=0; i<10; ++i)
		{
			if ( stop_.value() ) break;
			ACE_OS::sleep(sleep_tv);
		}
	}

	//pm->wait();

	ACE_OS::printf("(%u) Service_Monitor shutting down...\n", ACE_OS::thr_self());

	return 0;
}
