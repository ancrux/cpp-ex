#include "ace/OS.h"

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	try
	{
		//rc = run_resolver_test(argc, argv);
		rc = run_dns_test(argc, argv);
	}
	catch (std::exception& e)
	{
		ACE_OS::printf("exception:%s\n", e.what());
	}

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}

