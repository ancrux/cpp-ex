
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

int run_http_client_test(int argc, ACE_TCHAR* argv[])
{
	ACE_Sig_Action sa(signal_handler);
	ACE_Sig_Set ss;
	ss.sig_add(SIGINT);
	sa.mask(ss);
	sa.register_action(SIGINT);

	asio::error_code error;
	HTTP_Client client;
	// TCP_Client client;
	client.max_reuse_connection(10);

	ACE_Time_Value t1, t2;

	ACE_Time_Value sleep_tv; sleep_tv.set(1.0);
	int count = 0;
	int stop = 0;
	while( !stop )
	{
		++count;

		ACE_OS::printf("\n=====client start()=====\n");
		client.start(2);

		size_t n_request = 5000; // # of requests
		size_t n_concurrent = 1000; // # of concurrent connections
		long connect_timeout = 10; // connect timeout in second
		long socket_timeout = 10; // socket timeout in second

		t1 = ACE_OS::gettimeofday();

		ACE_Time_Value delay_tv; delay_tv.set(0.0001);
		for(size_t i=0; i < n_request && !cancel; ++i) // # of requests
		{
			client.connect(true, 443, "127.0.0.1", connect_timeout, socket_timeout);
			//client.connect(false, 80, "127.0.0.1", connect_timeout, socket_timeout);
			//client.connect(true, 443, "192.168.1.39", connect_timeout, socket_timeout);
			//client.connect(false, 80, "192.168.1.39", connect_timeout, socket_timeout);

			if ( i % 1000 == 0 )
				ACE_OS::printf("connect() n_conn:%d\n", client.n_connection());

			if ( i > n_concurrent )
			{
				while( client.n_connection() >= n_concurrent ) // # of concurrent connections
				{
					ACE_OS::sleep(delay_tv);
				}
			}
		}
		ACE_OS::printf("connect() n_conn:%d\n", client.n_connection());

		int rc_resize = -1;
		//for(int i=0;i < 20;++i)
		for(int i=0;;++i)
		{
			if ( cancel == 1 )
			{
				stop = 1;
				break;
			}

			if ( client.n_connection() == 0 )
			{
				stop = 1;
				break;
			}

			ACE_OS::sleep(sleep_tv);
			ACE_OS::printf("use/reuse:%d/%d\n", client.n_connection(), client.n_reuse_connection());
			
			///*
			//rc_resize = client.resize_thread_pool((i%10)+1);
			//ACE_OS::printf("n_thread:%d\n", client.thr_count());
			//*/
		}
		t2 = ACE_OS::gettimeofday();
		
		ACE_OS::printf("\n=====client stop()=====\n");
		client.stop();
	}

	double elapsed =  (double) (t2.msec()-t1.msec()); elapsed /= 1000;
	double request_per_sec = client.n_ok.value()/elapsed;
	long sum =
		client.n_ok.value() +
		client.n_ce.value() +
		client.n_ct.value() +
		client.n_he.value() +
		client.n_re.value() +
		client.n_we.value() +
		client.n_st.value();

	ACE_OS::printf("\n");
	ACE_OS::printf("ok: %d\n", client.n_ok.value());
	ACE_OS::printf("ce: %d\n", client.n_ce.value());
	ACE_OS::printf("ct: %d\n", client.n_ct.value());
	ACE_OS::printf("he: %d\n", client.n_he.value());
	ACE_OS::printf("re: %d\n", client.n_re.value());
	ACE_OS::printf("we: %d\n", client.n_we.value());
	ACE_OS::printf("st: %d\n", client.n_st.value());
	ACE_OS::printf("sum: %d\n", sum);
	ACE_OS::printf("time: %.3f sec(s)\n", elapsed);
	ACE_OS::printf("req/sec: %.3f\n", request_per_sec);
	ACE_OS::printf("\n");

	return 0;
}
