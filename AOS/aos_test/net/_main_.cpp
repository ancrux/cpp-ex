#include "ace/OS.h"

#include "aos/net/http/HTTP_Session.h"
#include "aos/net/http/HTTP.h"

#include "aos/net/smtp/SMTP_Session.h"

#include "aos/net/AIO_Config.h"
#include "aos/net/AIO_Acceptor.h"
#include "aos/net/AIO_Connector.h"

// for ACE_SOCK_Stream and ACE_SSL_SOCK_Stream
#include "ace/INET_Addr.h"
#include "ace/SSL/SSL_SOCK_Stream.h" //#include "ace/SOCK_Stream.h"
#include "ace/SSL/SSL_SOCK_Acceptor.h" //#include "ace/SOCK_Acceptor.h"
#include "ace/SSL/SSL_SOCK_Connector.h" //#include "ace/SOCK_Connector.h"

#include "aos/net/imap4/IMAP4_Client_IO.h"
#include "aos/net/imap4/IMAP4_Server_IO.h"
#include "aos/net/imap4/IMAP4_Session.h"

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	//ACE_OS::printf("sizeof: %d\n", sizeof(AIO_Session));
	//ACE_OS::printf("sizeof: %d\n", sizeof(AIO_SSL_Server_Session));

	int rc = 0;
	rc = run_http_test(argc, argv);
	//rc = run_smtp_test(argc, argv);
	//rc = run_imap4_client_session(argc, argv);

	return rc;
}
