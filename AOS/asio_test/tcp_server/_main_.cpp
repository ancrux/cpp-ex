#include "ace/OS.h"

#include "HTTP_Server.h"

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	ACE_OS::printf("sizeof(socket): %d\n", sizeof(asio::ip::tcp::socket));
	ACE_OS::printf("sizeof(ssl_socket): %d\n", sizeof(asio::ssl::stream< ip::tcp::socket >));
	ACE_OS::printf("sizeof(TCP_Server_Connection): %d\n", sizeof(asio::TCP_Server_Connection));
	ACE_OS::printf("sizeof(TCP_Server): %d\n", sizeof(asio::TCP_Server));

	try
	{
		rc = run_http_server_test(argc, argv);
	}
	catch (std::exception& e)
	{
		ACE_OS::printf("exception:%s\n", e.what());
	}

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}

