#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Configuration_Import_Export.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include "MP_Worker.h"
#include "MP_Enqueuer.h"

#include <iostream>
using namespace std;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	//ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = run_mp_service(argc, argv);
	//rc = run_msg_test(argc, argv);

	//ACE_Time_Value t2 = ACE_OS::gettimeofday();
	//ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc; 
}
