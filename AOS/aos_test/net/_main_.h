
int run_imap4_client_session(int argc, ACE_TCHAR* argv[])
{
	//ACE_SSL_Context* context = ACE_SSL_Context::instance();
	//// Note - the next two strings are naked on purpose... the arguments to
	//// the ACE_SSL_Context methods are const char *, not ACE_TCHAR *.
	//context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	//context->private_key("key.pem", SSL_FILETYPE_PEM);

	AIO_Config cfg;
	cfg.aio_type(POLL);
	cfg.n_thread(8);
	cfg.n_demultiplexor(2);
	Proactor_TaskPool task(cfg);

	// setup connector
	AIO_Connector_T< IMAP4_SSL_Client_Session > connector(task);
	ACE_Time_Value timeout_c(10);
	connector.set_timeout(timeout_c);
	connector.max_session(2048);

	
	task.start();
	task.enable_event_loop(); //? after acceptor.start()
	

	/* client start */
	// BUG HERE!
	// Sometimes, client connector/proactor terminates
	// before session comes alive. so session constructor is called
	// but not being freed (destructor not called)
	ACE_INET_Addr connect_addr(993, "zcsdemo.cellopoint.com");
	//ACE_INET_Addr connect_addr(143, "zcsdemo.cellopoint.com");
	connector.start();
	for(int i = 0; i < 1; ++i)
	{
		ACE_OS::write_n(ACE_STDOUT, "@", 1);
		connector.connect(connect_addr, 1);
		while( !connector.is_safe_to_delete()  )
		{
			ACE_OS::write_n(ACE_STDOUT, "+", 1);
			task.wait_signal();
		}
		//ACE_Time_Value tv(0, 1 * 1000);
		//timespec_t t  = (timespec_t) tv;
		//ACE_OS::nanosleep(&t);
	}

	// Method 1: wait for connection done
	while( !connector.is_safe_to_delete() || connector.n_session() > 0 )
	{
		ACE_OS::write_n(ACE_STDOUT, "-", 1);
		ACE_Time_Value tv(0, 1 * 1000);
		timespec_t t  = (timespec_t) tv;
		ACE_OS::nanosleep(&t);
	}

	//// Method 2: wait for connection done
	//while( !connector.is_safe_to_delete()  )
	//{
	//	task.wait_signal();
	//}
	//while( connector.n_session() > 0 )
	//{
	//	ACE_Time_Value tv(0, 1 * 1000);
	//	timespec_t t  = (timespec_t) tv;
	//	ACE_OS::nanosleep(&t);
	//}

	/* client stop */
	//// cancel all connection
	//connector.cancel_all_session();
	//for(;;) if ( connector.n_session() == 0 ) break;
	//// cancel connector
	//connector.cancel();
	//// wait for all connection to close
	//for(;;) if ( connector.is_safe_to_delete() ) break;
	////wait for connector to close
	//while( !connector.is_safe_to_delete() )
	//{
	//	task.wait_signal();
	//}

	task.stop();

	::printf("end\r\n");
	getchar();

	return 0;
}

int run_smtp_test(int argc, ACE_TCHAR* argv[])
{
	//ACE_SSL_Context* context = ACE_SSL_Context::instance();
	//// Note - the next two strings are naked on purpose... the arguments to
	//// the ACE_SSL_Context methods are const char *, not ACE_TCHAR *.
	//context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	//context->private_key("key.pem", SSL_FILETYPE_PEM);

	AIO_Config cfg;
	cfg.aio_type(POLL);
	cfg.n_thread(8);
	cfg.n_demultiplexor(2);
	Proactor_TaskPool task(cfg);

	// setup connector
	AIO_Connector_T< SMTP_SSL_Client_Session > connector(task);
	ACE_Time_Value timeout_c(10);
	connector.set_timeout(timeout_c);
	connector.max_session(2048);

	
	task.start();
	task.enable_event_loop(); //? after acceptor.start()
	

	/* client start */
	ACE_INET_Addr connect_addr(25, "zcsdemo.cellopoint.com");
	//ACE_INET_Addr connect_addr(25, "192.168.1.33");
	connector.start();
	for(int i = 0; i < 100; ++i)
	{
		connector.connect(connect_addr, 10);
		while( !connector.is_safe_to_delete()  )
		{
			task.wait_signal();
		}
		//ACE_Time_Value tv(0, 1 * 1000);
		//timespec_t t  = (timespec_t) tv;
		//ACE_OS::nanosleep(&t);
	}

	// Method 1: wait for connection done
	while( !connector.is_safe_to_delete() || connector.n_session() > 0 )
	{
		ACE_Time_Value tv(0, 1 * 1000);
		timespec_t t  = (timespec_t) tv;
		ACE_OS::nanosleep(&t);
	}

	//// Method 2: wait for connection done
	//while( !connector.is_safe_to_delete()  )
	//{
	//	task.wait_signal();
	//}
	//while( connector.n_session() > 0 )
	//{
	//	ACE_Time_Value tv(0, 1 * 1000);
	//	timespec_t t  = (timespec_t) tv;
	//	ACE_OS::nanosleep(&t);
	//}

	/* client stop */
	//// cancel all connection
	//connector.cancel_all_session();
	//for(;;) if ( connector.n_session() == 0 ) break;
	//// cancel connector
	//connector.cancel();
	//// wait for all connection to close
	//for(;;) if ( connector.is_safe_to_delete() ) break;
	////wait for connector to close
	//while( !connector.is_safe_to_delete() )
	//{
	//	task.wait_signal();
	//}


	task.stop();

	::printf("end\r\n");
	getchar();

	return 0;
}

