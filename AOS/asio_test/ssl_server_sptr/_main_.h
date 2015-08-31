
using namespace asio;

int run_http_server_test(int argc, ACE_TCHAR* argv[])
{
	asio::error_code error;

	HTTP_Server listener;
	//listener.start(25);

	//listener.ssl_connect(true); error = listener.start(443, 0, 2);
	listener.ssl_connect(false); error = listener.start(80, 0, 2);

	if ( error )
	{
		ACE_OS::printf("start error: %s\n", error.message().c_str());
		return -1;
	}

	while(1)
	{
		ACE_OS::sleep(1);
		ACE_OS::printf("n_conn:%d\n", listener.n_connection());
		//break;
	}

	//ACE_OS::sleep(30);

	listener.stop();

	return 0;
}

int run_ssl_server_test(int argc, ACE_TCHAR* argv[])
{
	SSL_Server listener;
	//listener.start(25);

	//listener.ssl_connect(true); listener.start(443, 0, 2);
	listener.ssl_connect(false); listener.start(80, 0, 2);

	while(1)
	{
		//listener.ssl_accept(!listener.ssl_accept());
		//ACE_OS::printf("ssl_on:%d\n", listener.ssl_accept()?1:0);

		ACE_OS::sleep(1);
		ACE_OS::printf("n_conn:%d\n", listener.n_connection());
		//break;
	}

	listener.stop();

	return 0;
}

