#define __SVC_VER__ "0.1.0"

int run_mp_service(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	Service_Process svc(argc, argv, __SVC_VER__, __DATE__ " " __TIME__);
	ACE_OS::printf("service: %s\n", svc.name());

	// start local_acceptor
	aos::ipc::Local_Acceptor acceptor;
	ACE_Time_Value timeout(1);

	if ( acceptor.open(svc.name()) == -1 )
	{
		ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
		return -1;
	}

	// start service
	MP_Worker worker;
	worker.start(ACE_OS::num_processors_online());
	MP_Enqueuer enqueuer(worker);
	enqueuer.start();

	int stop = 0;
	while(!stop)
	{
		aos::ipc::Local_Stream stream;
		if ( acceptor.accept(stream) == -1 )
		{
			//ACE_OS::printf("accept() == -1\n"); //@
			continue;
		}

		//ACE_OS::printf("client accepted!\n"); //@

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		n_recv = stream.recv_cstr();
		//ACE_OS::printf("command: %s\n", stream.buf().c_str()); //@

		if ( (n_send = svc.handle_predefined_command(stream)) > 0 )
		{
			// do nothing
		}
		// read "stop" command, stop service.
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "stop") == 0 )
		{
			char buf[256];
			int n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\tservice is stopping...", svc.pid(), svc.name());
			n_send = stream.send_cstr(buf, n+1); // include '\0'
			
			stop = 1;
			// send stop signal, and wait for service to stop
			enqueuer.stop();
			enqueuer.wait();
			worker.stop();
			//// if program will exit right after, don't care about msg_queue.flush()
			//size_t n_flush = worker.msg_queue()->flush();
			//ACE_OS::printf("%d flushed\n", n_flush);
			worker.wait();
		}
		else
		{
			n_send = svc.handle_unknown_command(stream);
		}
		stream.close();
	}
	acceptor.close();

	return rc;
}

int run_msg_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	std::string clone("123");

	int n_max = 10000000;

	//for(int i=0; i < n_max; ++i) std::string* msg = new std::string(clone);
	for(int i=0; i < n_max; ++i) MP_Message* msg = new MP_Message(clone);
	//for(int i=0; i < n_max; ++i) ACE_Message_Block* msg = new ACE_Message_Block(clone.size());

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	return rc;
}