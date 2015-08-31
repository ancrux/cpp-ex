#define USE_SSL 0

int run_smtp_client_io(int argc, ACE_TCHAR* argv[])
{
	// SSL Context must be created before ACE_SSL_SOCK_Acceptor to take effect
	ACE_SSL_Context* context = ACE_SSL_Context::instance();
	context->set_mode(ACE_SSL_Context::SSLv23);
	context->certificate("dummy.pem", SSL_FILETYPE_PEM);
	context->private_key("key.pem", SSL_FILETYPE_PEM);

	int port = 25;
#if USE_SSL == 1
	port = 465; // 465 or 587
	ACE_SSL_SOCK_Connector connector; // SSL
	aos::SSL_Socket_Stream stream; //ACE_SSL_SOCK_Stream stream; // SSL
#else
	port = 25;
	ACE_SOCK_Connector connector;
	aos::Socket_Stream stream; //ACE_SOCK_Stream stream;
#endif
	//ACE_INET_Addr server_addr(port, ACE_LOCALHOST);
	ACE_INET_Addr server_addr(port, "192.168.1.19");

	ACE_Time_Value timeout(3);
	int flags = 0;

	if ( connector.connect(stream, server_addr, &timeout) == -1 )
	{
		ACE_OS::printf("connect() failed!\n");
		return -1;
	}

	aos::SOCK_Stream* s = &stream; // stream pointer
	ACE_Message_Block mb(4096); // read/write buffer

	SMTP_Client_IO io;
	io.open();

	ssize_t n_recv = -1; //@
	ssize_t n_send = -1; //@

	// greetings
	if ( io.greetings(*s, flags, &timeout, mb) > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// helo/ehlo
	if ( io.ehlo(*s, flags, &timeout, mb, "localhost") > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// starttls
	if ( io.starttls(*s, flags, &timeout, mb) > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	aos::SSL_Socket_Stream ssl_stream;
	stream.start_ssl(ssl_stream);
	s = &ssl_stream; // switch stream pointer

	// helo/ehlo
	if ( io.ehlo(*s, flags, &timeout, mb, "localhost") > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	//// auth
	//if ( io.auth(*s, flags, &timeout, mb, "LOGIN") > 0 )
	//	ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// auth
	if ( io.auth_login(*s, flags, &timeout, mb, "angus", "111111") > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	//// mail from
	//if ( io.mail_from(*s, flags, &timeout, mb, "kk@email-home.com") > 0 )
	//	ACE_OS::printf("S:%s", io.buf().c_str()); //@

	//// rcpt to
	//if ( io.rcpt_to(*s, flags, &timeout, mb, "angus@zcsdemo.cellopoint.com") > 0 )
	//	ACE_OS::printf("S:%s", io.buf().c_str()); //@

	//// data
	//if ( io.data(*s, flags, &timeout, mb) > 0 )
	//	ACE_OS::printf("S:%s", io.buf().c_str()); //@

	//// write data
	//io.write_data(*s, flags, &timeout, "body", 4);

	//// data completed
	//if ( io.data_completed(*s, flags, &timeout, mb) > 0 )
	//	ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// rset
	if ( io.rset(*s, flags, &timeout, mb) > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// noop
	if ( io.noop(*s, flags, &timeout, mb) > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	// quit
	if ( io.quit(*s, flags, &timeout, mb) > 0 )
		ACE_OS::printf("S:%s", io.buf().c_str()); //@

	io.close();
	stream.close();

	ACE_OS::printf("client stopped!\n");

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

