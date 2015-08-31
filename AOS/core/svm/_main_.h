#define __SVC_VER__ "0.1.0"

int run_svm_service(int argc, ACE_TCHAR* argv[])
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

	//+ start service
	Service_Monitor svm;
	svm.start();

	aos::Multi_String params;
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
		std::string cmd = stream.buf() + "\n";
		//SVM_LOG->log(cmd.c_str(), cmd.size()); //@
		//ACE_OS::printf("command: %s\n", stream.buf().c_str()); //@

		params.clear();
		aos::Tokenizer toker(stream.buf().c_str(), stream.buf().size());
		toker.set_separator("\t");
		while( toker.next() > aos::Tokenizer::End )
			params.push_back(toker.token(), toker.size());

		// support following command
		// getter: svc svm service; return "-1", "+0", "+N"
		// setter: svc svm service N; (N < 0) remove svm for that service; return "-1/+0/+N svm service" if successful
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
			
			if ( n_send > 0 )
			{
				stop = 1;
				//+ send stop signal, and wait for service to stop
				svm.stop();
			}
		}
		// read "get" command, get monitor info.
		else if ( ACE_OS::strncasecmp(params[0], "get", 3) == 0 )
		{
			char buf[256];
			int n = 0;

			const char* service = params[1];
			if ( service && params.size() == 2 )
			{
				if ( *service != '*' )
				{
					int n_sec = svm.get_monitor(service);
					if ( n_sec < 0 )
						n = ACE_OS::snprintf(buf, 255, "%d\t[%s]\t%s", n_sec, svc.name(), service);
					else
						n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\t%s", n_sec, svc.name(), service);
					n_send = stream.send_cstr(buf, n+1); // include '\0'
				}
				else
				{
					ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, svm.lock(), -1);

					Service_Monitor::MONITORS& monitors = svm.monitors();
					for(Service_Monitor::MONITORS::iterator iter = monitors.begin();
						iter != monitors.end();
						++iter)
					{
						n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\t%s\r\n", iter->second, svc.name(), iter->first.c_str());
						n_send = stream.send_cstr(buf, n);
					}
					n_send = stream.send_cstr("\0", 1); // send null-terminator
				}
			}
			else
			{
				n = ACE_OS::snprintf(buf, 255, "%d\t[%s]\t%s", -1, svc.name(), "Invalid parameters!");
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			}
		}
		// read "set" command, get monitor info.
		else if ( ACE_OS::strncasecmp(params[0], "set", 3) == 0 )
		{
			char buf[256];
			int n = 0;

			const char* service = params[1];
			if ( service && params.size() == 3 )
			{
				int n_sec = ACE_OS::atoi(params[2]); // new value
				if ( *service != '*' )
				{
					svm.set_monitor(service, n_sec);
					if ( n_sec < 0 )
						n = ACE_OS::snprintf(buf, 255, "%d\t[%s]\t%s", n_sec, svc.name(), service);
					else
						n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\t%s", n_sec, svc.name(), service);
					n_send = stream.send_cstr(buf, n+1); // include '\0'
				}
				else
				{
					ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, svm.lock(), -1);

					Service_Monitor::MONITORS& monitors = svm.monitors();
					if ( n_sec < 0 )
					{
						for(Service_Monitor::MONITORS::iterator iter = monitors.begin();
							iter != monitors.end();
							++iter)
						{
							n = ACE_OS::snprintf(buf, 255, "%d\t[%s]\t%s\r\n", -1, svc.name(), iter->first.c_str());
							n_send = stream.send_cstr(buf, n);
						}
						monitors.clear();
						n_send = stream.send_cstr("\0", 1); // send null-terminator
					}
					else
					{
						for(Service_Monitor::MONITORS::iterator iter = monitors.begin();
							iter != monitors.end();
							++iter)
						{
							iter->second = n_sec;
							n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\t%s\r\n", iter->second, svc.name(), iter->first.c_str());
							n_send = stream.send_cstr(buf, n);
						}
						n_send = stream.send_cstr("\0", 1); // send null-terminator
					}
				}
			}
			else
			{
				n = ACE_OS::snprintf(buf, 255, "%d\t[%s]\t%s", -1, svc.name(), "Invalid parameters!");
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			}
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

