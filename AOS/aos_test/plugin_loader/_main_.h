
int run_plugin_loader_test(int argc, ACE_TCHAR* argv[])
{
	typedef aos::plugin::Plugin_T< My_Plugin_Interface > P;
	std::vector< P* > dll_array;

	// plugin_dir settings
	char buf[PATH_MAX+1];
	//ACE_OS::realpath("../plugin_dir", buf);
	std::string plugin_dir(ACE_OS::realpath("../plugin_dir", buf));
	std::string suffix = ".so";

#if defined ( WIN32 )
	suffix = ".dll";
#	ifdef _DEBUG
		plugin_dir += "/debug";
#	endif
#endif

	ACE_OS::printf("suffix: %s\n", suffix.c_str());

	// read all plugins
	ACE_Dirent dir;
	ACE_DIRENT* d;
	std::string path;

	dir.open(plugin_dir.c_str());
	while( (d = dir.read()) != 0 )
	{
		path = plugin_dir;
		path += ACE_DIRECTORY_SEPARATOR_CHAR;
		path.append(d->d_name);

		std::string suf(path.substr(path.size()-suffix.size(), suffix.size()));
		ACE_stat stat;
		if ( ACE_OS::lstat(path.c_str(), &stat) != -1
			&& (stat.st_mode & S_IFMT) == S_IFREG
			&& path.size() > suffix.size()
			&& aos::tolower(suf) == suffix ) 
		{
			ACE_OS::printf("file: %s\n", path.c_str());

			// create ACE_DLL and register plugin
			P* dll = new P();
			dll_array.push_back(dll);
			int rc = dll->open(path.c_str());
			ACE_OS::printf("rc: %d\n", rc);
			if ( rc != -1 )
			{
				
				My_Plugin_Interface* ptr = 0;
				std::string str;

				// create first instance
				//My_Plugin_Interface* plugin = dll->create_plugin();

				ptr = dll->create_instance();
				std::auto_ptr< My_Plugin_Interface > plugin(ptr);
				ACE_OS::printf("create_instance(): %p\n", ptr);

				str = "\t  abc cde  \t";
				ACE_OS::printf("str: \"%s\"\n", str.c_str());

				plugin->process(str);
				ACE_OS::printf("str: \"%s\"\n", str.c_str());


				// create second instance
				ptr = dll->create_instance();
				std::auto_ptr< My_Plugin_Interface > plugin2(ptr);
				ACE_OS::printf("create_instance(): %p\n", ptr);

				str = "\t  xyz cde  \t";
				ACE_OS::printf("str: \"%s\"\n", str.c_str());

				plugin2->process(str);
				ACE_OS::printf("str: \"%s\"\n", str.c_str());
			}
			else
			{
				ACE_OS::printf("dll open() FAILED!\n");
			}
			
		} // if()
	}
	dir.close();
	ACE_OS::printf("------\n");

	// cleanup ACE_DLL
	::printf("delete dll_array\n");
	for(size_t i=0; i < dll_array.size(); ++i)
	{
		P* ptr = dll_array.at(i);
		delete ptr;
	}
	dll_array.clear();

	return 0;
}

