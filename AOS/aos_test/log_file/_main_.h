
int run_logger2_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	aos::Logger logger(".", "my_", "log", aos::Logger::Rotate::SECOND, 5);
	ACE_OS::printf("sizeof:%d\n", sizeof(logger));
	ACE_OS::printf("dir='%s', prefix='%s', suffix='%s'\n", logger.dir(), logger.prefix(), logger.suffix());

	//std::string msg("123456789\n");
	char msg[128];
	ACE_OS::memset(msg, 0, sizeof(msg));
	//for(int i = 0; i < 500000000; ++i)
	for(int i = 0; i < 50000; ++i)
	{
		ACE_OS::itoa(i+1, msg, 10);
		size_t n_msg = ACE_OS::strlen(msg);
		msg[n_msg] = '\n';
		msg[n_msg+1] = '\0';
		logger.log(msg);
		
		//store.log(*str);
		///*
		if ( i % 100 == 0 )
		{
			timespec sleep_ts;
			sleep_ts.tv_sec = 0;
			sleep_ts.tv_nsec = 600 * 1000 * 1000;

			ACE_OS::nanosleep(&sleep_ts);
		}
		//*/
	}

	return rc;
}

int run_worker_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	mozart::Log_Store store(".");

	mozart::Log_Monitor monitor;
	monitor.activate();

	mozart::Log_Worker worker;
	worker.log_store(&store);
	worker.log_monitor(&monitor);
	worker.activate(THR_NEW_LWP | THR_JOINABLE, 1); //ACE_OS::num_processors_online());

	std::string msg("123456789\n");

	for(int i = 0; i < 500000000; ++i)
	{
		ACE_OS::itoa(i+1, &msg[0], 10);
		
		//ACE_Message_Block* mb = new ACE_Message_Block(40);
		//mb->copy(msg.c_str(), msg.size());
		////mb->copy(msg.c_str());
		//worker.msg_queue()->enqueue_tail(mb);

		//store.log(mb->base(), mb->length());

		std::string* str = new std::string(msg);
		worker.msg_queue()->enqueue_tail(str);
		
		//store.log(*str);
		/*
		if ( i % 100 == 0 )
		{
			timespec sleep_ts;
			sleep_ts.tv_sec = 0;
			sleep_ts.tv_nsec = 600 * 1000 * 1000;

			ACE_OS::nanosleep(&sleep_ts);
		}
		//*/
	}

	/*
	// udp log server
	ACE_INET_Addr local_addr(16800);
	ACE_INET_Addr remote_addr;
	ACE_SOCK_Dgram udp(local_addr);

	static const int BUFSIZE = 4096;
	char buf[BUFSIZE];

	ssize_t n_recv = 0;
	do
	{
		n_recv = udp.recv(buf, BUFSIZE, remote_addr);
		::printf("%s\n", buf);
	}
	while( n_recv > 0 );

	udp.close();
	//*/

	// send terminate signal
	worker.msg_queue()->enqueue_tail(new std::string());
	worker.wait();
	
	monitor.msg_queue()->enqueue_tail(new std::string());
	monitor.wait();

	return rc;
}

int run_queue_test(int argc, ACE_TCHAR* argv[])
{
	mozart::Log_Recycler recycler;
	recycler.msg_queue()->high_water_mark(-1);
	//recycler.activate(THR_NEW_LWP | THR_JOINABLE, 1);

	ACE_Thread_Mutex mtx;

	mozart::MyQueue q;

	//ACE_OS::printf("%d\n", sizeof(ACE_Time_Value));

	ACE_Time_Value t1 = ACE_OS::gettimeofday();
	for(int i = 0; i < 1000000; ++i)
	{
		//ACE_Time_Value tv = ACE_OS::gettimeofday();
		//time_t tt = ACE_OS::time();

		//ACE_Message_Block* mb = new ACE_Message_Block(128);
		//char* cstr = new char[40];
		//std::string* str = new std::string(128, 0);

		//ACE_Message_Block* mb;// = recycler.get_mb();
		////if ( mb == 0 ) 
		//	mb = new ACE_Message_Block(10);
		//recycler.msg_queue()->enqueue_tail(mb);

		//ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, mtx, -1);
		//ACE_GUARD(ACE_Thread_Mutex, ace_mon, mtx);

		q.enqueue(0);
	}
	ACE_Time_Value t2 = ACE_OS::gettimeofday();

	ACE_OS::printf("elaspe: %d\n", t2.msec() - t1.msec());

	getchar();

	return 0;
}

// main.h