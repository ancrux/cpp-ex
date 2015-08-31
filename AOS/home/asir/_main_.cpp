#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Configuration_Import_Export.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include "MIRS_Score.h"

//#include "SBRS.h"
//#include "SBRS_Loader.h"
//#include "SBRS_Server.h"
//using namespace asio;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = run_mirs_score_test(argc, argv);
	//rc = run_svc_test(argc, argv);
	//rc = run_server_test(argc, argv);
	//rc = run_loader_test(argc, argv);
	//rc = run_ipv4_string_test(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}
