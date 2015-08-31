#include "ace/OS.h"
#include "ace/INET_Addr.h"

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	//rc = run_domain_queue_test(argc, argv);
	//rc = run_mailer_test(argc, argv);
	rc = run_mailer_connection_test(argc, argv);
	//rc = run_gethostbyname_test(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}
