
int run_mirs_score_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	MIRS_Score score;

	ACE_Time_Value t1 = ACE_OS::gettimeofday();
	int max = 20*1000*1000;
	for(int i=0; i<max; ++i)
	{
		score.insert(i, 1);
	}
	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

	ACE_OS::printf("size:%d\n", score.size());

	return rc;
}

/*
#define __SVC_VER__ "0.1.0 (alpha)"

int run_svc_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	Service_Process svc(argc, argv, __SVC_VER__, __DATE__ " " __TIME__);
	ACE_OS::printf("service: %s\n", svc.name());

	unsigned short server_port = 56789;
	long server_timeout = 60;
	int n_loader = 0;
	std::string dump_file;

	ACE_Get_Opt cmd(argc, argv);
	cmd.long_option(ACE_TEXT("loader"), 'l', ACE_Get_Opt::ARG_REQUIRED);
	cmd.long_option(ACE_TEXT("dump"), ACE_Get_Opt::ARG_REQUIRED);
	int ch;
	while( (ch = cmd()) != EOF )
	{
		switch(ch)
		{
		case 0:
			if ( ACE_OS::strcasecmp(cmd.last_option(), "dump") == 0 )
			{
				dump_file = cmd.opt_arg();
			}
			break;
		case 'l':
			n_loader = ACE_OS::atoi(cmd.opt_arg());	
			break;
		}
	}
	if ( dump_file.size() ) n_loader = 1;

	// change to program's directory
	char path[PATH_MAX+1];
	ACE_OS::realpath(argv[0], path);
	ACE_OS::chdir(ACE::dirname(path, ACE_DIRECTORY_SEPARATOR_CHAR));

	std::string ini_file(ACE::basename(path, ACE_DIRECTORY_SEPARATOR_CHAR));
	size_t last_dot = ini_file.rfind('.');
	if ( last_dot != std::string::npos )
		ini_file.resize(last_dot);
	ini_file += ".ini";
	ACE_OS::printf("ini_file=%s\n", ini_file.c_str());

	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	iniIO.import_config(ini_file.c_str());

	ACE_Configuration_Section_Key sec;
	ACE_TString val;

	config.expand_path(config.root_section(), ACE_TEXT("Server"), sec);
	config.get_string_value(sec, ACE_TEXT("Port"), val);

	unsigned short port = (unsigned short) ACE_OS::atoi(val.c_str());
	server_port = (port > 0 && port < 65536)?port:server_port;
	ACE_OS::printf("Port=%u\n", server_port);

	config.expand_path(config.root_section(), ACE_TEXT("Server"), sec);
	config.get_string_value(sec, ACE_TEXT("Timeout"), val);

	long timeout = (long) ACE_OS::atoi(val.c_str());
	server_timeout = (timeout > 0)?timeout:server_timeout;
	ACE_OS::printf("Timeout=%d\n", server_timeout);

	int n_map = ACE_OS::num_processors_online();
	if ( n_map < 1 ) n_map = 1;
	if ( n_map > 8 ) n_map = 8;
	if ( n_loader ) n_map = n_loader;

	// create maps
	SBRS_MAPS maps;
	for(int i=0; i < n_map; ++i)
	{
		maps.push_back(new SBRS_MAP());
	}


	//// load maps
	//ACE_Time_Value t1 = ACE_OS::gettimeofday();
	//SBRS_Loader loader(maps);
	//loader.start(n_map);
	//ACE_OS::printf("n_map=%d\n", n_map);
	//loader.wait();
	//ACE_Time_Value t2 = ACE_OS::gettimeofday();
	//ACE_OS::printf("loading elasped:%d\n", t2.msec()-t1.msec());


	// display maps
	for(int i=0; i < n_map; ++i)
	{
		ACE_OS::printf("maps[%d].size()=%d\n", i, (maps[i])->size());
	}

	do
	{
		// dump file and exit
		if ( dump_file.size() )
		{
			SBRS_MAP& map0 = *(maps[0]);
			ACE_OS::printf("dump size: %d\n", map0.size());

			ACE_HANDLE fh = ACE_OS::open(dump_file.c_str(), O_CREAT | O_TRUNC | O_BINARY | O_WRONLY);
			if ( fh == ACE_INVALID_HANDLE )
			{
				ACE_OS::printf("open '%s' failed!\n", dump_file.c_str());
				break;
			}

			int count = 0;
			ACE_UINT32 mask_int = -1;
			unsigned char ip[4];
			std::string block;
			size_t n_block = 0;
			for(SBRS_MAP::iterator iter = map0.begin(); iter != map0.end(); ++iter)
			{
				ACE_UINT32 ip_int = iter->first;
				unsigned char score = iter->second;
				//ACE_OS::printf("%u=%d\n", ip_int, score);
				if ( score != (unsigned char) '\x00' ) continue;
				//ACE_OS::printf("%u=%d\n", ip_int, score);

				ip[0] = (ip_int >> 24);
				ip[1] = (ip_int >> 16) % 256;
				ip[2] = (ip_int >> 8) % 256;
				ip[3] = ip_int % 256;
				
				if ( mask_int != (ip_int >> 16 ) )
				{
					if ( block.size() > 4 )
					{
						//size_t n_size = (block.size()-4)/2 - 1;
						//ACE_OS::printf("n_block:size=%d:%d\n", n_block-1, n_size);

						n_block -= 1;
						int n_hi = (n_block >> 8) % 256;
						int n_lo = n_block % 256;
						unsigned char* buf = (unsigned char*) block.c_str();
						*(buf+2) = n_hi;
						*(buf+3) = n_lo;

						block.append(2, '\x00');
						// write to file
						ACE_OS::write(fh, (void*) block.c_str(), block.size());
					}

					n_block = 0;
					block = ip[0]; block += ip[1];
					block.append(2, '\x00');
					//block += '\x00'; block += '\x00';

					//ACE_OS::printf("%u=%d\n", ip_int, block.size());
					mask_int = (ip_int >> 16 );
					//++count;
				}
				block += ip[2]; block += ip[3];
				++n_block;
				++count;
			}
			if ( block.size() > 4 )
			{
				//size_t n_size = (block.size()-4)/2 - 1;
				//ACE_OS::printf("n_block:size=%d:%d\n", n_block-1, n_size);

				n_block -= 1;
				int n_hi = (n_block >> 8) % 256;
				int n_lo = n_block % 256;
				unsigned char* buf = (unsigned char*) block.c_str();
				*(buf+2) = n_hi;
				*(buf+3) = n_lo;

				block.append(2, '\x00');
				// write to file
				ACE_OS::write(fh, (void*) block.c_str(), block.size());
			}
			ACE_OS::close(fh);

			ACE_OS::printf("total=%d\n", count);
			break;
		}

		// start local_acceptor
		aos::ipc::Local_Acceptor acceptor; //ACE_Time_Value timeout(1);

		if ( acceptor.open(svc.name()) == -1 )
		{
			ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
			rc = -1;
			break;
		}

		// run server
		io_service ios;
		SBRS_Server server(ios, maps);
		server.timeout(server_timeout);
		server.start(server_port, 0);

		//+ if server.start() fails (e.g bind() fails), break;

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

				if ( n_send > 0 )
				{
					stop = 1;
					// send stop signal, and wait for service to stop
					server.stop();
				}
			}
			else
			{
				n_send = svc.handle_unknown_command(stream);
			}
			stream.close();
		}
		acceptor.close();
	}
	while(0);

	// delete maps
	for(int i=0; i < n_map; ++i)
	{
		delete maps[i];
	}
	maps.clear();

	return rc;
}

int run_server_test(int argc, ACE_TCHAR* argv[])
{
	SBRS_MAPS maps;

	io_service ios;
	SBRS_Server server(ios, maps);
	//server.start(16800);
	server.start(16800, 0, 8);

	while(1)
	{
		ACE_OS::sleep(1);
		ACE_OS::printf("n_conn:%d\n", server.n_connection());
		//break;
	}

	server.stop();

	return 0;
}

int run_loader_test(int argc, ACE_TCHAR* argv[])
{
	int n_map = ACE_OS::num_processors_online();
	if ( n_map < 1 ) n_map = 1;
	if ( n_map > 8 ) n_map = 8;

	//n_map = 1; //@

	// create maps
	SBRS_MAPS maps;
	for(int i=0; i < n_map; ++i)
	{
		maps.push_back(new SBRS_MAP());
	}

	// load maps
	ACE_Time_Value t1 = ACE_OS::gettimeofday();
	SBRS_Loader loader(maps);
	loader.start(n_map);
	ACE_OS::printf("n_map=%d\n", n_map);
	loader.wait();
	ACE_Time_Value t2 = ACE_OS::gettimeofday();
	ACE_OS::printf("loading elasped:%d\n", t2.msec()-t1.msec());


	// display maps
	for(int i=0; i < n_map; ++i)
	{
		ACE_OS::printf("maps[%d].size()=%d\n", i, (maps[i])->size());
	}

	// run server
	io_service ios;
	SBRS_Server server(ios, maps);
	server.start(16800, 0);
	while(1)
	{
		ACE_OS::sleep(1);
		//ACE_OS::printf("n_conn:%d\n", server.n_connection());
		//break;
	}
	server.stop();


	// delete maps
	for(int i=0; i < n_map; ++i)
	{
		delete maps[i];
	}
	maps.clear();

	return 0;
}


//// input maps
//std::ifstream infile("c:/btree.dat");
//maps[0]->restore(infile);
//infile.close();
//
//// output maps
//std::ofstream outfile("c:/btree.dat");
//(maps[0])->dump(outfile);
//outfile.close();


int run_ipv4_string_test(int argc, ACE_TCHAR* argv[])
{
	ACE_UINT32 ip_int;
	std::string ip_str;

	ip_str = "59.124.255.226";
	ip_int = IPv4::to_uint32(ip_str);
	ACE_OS::printf("ip_int:%u\n", ip_int);
	ip_str = IPv4::to_string(ip_int);
	ACE_OS::printf("ip_str:%s\n", ip_str.c_str());

	return 0;
}
//*/



