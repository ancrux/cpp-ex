
int test_evl(int argc, ACE_TCHAR* argv[])
{
	Envelope v;
	v.open("test.evl");

	return 0;
}

int test_mp(int argc, ACE_TCHAR* argv[])
{
	ACE_Get_Opt cmd(argc, argv, ACE_TEXT(""));

	//if ( cmd.argc() != 2 )
	//{
	//	ACE_OS::printf("Invalid arguments!\r\n");
	//	ACE_OS::exit(1);
	//}

	// print max path
	ACE_OS::printf("max_path:%d\n", MAX_PATH);

	MP_Worker mp;

	mp.activate(THR_NEW_LWP | THR_JOINABLE, 1); //ACE_OS::num_processors_online());

	MIME_Entity e;

	// delete log
	ACE_OS::unlink("d:/_spam_/_log_/utf-8.txt");

	// search for all files in path
	ACE_TCHAR path[MAX_PATH+1];

	// NOTE: turn-off Anti-Virus Software if possible, 
	// they have huge performance impact on file I/Os
	ACE_OS::strncpy(path, "d:/_spam_/2009-08/", MAX_PATH);
	//ACE_OS::strncpy(path, "d:/_spam_edm_/2008-09-22/", MAX_PATH);
	//ACE_OS::strncpy(path, "D:/_send_/temp/", MAX_PATH);
	ACE_OS::strncpy(path, "c:/!tmp/ham_2010-05/2010-05-31/", MAX_PATH);
	size_t n_path = ACE_OS::strlen(path);
	// dirent
	ACE_Dirent dir;
	ACE_DIRENT* d;

	int n_file = 0;

	cout << "ACE_Dirent: " << path << endl;
	cout << "open: " << dir.open(path) << endl;

	while( (d = dir.read()) != 0 )
	{
		ACE_stat stat;
		ACE_OS::strcpy(path + n_path, d->d_name);
		if ( ACE_OS::lstat(path, &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFREG ) 
		{
			if (  1 || n_file >=0 && n_file < 100 )
			{
			///*
			ACE_Message_Block* mb = new ACE_Message_Block(strlen(path)+1);
			mb->copy(path);
			mp.msg_queue()->enqueue_tail(mb);
			//*/
			}

			/*
			//cout << path << endl;
			e.import_file(path);
			//cout << e.get_content_type() << endl;
			//*/

			++n_file;
		}
	}
	dir.close();
	path[n_path] = '\0'; // restore path length
	
	// send stop signal
	for(size_t i = 0, c = mp.thr_count(); i < c; ++i)
		mp.msg_queue()->enqueue_tail(new ACE_Message_Block(""));

	mp.wait();
	//*/
	::printf("total files:%d\n", n_file);

	return 0;
}

int test_mime(int argc, ACE_TCHAR* argv[])
{
	/*
	MIME_Entity e;

	std::string buf;
	buf.resize(102400);

	FILE* file = ::fopen("d:\\_spam_\\2008-05-06_481FFB4E_1.eml", "rb");
	if ( file )
	{
		size_t n_byte = ::fread(&buf[0], sizeof(char), 102400, file);
		buf.resize(n_byte);
		::fclose(file);

		MIME_Reader reader;
		reader.load_header(e, &buf[0], buf.size());

		std::string value = e.header().get_charset();
		::printf("attrib: %s\r\n", value.c_str());
		//e.header()["Content-Type"].set_attribute("x-TypE", "abc", 1);

		e.dump();
	}
	else
		::printf("open failed!\r\n");
	//*/

	return 0;
}

int test_codec(int argc, ACE_TCHAR* argv[])
{
	/*
	char inbuf[1000];
	char outbuf[4000];
	size_t n_byte;

	for(int i=0; i<600; ++i)
	{
		inbuf[i] = '\x00';
	}
	inbuf[i] = '\0';
	printf("%d\n", strlen(txt));
	strcpy(inbuf, txt);
	size_t insize = ::strlen(inbuf);

	Base64 b64;
	QP qp;
	UU uu;

	::printf("guess encode size: %d\n", uu.enough_encode_size(insize));
	n_byte = uu.encode(inbuf, insize, outbuf);
	::printf("%d\n%s\n", n_byte, outbuf);

	size_t outsize = ::strlen(outbuf);

	::printf("guess decode size: %d\n", uu.enough_decode_size(outsize));
	n_byte = uu.decode(outbuf, outsize, inbuf);
	::printf("%d\n%s\n", n_byte, inbuf);
	//*/

	return 0;
}