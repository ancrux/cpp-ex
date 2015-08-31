#include "ace/OS.h"

#include "SSL_Server.h"
#include "HTTP_Server.h"

#include "_main_.h"

#ifdef WIN32
#include "vld.h"
#endif

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;
	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	ACE_OS::printf("sizeof(socket): %d\n", sizeof(asio::ip::tcp::socket));
	ACE_OS::printf("sizeof(ssl_socket): %d\n", sizeof(asio::ssl::stream< ip::tcp::socket >));

	ACE_OS::printf("sizeof(SSL_Server_Connection): %d\n", sizeof(asio::SSL_Server_Connection));

	rc = run_http_server_test(argc, argv);
	//rc = run_ssl_server_test(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}

