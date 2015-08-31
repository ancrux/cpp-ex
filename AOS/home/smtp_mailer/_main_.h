#include "SMTP_Mailer.h"
#include "SMTP_Mailer_Domain_Queue.h"

using namespace asio;

int run_domain_queue_test(int argc, ACE_TCHAR* argv[])
{
	io_service ios;
	SMTP_Mailer mailer(ios);
	//mailer.start();
	mailer.start(1);

	//std::string remote_addr("192.168.1.232");
	std::string remote_addr("192.168.1.19");

	//SMTP_Mailer_Domain_Queue queue(&mailer);
	SMTP_Mailer_Domain_Queue queue;
	ACE_OS::printf("count: %d\n", queue.count());

	/*
	while(1)
	{
		ACE_OS::printf("starting connect()\n");
		for(int i=0; i<20000;)
		{
			mailer.connect(remote_addr.c_str(), 25, 5, 5);
			++i;
			if ( i % 100 == 0 )
			{
				size_t c_conn = mailer.n_connection();
				ACE_OS::printf("c_conn:%d\n", c_conn);
			}
		}
		ACE_OS::printf("connect() completed!\n");
		//break; //@
		
		while(1)
		{
			size_t n_conn = mailer.n_connection();
			ACE_OS::printf("n_conn:%d\n", n_conn);
			if ( n_conn == 0 ) break;
			ACE_OS::sleep(1);
		}
	}
	//*/

	mailer.stop();

	return 0;
}

int run_mailer_test(int argc, ACE_TCHAR* argv[])
{
	io_service ios;
	SMTP_Mailer mailer(ios);
	//mailer.start();
	mailer.start(2);

	//std::string remote_addr("192.168.1.232");
	std::string remote_addr("192.168.1.19");

	while(1)
	{
		ACE_OS::printf("starting connect()\n");
		for(int i=0; i<20000;)
		{
			mailer.connect(remote_addr.c_str(), 25, 5, 5);
			++i;
			if ( i % 100 == 0 )
			{
				size_t c_conn = mailer.n_connection();
				ACE_OS::printf("c_conn:%d\n", c_conn);
			}
		}
		ACE_OS::printf("connect() completed!\n");
		//break; //@
		
		while(1)
		{
			size_t n_conn = mailer.n_connection();
			ACE_OS::printf("n_conn:%d\n", n_conn);
			if ( n_conn == 0 ) break;
			ACE_OS::sleep(1);
		}
	}

	mailer.stop();

	return 0;
}

int run_mailer_connection_test(int argc, ACE_TCHAR* argv[])
{
	bool is_heap = true;
	io_service ios;

	SMTP_Mailer_Job* job = new SMTP_Mailer_Job();
	job->from = "angus@email-home.com";
	job->to = "edm1@zcsdemo.cellopoint.com";
	job->eml = "From: Angus <angus@email-home.com>\r\nTo: Edm1 <edm1@zcsdemo.cellopoint.com\r\nSubject: Test\r\n\r\nThis is a test\r\n";

	std::string remote_addr("192.168.1.19");
	if ( is_heap )
	{
		SMTP_Mailer_Connection* client = new SMTP_Mailer_Connection(ios); //SMTP_Mailer_Connection client(ios);

		client->connect(remote_addr.c_str(), 25, 5, 10, job);
		ios.run();
		delete client;
	}
	else
	{
		SMTP_Mailer_Connection client(ios);
		client.connect(remote_addr.c_str(), 25, 5, 10, job);
		ios.run();
	}

	return 0;
}

int run_gethostbyname_test(int argc, ACE_TCHAR* argv[])
{
	const char* ip_addr;
	for(int i=0; i<10000; ++i)
	{
		ACE_INET_Addr addr(1, "zcsdemo.cellopoint.com"); // port must set
		ip_addr = addr.get_host_addr();
	}
	ACE_OS::printf("ip:%s\n", ip_addr); // ACE_OS::gethostbyname();

	return 0;
}
