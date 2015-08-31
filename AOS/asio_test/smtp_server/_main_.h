#include "SMTP_Listener.h"

using namespace asio;

int run_listener_test(int argc, ACE_TCHAR* argv[])
{
	io_service ios;
	SMTP_Listener listener(ios);
	//listener.start(25);
	listener.start(25, 0, 8);

	while(1)
	{
		ACE_OS::sleep(1);
		ACE_OS::printf("n_conn:%d\n", listener.n_connection());
		//break;
	}

	listener.stop();

	return 0;
}

