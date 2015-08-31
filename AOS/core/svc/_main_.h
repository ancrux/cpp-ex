
int svc(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	if ( argc < 3 )
	{
		ACE_OS::printf("Usage: %s <service> <command>\n", ACE::basename(argv[0]));
		rc = -1;
		return rc;
	}

	//+ Todo:
	// support '$' for system commands
	// support '@' for getting service which matches attribute value
	// support '+' for adding service entry
	// support '-' for removing service entry
	// keep track of active services in 'svc.lst' file via start/stop command
	// finally, a svc_httpd as a service control center: svc manager/gateway/host
	// every machine can join a team as team leader/member
	// or, write a svc.php?s=[service_name]&c=[command] that accept
	// connection from 127.0.0.1(localhost) only.

	Service_Control sc(argc, argv);

	if ( *(argv[1]) == '$' )
	{
		// exec system command
	}
	else if ( *(argv[1]) == '?' )
	{
		// query ini [?] values
		std::string query_key(argv[2]);

		if ( !query_key.empty() )
		{
			// load ini file
			ACE_Configuration_Heap config;
			config.open();

			ACE_Ini_ImpExp iniIO(config);
			iniIO.import_config(sc.default_ini_file().c_str());

			ACE_Configuration_Section_Key sec;
			ACE_TString val;

			config.expand_path(config.root_section(), ACE_TEXT("?"), sec);
			config.get_string_value(sec, query_key.c_str(), val);

			//::printf("[?] %s=%s\n", query_key.c_str(), val.c_str());
			if ( !val.empty() ) ACE_OS::printf("%s\n", val.c_str());
		}
	}
	else if ( *(argv[1]) == '@' )
	{
		// exec command that service's attribute matches key==value
		std::string expr(argv[1]); // expression
		std::string attrib_key, attrib_val;

		size_t eq = expr.find_first_of("==", 1);
		if ( eq != std::string::npos )
		{
			attrib_key = expr.substr(1, eq-1);
			attrib_val = expr.substr(eq+2);
		}

		if ( attrib_key.empty() || attrib_val.empty() )
		{
			ACE_OS::printf("Usage: %s @key==value <command>\n", ACE::basename(argv[0]));
			rc = -1;
			return rc;
		}

		// load ini file
		ACE_Configuration_Heap config;
		config.open();

		ACE_Ini_ImpExp iniIO(config);
		iniIO.import_config(sc.default_ini_file().c_str());

		ACE_Configuration_Section_Key sec;
		ACE_TString val;

		sc.load(sc.default_ini_file().c_str()); //sc.dump_services(); //@
		const Service_Control::SERVICES& services = sc.services();
		if ( ACE_OS::strcmp("stop", argv[2]) != 0 )
		{
			for(Service_Control::SERVICES::const_iterator iter = services.begin(); iter != services.end(); ++iter)
			{
				val.resize(0);
				config.expand_path(config.root_section(), iter->first.c_str(), sec);
				config.get_string_value(sec, attrib_key.c_str(), val);
				//::printf("[%s] %s=%s\n", iter->first.c_str(), attrib_key.c_str(), val.c_str());

				if ( ACE_OS::strcmp(attrib_val.c_str(), val.c_str()) == 0 )
					rc = sc.service_exec(iter->first.c_str());
			}
		}
		else
		{
			for(Service_Control::SERVICES::const_reverse_iterator iter = services.rbegin(); iter != services.rend(); ++iter)
			{
				val.resize(0);
				config.expand_path(config.root_section(), iter->first.c_str(), sec);
				config.get_string_value(sec, attrib_key.c_str(), val);
				//::printf("[%s] %s=%s\n", iter->first.c_str(), attrib_key.c_str(), val.c_str());

				if ( ACE_OS::strcmp(attrib_val.c_str(), val.c_str()) == 0 )
					rc = sc.service_exec(iter->first.c_str());
			}
		}
	}
	else if ( *(argv[1]) == '*' )
	{
		// exec all services command
		sc.load(sc.default_ini_file().c_str()); //sc.dump_services(); //@
		const Service_Control::SERVICES& services = sc.services();
		if ( ACE_OS::strcmp("stop", argv[2]) != 0 )
		{
			for(Service_Control::SERVICES::const_iterator iter = services.begin(); iter != services.end(); ++iter)
			{
				rc = sc.service_exec(iter->first.c_str());
			}
		}
		else
		{
			for(Service_Control::SERVICES::const_reverse_iterator iter = services.rbegin(); iter != services.rend(); ++iter)
			{
				rc = sc.service_exec(iter->first.c_str());
			}
		}
	}
	else
	{
		// exec single service command
		if ( ACE_OS::strcmp("start", argv[2]) == 0 || ACE_OS::strcmp("restart", argv[2]) == 0 )
		{
			sc.load(sc.default_ini_file().c_str()); //sc.dump_services(); //@
		}
		rc = sc.service_exec(argv[1]);
	}

	return rc;
}

int run_ipc_io_test(int argc, ACE_TCHAR* argv[])
{
	///*
	static const int bsize = 4095;
	const char* service = "sys_stat";
	char command[bsize+1];

	memset(command, 'C', bsize);
	command[bsize] = '\0';

	ACE_Time_Value t1 = ACE_OS::gettimeofday();

	aos::ipc::Local_Stream stream;
	aos::ipc::Local_Connector connector;
	ACE_Time_Value timeout(1);

	if ( connector.connect(stream, service) == -1 )
	{
		//ACE_OS::printf("-1 '%s' is not running! (error: %s)\n", service, ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("connect()");
		return -1;
	}

	std::string str;
	
	ssize_t total_send = 0;
	ssize_t total_recv = 0;
	for(int i = 0; i < 10000; ++i)
	{
		ssize_t n_send = stream.send_cstr(command, bsize+1); // include '\0'
		if ( n_send < 0 ) return -1;
		total_send += n_send;

		ssize_t n_recv = stream.recv_cstr();
		if ( n_recv < 0 ) return -1;
		total_recv += n_recv;
		
		//str.append(stream.buf()); ::printf("%s\n", str.c_str());
		//stream.close();
	}
	stream.close();

	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	::printf("total_send: %d\n", total_send);
	::printf("total_recv: %d\n", total_recv);

	return 0;
	//*/
}

int run_max_mutex_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	for(int i = 0; i < 1000; ++i)
	{
		char name[128];
		ACE_OS::itoa(i, name, 10);
		ACE_Process_Mutex* mutex = new ACE_Process_Mutex(name);

		int res1 = mutex->tryacquire();

		printf("mutex: %d (%d)\n", i, res1);
	}
	getchar();

	return rc;
}