/*
int run_plugin_loader_test(int argc, ACE_TCHAR* argv[])
{
	std::vector< ACE_DLL* > dll_array;
	My_Plugin_Manager* pm = new My_Plugin_Manager();

	// plugin_dir settings
	char buf[PATH_MAX+1];
	//ACE_OS::realpath("../plugin_dir", buf);
	std::string plugin_dir(ACE_OS::realpath("../plugin_dir", buf));
	std::string suffix = ".so";

#if defined ( WIN32 )
#	ifdef _DEBUG 
		suffix = ".d.dll"; // Debug
#	else
		suffix = ".r.dll"; // Release
#	endif
#endif

	ACE_OS::printf("suffix: %s\n", suffix.c_str());

	// read all plugins
	ACE_Dirent dir;
	ACE_DIRENT* d;
	std::string path;

	dir.open(plugin_dir.c_str());
	while( (d = dir.read()) != 0 )
	{
		path = plugin_dir;
		path += ACE_DIRECTORY_SEPARATOR_CHAR;
		path.append(d->d_name);

		std::string suf(path.substr(path.size()-suffix.size(), suffix.size()));
		ACE_stat stat;
		if ( ACE_OS::lstat(path.c_str(), &stat) != -1
			&& (stat.st_mode & S_IFMT) == S_IFREG
			&& path.size() > suffix.size()
			&& aos::tolower(suf) == suffix ) 
		{
			ACE_OS::printf("file: %s\n", path.c_str());

			ACE_SHLIB_HANDLE lh = ACE_OS::dlopen(path.c_str());
			ACE_OS::printf("dlerror: %s\n", ACE_OS::dlerror());
			ACE_OS::printf("lh: %p\n", lh);

			// create ACE_DLL and register plugin
			ACE_DLL* dll = new ACE_DLL();
			dll_array.push_back(dll);
			int rc = dll->open(path.c_str());
			ACE_OS::printf("rc: %d\n", rc);
			if ( rc != -1 )
			{
				typedef void fnRegister(My_Plugin_Manager&);

				fnRegister* pfn_register = (fnRegister*) dll->symbol("register_plugin");
				if ( pfn_register )
				{
					pfn_register(*pm);
					ACE_OS::printf("register OK!\n");
				}
				else
				{
					ACE_OS::printf("register FAILED!\n");
				}

				typedef const char* fnEngVer();
				fnEngVer* pfn_engine_version = (fnEngVer*) dll->symbol("engine_version");
				if ( pfn_engine_version )
				{
					ACE_OS::printf("engine:%s\n", pfn_engine_version());
				}
			}
			else
			{
				ACE_OS::printf("dll open() FAILED!\n");
			}
			
		} // if()
	}
	dir.close();
	ACE_OS::printf("------\n");

	// use plugins
	std::string str("\t  abc cde  \t");
	ACE_OS::printf("str: \"%s\"\n", str.c_str());

	for(size_t i=0; i < pm->size(); ++i)
	{
		ACE_OS::printf("--- beg (%d) ---\n", i);

		My_Plugin_Interface* plugin;
		plugin = pm->at(i);

		ACE_OS::printf("pointer: %p\n", plugin);

		ACE_OS::printf("name: %s\n", plugin->plugin_name());
		ACE_OS::printf("ver: %s\n", plugin->plugin_version());
 		
		plugin->process(str);
		ACE_OS::printf("str: \"%s\"\n", str.c_str());

		// clone test
		My_Plugin_Interface* clone;
		std::string str2("\t  aaa aaa  \t");
		clone = plugin->clone();
		clone->process(str2);
		ACE_OS::printf("str2: \"%s\"\n", str2.c_str());
		delete clone;

		ACE_OS::printf("--- end (%d) ---\n", i);
	}

	// cleanup My_Plugin_Manager
	delete pm;

	// cleanup ACE_DLL
	::printf("delete dll_array\n");
	for(size_t i=0; i < dll_array.size(); ++i)
	{
		ACE_DLL* ptr = dll_array.at(i);
		delete ptr;
	}
	dll_array.clear();

	return 0;
}
//*/

int run_plugin_loader_test2(int argc, ACE_TCHAR* argv[])
{
	std::string str = "\t\t\t\t\tHello World!\t\t\t\t";
	ACE_OS::printf("(%d)%s\n", str.size(), str.c_str());

	int rc = -1;
	ACE_DLL dll;
#if defined ( _DEBUG )
	rc = dll.open("d:/angus/AOS/home/plugin_dir/plugin_1.d.dll"); // Debug
#else
	rc = dll.open("d:/angus/AOS/home/plugin_dir/plugin_1.r.dll"); // Release
#endif

	// typedef pointer to function
	typedef std::string& (*PF_Trim)(std::string&, const char*);
	PF_Trim pf_trim = (PF_Trim) dll.symbol("trim");
	//PF_Trim pf_trim = (PF_Trim) dll.symbol("ltrim");
	//PF_Trim pf_trim = (PF_Trim) dll.symbol("rtrim");

	if ( pf_trim )
	{
		str = pf_trim(str, "\t");
		ACE_OS::printf("(%d)%s\n", str.size(), str.c_str());
	}
	else
	{
		ACE_OS::printf("GetProcAddress() or dlsym() failed!\n");
	}

	return 0;
}

int run_plugin_loader_test3(int argc, ACE_TCHAR* argv[])
{
	ACE_DLL dll;
	int rc = dll.open("d:/angus/AOS/aos_lib/AOSd.dll"); // Debug
	AOS_UNUSE(rc);

	// typedef pointer to function
	typedef const char* fnFoo(void);
	//typedef const char* (*pfnFoo)(void);
	//fnFoo* pf_func = (fnFoo*) dll.symbol("version");
	//fnFoo* pf_func = (fnFoo*) dll.symbol("build_timestamp");
	fnFoo* pf_func = (fnFoo*) dll.symbol("sqlite3_libversion");

	if ( pf_func )
	{
		const char* ver = pf_func();
		ACE_OS::printf("%s\n", ver);
	}
	else
	{
		ACE_OS::printf("GetProcAddress() or dlsym() failed!\n");
	}

	return 0;
}

