#include "ace/OS.h"
#include "ace/Log_Msg.h"

//#include "aos/mime/MIME_Util.h"
//using namespace aos;

//#include "aos/mailbox/Mailbox.h"
#include "Mailbox_SQLite.h"

#include <string>

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();
	
	rc = run_mbox_api_test(argc, argv);
	//rc = run_mailbox_test(argc, argv);
	//rc = run_mail_entry_test(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	//std::string str("abc; abc;  \r\n");
	//size_t pos = str.find("bc;");

	////size_t pos = str.find_last_not_of("\r\n");
	//////pos = str.find_last_not_of(" \t\v", pos);
	//ACE_OS::printf("(%d)%c\n", pos+2, str[pos+2]);

	return rc;
}

