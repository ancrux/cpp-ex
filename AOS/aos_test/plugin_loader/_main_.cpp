#include "ace/OS.h"
#include "ace/Log_Msg.h"
#include "ace/Dirent.h"
#include "ace/DLL.h"

#include "aos/String.h"

#include "My_Interface.h" // include My_Plugin_Interface

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = run_plugin_loader_test(argc, argv);
	//rc = run_plugin_loader_test2(argc, argv);
	//rc = run_plugin_loader_test3(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc; 
}

