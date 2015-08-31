#include "ace/OS.h"

#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

#include "aos/String.h"
#include "aos/String_Matcher.h"
#include "aos/SList.h"
using namespace aos;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	rc = run_sorter_test(argc, argv);
	//rc = run_matcher_test(argc, argv);
	//rc = run_trim_test(argc, argv);
	//rc = run_char_map_test(argc, argv);
	//rc = run_slist_test(argc, argv);
	//rc = run_char_tokenizer_test(argc, argv);
	
	//::strtok("abc abc", " ");

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}

