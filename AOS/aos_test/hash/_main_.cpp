#include "ace/OS.h"

#include "aos/hash/FNV.h"
using namespace aos::hash;

#include <iostream>

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();
	
	rc = run_hash_fnv_test(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());


	return rc;
}

