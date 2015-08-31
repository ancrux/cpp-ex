
int run_smtp_server_io(int argc, ACE_TCHAR* argv[])
{
	// SSL Context must be created before ACE_SSL_SOCK_Acceptor to take effect
	ACE_SSL_Context* context = ACE_SSL_Context::instance();
	context->set_mode(ACE_SSL_Context::SSLv23);
	context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	context->private_key("key.pem", SSL_FILETYPE_PEM);

	int port = 25;
#if USE_SSL == 1
	port = 465; // 465 or 587
#else
	port = 25;
#endif
	ACE_INET_Addr server_addr(port, ACE_LOCALHOST); //ACE_INET_Addr server_addr(port);

	SMTP_Server server;
	if ( server.open(server_addr) == 0 )
	{
		///*
		server.start();

		ACE_OS::printf("press any key to stop...\n");
		getchar();
		server.stop();

		server.thr_mgr()->wait(); // don't use http.wait();
		//*/

		/*
		while(1)
		{
			server.start();

			ACE_OS::sleep(5);
			ACE_OS::printf("disable()...\n");
			server.disable();

			server.thr_mgr()->wait(); // don't use http.wait();
			//ACE_OS::sleep(5);
		}
		//*/
	}
	ACE_OS::printf("server terminated!\n");

	return 0;
}


int run_smtp_io_speed(int argc, ACE_TCHAR* argv[])
{
	std::string data = "POST /abc HTTP/1.1\r\n"
	"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\n"
	"Accept-Language: zh-tw\r\n"
	"Content-Type: application/x-www-form-urlencoded\r\n"
	"UA-CPU: x86\r\n"
	"Accept-Encoding: gzip, deflate\r\n"
	"User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727)\r\n"
	"Host: localhost\r\n"
	"Content-Length: 16\r\n"
	"Connection: Keep-Alive\r\n"
	"Cache-Control: no-cache\r\n"
	"\r\n"
	"key1=&key2=12345\r\n";

/*
	SMTP_Server_IO io;
	io.open();

	for(int i=0; i < 1000000; ++i)
	{
		ACE_Message_Block mb(4096);
		mb.copy(data.c_str(), data.size());
		
		io.read_reset();
		while( io.read_state() < SMTP_Server_IO::RD_OK )
		{
			// process buffer
			if ( mb.length() > 0 )
			{
				if ( io.read_state() != SMTP_Server_IO::RD_DATA )
				{
					aos::bcstr header = io.read_header(mb);
					////ACE_OS::printf("%s", io.buf().c_str());
					//if ( io.read_state() > HTTP_Server_IO::RD_HEADER )
					//{
					//	io.dump_headers();
					//}
				}
				else
				{
					aos::bcstr data = io.read_data(mb);
					//std::string in_data(data.buf, data.len);
					//ACE_OS::printf("%s", in_data.c_str());
				}
			}
		}
		//ACE_OS::printf("\n\n");
	}

	io.close();
//*/

	return 0;
}