int run_http_test(int argc, ACE_TCHAR* argv[])
{
	ACE_SSL_Context* context = ACE_SSL_Context::instance();
	// Note - the next two strings are naked on purpose... the arguments to
	// the ACE_SSL_Context methods are const char *, not ACE_TCHAR *.
	context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	context->private_key("key.pem", SSL_FILETYPE_PEM);

	HTTP::Responser http;

	for(int i=0; i < 1; ++i)
	{
		static const int BUFSIZE = 4096;
		std::string url("/%d.txt");
		url.resize(256);
		ACE_OS::sprintf(&url[0], url.c_str(), i);

		ACE_Message_Block mb(BUFSIZE), out(BUFSIZE);
		int n = 0;
		n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), HTTP::Header::_Request_, "GET", url.c_str(), 1.1); mb.wr_ptr(n);
		n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), HTTP::Header::Connection, "keep-alive"); mb.wr_ptr(n);
		n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), HTTP::Header::_Header_, "key", "value"); mb.wr_ptr(n);
		n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), HTTP::Header::_End_); mb.wr_ptr(n);

		int n_response = http.parse_header(&mb, &out);
		//printf("http_response: %d\r\n", n_response);

		ACE_OS::write(ACE_STDOUT, out.base(), out.length());
	}

	TaskPool::disable_signal(ACE_SIGRTMIN, ACE_SIGRTMAX);
	TaskPool::disable_signal(SIGPIPE, SIGPIPE);
	TaskPool::disable_signal(SIGIO, SIGIO);

	//ACE_LOG_MSG->priority_mask(LM_ERROR, ACE_Log_Msg::PROCESS);

	AIO_Config cfg;
	cfg.aio_type(POLL);
	cfg.n_thread(8);
	cfg.n_demultiplexor(2);
	Proactor_TaskPool task(cfg);

	// setup acceptor
	AIO_Acceptor_T< HTTP_SSL_Server_Session > acceptor(task); //SSL
	//AIO_Acceptor_T< HTTP_Server_Session > acceptor(task); // non-SSL
	ACE_Time_Value timeout_a(5);
	acceptor.set_timeout(timeout_a);
	acceptor.max_session(2048);

	// setup connector
	AIO_Connector_T< HTTP_SSL_Client_Session > connector(task);
	ACE_Time_Value timeout_c(10);
	connector.set_timeout(timeout_c);
	connector.max_session(2048);

	task.start();
	task.enable_event_loop(); //? after acceptor.start()

	/* server start */
	ACE_INET_Addr server_addr(443);
	acceptor.start(server_addr);

	/* client start */
	//ACE_INET_Addr connect_addr(80, "mailgw.cellopoint.com");
	ACE_INET_Addr connect_addr(443, "mailgw.cellopoint.com");
	connector.start();
	for(int i = 0; i < 1; ++i)
	{
		connector.connect(connect_addr, 5);
		while( connector.n_session() > 0 )
		{
			ACE_Time_Value tv(0, 1 * 1000);
			timespec_t t  = (timespec_t) tv;
			ACE_OS::nanosleep(&t);
		}
	}


	/* wait for user to stop */
	ACE_OS::sleep(1);
	::printf("done\r\n");
	getchar();

	/* client stop */
	// cancel all connection
	connector.cancel_all_session();
	for(;;) if ( connector.n_session() == 0 ) break;
	// cancel connector
	connector.cancel();
	// wait for all connection to close
	for(;;) if ( connector.is_safe_to_delete() ) break;
	//wait for connector to close
	while( !connector.is_safe_to_delete() )
	{
		task.wait_signal();
	}


	/* server stop */
	// cancel all accept
	acceptor.cancel();
	for(;;) if ( acceptor.is_safe_to_delete() ) break;
	// cancel all connection
	acceptor.cancel_all_session();
	// wait for all connection to close
	for(;;) if ( acceptor.n_session() == 0 ) break;
	// wait for all accept to close
	while( !acceptor.is_safe_to_delete() )
	{
		task.wait_signal();
	}
	

	task.stop();

	::printf("end\r\n");
	getchar();

	return 0;
}
