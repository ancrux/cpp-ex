
int file_var_replace(std::map< std::string, std::string >& vars, const char* file_in, const char* file_out)
{
	int rc = -1;

	// read template
	ACE_HANDLE fh_in = ACE_OS::open(file_in, O_BINARY | O_RDONLY);
	if ( fh_in == ACE_INVALID_HANDLE )
		return rc;

	ACE_OFF_T fsize_in = ACE_OS::filesize(fh_in);
	std::string buf; buf.resize(fsize_in);

	ssize_t n_read = ACE_OS::read(fh_in, (char*) buf.c_str(), buf.size());
	ACE_OS::close(fh_in);
	if ( n_read != (ssize_t) buf.size() )
		return rc;

	// replace template
	aos::var_replace(buf, vars);

	// write template
	ACE_HANDLE fh_out = ACE_OS::open(file_out, O_BINARY | O_WRONLY | O_CREAT | O_TRUNC);
	if ( fh_out == ACE_INVALID_HANDLE )
		return rc;

	ssize_t n_write = ACE_OS::write(fh_out, buf.c_str(), buf.size());
	ACE_OS::close(fh_out);
	if ( n_write != (ssize_t) buf.size() )
		return rc;

	return 0;
}


int setup(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	// variables holder
	std::map< std::string, std::string > vars;

	// find app root directory and chdir() to it
	std::string root; // app root
	char path[PATH_MAX+1];
	root = ACE::dirname(ACE_OS::realpath(argv[0], path));
	ACE_OS::chdir(root.c_str());

	// set ROOT
	vars.insert(std::make_pair("ROOT", root));
	//ACE_OS::printf("root: %s\n", root.c_str()); //@

	// import setup.ini
	std::string ini_file("./setup.ini");

	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	iniIO.import_config(ini_file.c_str());

	ACE_Configuration_Section_Key sec;
	ACE_TString key;
	ACE_TString val;
	ACE_Configuration::VALUETYPE val_type;

	// read [@] variables section
	config.open_section(config.root_section(), "@", 0, sec);
	for(int i=0; config.enumerate_values(sec, i, key, val_type) == 0; ++i)
	{
		config.get_string_value(sec, key.c_str(), val);
		vars.insert(std::make_pair(key.c_str(), val.c_str()));
		//ACE_OS::printf("[@]%s=%s\n", key.c_str(), val.c_str()); //@
	}

//#ifdef ACE_WIN32
	///* compile & run setup.js here
	std::string js_file("./setup.js");

	// Create a new context. 
	Persistent<Context> context = Context::New();
	// Enter the created context for compiling and 
	// running the hello world script.
	Context::Scope context_scope(context);

	// Create a stack-allocated handle scope. 
	HandleScope handle_scope;

	// Create object template for map 
	Handle<ObjectTemplate> map_tpl = ObjectTemplate::New();
	// Set one internal field for c++ object reference
	map_tpl->SetInternalFieldCount(1);
	// Set interceptors
	map_tpl->SetNamedPropertyHandler(V8_Map::getter, V8_Map::setter);

	// Create an empty map wrapper.
	Handle<Object> obj = map_tpl->NewInstance();
	// Wrap the raw C++ pointer in an External so it can be referenced
	// from within JavaScript.
	obj->SetInternalField(0, External::New(&vars));

	// Set the options object as a property on the global object.
	context->Global()->Set(String::New("VARS"), obj);

	// Create a string containing the JavaScript source code.
	Handle<String> source = V8_Util::read_file(js_file);

	// Compile the source code.
	Handle<Script> script = Script::Compile(source);
	// Run the script to get the result.
	Handle<Value> result = script->Run();

	// Convert the result to an ASCII string and print it.
	String::AsciiValue ascii(result);
	ACE_OS::printf("script: %s\n", *ascii); //@
	V8_Map::dump(&vars); //@

	// Dispose the persistent context.
	context.Dispose();
	//*/
//#endif //ACE_WIN32

	// enum [Template] section for variables replacement 
	config.open_section(config.root_section(), "Template", 0, sec);
	for(int i=0; config.enumerate_values(sec, i, key, val_type) == 0; ++i)
	{
		config.get_string_value(sec, key.c_str(), val);
		if ( ACE_OS::atoi(val.c_str()) )
		{
			ACE_TString tpl(key); tpl += ".tpl";
			rc = file_var_replace(vars, tpl.c_str(), key.c_str());
			//ACE_OS::printf("rc=%d\n", rc); //@
		}
		//ACE_OS::printf("[Template]%s=%s\n", key.c_str(), val.c_str()); //@
	}

	//// lighttpd document root
	//rc = file_var_replace(vars,"./lighttpd/lighttpd-root.conf.tpl", "./lighttpd/lighttpd-root.conf");
	////ACE_OS::printf("rc=%d\n", rc); //@

	return rc;
}
