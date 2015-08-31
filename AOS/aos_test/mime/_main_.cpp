#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Dirent.h"

#include "aos/String.h"
#include "aos/mime/MIME_Entity.h"
#include "aos/mime/MIME_Util.h"
#include "aos/mime/Envelope.h"
using namespace aos;

#include "MP_Worker.h"

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include <boost/regex.hpp>
using namespace boost;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	//rc = test_evl(argc, argv);
	rc = test_mp(argc, argv);
	//rc = test_mime(argc, argv);
	//rc = test_codec(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}