/*
int run_imap4_server_io(int argc, ACE_TCHAR* argv[])
{

	ACE_SSL_Context* context = ACE_SSL_Context::instance();
	// Note - the next two strings are naked on purpose... the arguments to
	// the ACE_SSL_Context methods are const char *, not ACE_TCHAR *.
	context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	context->private_key("key.pem", SSL_FILETYPE_PEM);

	ACE_INET_Addr server_addr(143, ACE_LOCALHOST);
	ACE_SOCK_Acceptor acceptor;

	//ACE_INET_Addr server_addr(993, ACE_LOCALHOST); // SSL
	//ACE_SSL_SOCK_Acceptor acceptor; // SSL

	
	if ( acceptor.open(server_addr, 1) == -1 )
		return -1; // cannot bind on the server_addr
	
	while(true)
	{
		ACE_SOCK_Stream stream;
		//ACE_SSL_SOCK_Stream stream; // SSL

		ACE_Time_Value timeout(3);
		int flags = 0;

		ACE_INET_Addr client_addr;
		if ( acceptor.accept(stream, &client_addr, 0, 0) == -1 )
		{
			ACE_OS::printf("accept() == -1\n");
		}
		else
		{
			ACE_TCHAR addr[MAXHOSTNAMELEN];
			client_addr.addr_to_string(addr, MAXHOSTNAMELEN);
			ACE_OS::printf("%s\n", addr);

			ACE_Message_Block mb(4096); // read/write buffer
			IMAP4_Server_IO io;
			io.open();

			ssize_t n_recv = -1;
			ssize_t n_send = -1;

			// server-write
			io.greetings(mb);
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

			while(true)
			{
				// read command
				io.read_reset(mb);
				while( io.read_state() != IMAP4_Server_IO::RD_OK )
				{
					// buffer consumed, read more...
					if ( mb.length() == 0 )
					{
						mb.reset();
						n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
						if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
							break;
						else
							mb.wr_ptr(n_recv);
					}
					// process buffer
					if ( mb.length() > 0 )
					{
						aos::bcstr line = io.read_cmd(mb);
						ACE_OS::printf("%s", io.buf().c_str());
					}
				}
				if ( n_recv < 1 ) break; // read failed, close connection

				// server exec
				int rc = io.exec_cmd(mb);

				// server write
				n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
				if ( n_send < 1 ) break; // write failed, close connection

				if ( rc < 0 ) break; // exec command failed and require close connection
			}

			io.close();
			stream.close();
		}
	}

	return 0;
}

int run_imap4_client_io(int argc, ACE_TCHAR* argv[])
{
	//ACE_INET_Addr server_addr(143, "zcsdemo.cellopoint.com");
	//ACE_SOCK_Connector connector;
	//ACE_SOCK_Stream stream;

	ACE_INET_Addr server_addr(993, "zcsdemo.cellopoint.com"); // SSL
	ACE_SSL_SOCK_Connector connector; // SSL
	ACE_SSL_SOCK_Stream stream; // SSL

	ACE_Time_Value timeout(3);
	int flags = 0;

	ACE_Message_Block mb(4096); // read/write buffer
	if ( connector.connect(stream, server_addr, &timeout) != -1 )
	{
		IMAP4_Client_IO io;
		io.open("A001");

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		// read greetings
		io.read_reset();
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_greeting(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		// login
		if ( io.cmd_login(mb, "ntut", "111111") == IMAP4_Client_IO::WR_OK )
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read login response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		// write command
		if ( io.cmd_capability(mb) == IMAP4_Client_IO::WR_OK &&
			stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout) > 0 )
		{
			// read response
			io.read_reset(mb);
			while( io.read_state() != IMAP4_Client_IO::RD_OK )
			{
				if ( mb.length() == 0 )
				{
					mb.reset();
					n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
					if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
						break;
					if ( n_recv > 0 ) mb.wr_ptr(n_recv);
				}
				if ( mb.length() > 0 )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
			}
		}

		// user-command
		mb.reset();
		mb.copy("A001 SELECT INBOX\r\n");
		mb.wr_ptr(-1); // take '\0' out!
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read command response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}
		
		// write command
		if ( io.cmd_noop(mb) == IMAP4_Client_IO::WR_OK &&
			stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout) > 0 )
		{
			// read response
			io.read_reset(mb);
			while( io.read_state() != IMAP4_Client_IO::RD_OK )
			{
				if ( mb.length() == 0 )
				{
					mb.reset();
					n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
					if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
						break;
					if ( n_recv > 0 ) mb.wr_ptr(n_recv);
				}
				if ( mb.length() > 0 )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
			}
		}

		// user-command
		mb.reset();
		mb.copy("A001 FETCH 1 BODY[]\r\n");
		mb.wr_ptr(-1); // take '\0' out!
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read command response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout); // Non-blocking mode
				//n_recv = stream.recv(mb.wr_ptr(), mb.space()); // Blocking mode
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				if ( io.read_state() != IMAP4_Client_IO::RD_DATA )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
				else
				{
					aos::bcstr data = io.read_data(mb);
					std::string in_data(data.buf, data.len);
					ACE_OS::printf("%s", in_data.c_str());
				}
			}
		}

		// logout
		if ( io.cmd_logout(mb) == IMAP4_Client_IO::WR_OK )
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read logout response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		io.close();
		stream.close();
	}

	return 0;
}
//*/

