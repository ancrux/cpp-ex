#include "ace/OS.h"

// for ACE_SOCK_Stream and ACE_SSL_SOCK_Stream
#include "ace/INET_Addr.h"
#include "ace/SSL/SSL_SOCK_Stream.h" //#include "ace/SOCK_Stream.h"
#include "ace/SSL/SSL_SOCK_Acceptor.h" //#include "ace/SOCK_Acceptor.h"
#include "ace/SSL/SSL_SOCK_Connector.h" //#include "ace/SOCK_Connector.h"

#include "aos/net/imap4/IMAP4_Client_IO.h"
#include "aos/net/imap4/IMAP4_Server_IO.h"

#include "aos/net/http/HTTP_Client_IO.h"
#include "aos/net/http/HTTP_Server_IO.h"
#include "HTTP_Server.h"

#include <list>

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	//rc = run_imap4_client_io(argc, argv);
	//rc = run_imap4_server_io(argc, argv);
	rc = run_http_server_io(argc, argv);

	//rc = run_http_io_speed(argc, argv);

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}
