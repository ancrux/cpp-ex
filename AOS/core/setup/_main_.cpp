#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Configuration_Import_Export.h"

#include "aos/String.h"

#include "V8.h"

#ifdef ACE_WIN32

#include "NT_Service.h"

ACE_NT_SERVICE_DEFINE(
	NTSVC_ENTRY,
	NT_Service,
	NTSVC_DESC
	);

#endif //ACE_WIN32

#include <iostream>
using namespace std;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	int install = 0;
	int uninstall = 0;
	int start = 0;
	int stop = 0;
	int console = 0;

	// Get_Opt
	ACE_Get_Opt cmd(argc, argv);
	cmd.long_option(ACE_TEXT("install"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("uninstall"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("start"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("stop"), ACE_Get_Opt::NO_ARG);
	cmd.long_option(ACE_TEXT("console"), ACE_Get_Opt::NO_ARG);

	int ch;
	while( (ch = cmd()) != EOF )
	{
		//ACE_OS::printf("index:%d=%c\n", cmd.opt_ind(), ch); //@
		switch(ch)
		{
		case 0:
			if ( ACE_OS::strcasecmp(cmd.last_option(), "install") == 0 )
			{
				install = 1;
			}
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "uninstall") == 0 )
			{
				uninstall = 1;
			}
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "start") == 0 )
			{
				start = 1;
			}
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "stop") == 0 )
			{
				stop = 1;
			}
			else if ( ACE_OS::strcasecmp(cmd.last_option(), "console") == 0 )
			{
				console = 1;
			}
			////else if ( cmd.argc() == 2 && ACE_OS::strcasecmp(cmd.last_option(), "build_timestamp") == 0 )
			//else if ( ACE_OS::strcasecmp(cmd.last_option(), "build_timestamp") == 0 )
			//{
			//	char date[64];
			//	ACE_OS::strcpy(date, "");
			//	if ( this->build_ != ACE_Time_Value::zero )
			//	{
			//		time_t build = this->build_timestamp().sec();
			//		struct tm tm_val;
			//		ACE_OS::localtime_r(&build, &tm_val);
			//		size_t n_buf = ACE_OS::strftime(date, 63, "%Y-%m-%d %H:%M:%S", &tm_val);
			//	}
			//	ACE_OS::printf("%s\n", date);
			//	ACE_OS::exit(0);
			//}
			break;
		//case 's':
		//	if ( cmd.opt_arg() )
		//		svc_arg = cmd.opt_arg();
		//	break;
		}
	}

	//while(loop)
	//	ACE_OS::sleep(1);

#ifdef ACE_WIN32

	NT_SERVICE::instance()->name(
		NTSVC_NAME,
		NTSVC_DESC
		);

	if ( install )
		return NT_SERVICE::instance()->insert(SERVICE_AUTO_START);
	
	if ( uninstall )
		return NT_SERVICE::instance()->remove();

	if ( start )
		return NT_SERVICE::instance()->start_svc();

	if ( stop )
		return NT_SERVICE::instance()->stop_svc();

	// run service here
	rc = setup(argc, argv);

	if ( console )
	{
		::SetConsoleCtrlHandler(&ConsoleHandler, 1);
		NT_SERVICE::instance()->console(1);
		NT_SERVICE::instance()->svc();
	}
	else
	{
		ACE_NT_SERVICE_RUN(
			NTSVC_ENTRY,
			NT_SERVICE::instance(),
			ret
			);
	}

#else

	// run service here
	rc = setup(argc, argv);

#endif

	return rc;
}
