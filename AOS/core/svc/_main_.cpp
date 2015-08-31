#include "ace/OS.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	//ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = svc(argc, argv);
	//rc = run_ipc_io_test(argc, argv);
	//rc = run_max_mutex_test(argc, argv);

	//ACE_Time_Value t2 = ACE_OS::gettimeofday();
	//ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc; 
}

