
using namespace asio;

#if defined(_WIN32)
//#include "vld.h"
#endif

#include "ace/Signal.h"
static int cancel = 0;
static void signal_handler(int sig_no)
{
	ACE_OS::printf("SIGNAL: %d\n", sig_no);
	cancel = 1;
}

int run_http_server_test(int argc, ACE_TCHAR* argv[])
{
	ACE_Sig_Action sa(signal_handler);
	ACE_Sig_Set ss;
	ss.sig_add(SIGINT);
	sa.mask(ss);
	sa.register_action(SIGINT);

	asio::error_code error;
	// TCP_Server server;
	HTTP_Server server; 
	server.max_reuse_connection(10);
	server.max_connection(0); //server.max_connection(-1); // refuse new connections

	ACE_Time_Value sleep_tv; sleep_tv.set(1.0);
	int count = 0;
	int stop = 0;
	while( !stop )
	{
		//if ( count >= 3 ) break;
		++count;

		ACE_OS::printf("\n=====server start()=====\n");
		server.ssl_connect(true); error = server.start(443, 0, 2);
		//server.ssl_connect(false); error = server.start(80, 0, 2);
		if ( error )
		{
			ACE_OS::printf("server start error: %s\n", error.message().c_str());
			break;
		}
		
		int rc_resize = -1;
		//for(int i=0;i < 20;++i)
		for(int i=0;;++i)
		{
			if ( cancel == 1 )
			{
				stop = 1;
				break;
			}

			ACE_OS::sleep(sleep_tv);
			ACE_OS::printf("use/reuse:%d/%d\n", server.n_connection(), server.n_reuse_connection());

			/*
			rc_resize = server.resize_thread_pool((i%10)+1);
			//ACE_OS::printf("n_thread:%d\n", server.thr_count());
			//*/
		}
		ACE_OS::printf("\n=====server stop()=====\n");
		server.stop();
	}

	return 0;
}

