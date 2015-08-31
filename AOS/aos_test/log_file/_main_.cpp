#include "ace/OS.h"
#include "ace/Log_Msg.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Dgram.h"

#include <iostream>
using namespace std;

#include "aos/Logger.h"
#include "Log_Store.h"
#include "Log_Worker.h"
#include "Log_Monitor.h"

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	rc = run_logger2_test(argc, argv);
	//rc = run_worker_test(argc, argv);
	//rc = run_queue_test(argc, argv);
	return rc;
}