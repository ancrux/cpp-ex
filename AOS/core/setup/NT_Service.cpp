#include "NT_Service.h"

#include "aos/String.h"

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

	std::string cmd;
	cmd = "svc"; cmd += ACE_DIRECTORY_SEPARATOR_CHAR; cmd += "svc * start 0";
	ACE_OS::system(cmd.c_str());

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

	cmd = "svc"; cmd += ACE_DIRECTORY_SEPARATOR_CHAR; cmd += "svc * stop";
	ACE_OS::system(cmd.c_str());

	return 0;
}

