#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Log_Msg.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include "System_Stat.h"
//#include "Cello_Directory_Updater.h"

#include <iostream>
using namespace std;

#include "_main_.h"

#include "ace/Process.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	//ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = run_svc_test(argc, argv); // main program
	//rc = run_ipc_io_test(argc, argv);
	//rc = run_all_test(argc, argv);

	//ACE_Time_Value t2 = ACE_OS::gettimeofday();
	//ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}

