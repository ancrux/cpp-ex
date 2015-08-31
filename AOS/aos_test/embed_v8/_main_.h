
// Convert a JavaScript string to a std::string.  To not bother too
// much with string encodings we just use ascii.
std::string
ObjectToString(Local<Value> value)
{
	String::Utf8Value utf8_value(value);
	return std::string(*utf8_value);
}

Handle<Value>
map_getter(Local<String> name, const AccessorInfo& info)
{
	Handle<External> field = Handle<External>::Cast((info.Holder())->GetInternalField(0));
	void* ptr = field->Value();
	map<string, string>* obj = static_cast<map<string, string>*>(ptr);

	// Convert the JavaScript string to a std::string.
	std::string key = ObjectToString(name);

	// Look up the value if it exists using the standard STL ideom.
	map<string, string>::iterator iter = obj->find(key);

	// If the key is not present return an empty handle as signal
	if (iter == obj->end()) return Handle<Value>();

	// Otherwise fetch the value and wrap it in a JavaScript string
	const std::string& value = (*iter).second;

	return String::New(value.c_str(), (int) value.length());
}

static
Handle<Value>
map_setter(Local<String> name, Local<Value> value_obj, const AccessorInfo& info)
{
	Handle<External> field = Handle<External>::Cast((info.Holder())->GetInternalField(0));
	void* ptr = field->Value();
	map<string, string>* obj = static_cast<map<string, string>*>(ptr);

	// Convert the key and value to std::strings.
	std::string key = ObjectToString(name);
	std::string value = ObjectToString(value_obj);

	// Update the map.
	(*obj)[key] = value;

	// Return the value; any non-empty handle will work.
	return value_obj;
}

// Reads a file into a v8 string.
Handle<String>
ReadFile(const string& name)
{
	FILE* file = fopen(name.c_str(), "rb");
	if (file == NULL) return Handle<String>();

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (int i = 0; i < size;) {
		int read = (int) fread(&chars[i], 1, size - i, file);
		i += read;
	}
	fclose(file);
	Handle<String> result = String::New(chars, size);
	delete[] chars;
	return result;
}

void
PrintMap(map<string, string>* m)
{
	for (map<string, string>::iterator i = m->begin(); i != m->end(); i++)
	{
		pair<string, string> entry = *i;
		printf("%s: %s\n", entry.first.c_str(), entry.second.c_str());
	}
}

int run_v8_test2(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	///* compile & run setup.js here
	//std::string js_file("./setup.js");

	// Create a new context. 
	Persistent<Context> context = Context::New();
	// Enter the created context for compiling and 
	// running the hello world script.
	Context::Scope context_scope(context);

	// Create a stack-allocated handle scope. 
	HandleScope handle_scope;

	Handle<Object> g = context->Global();
	g->Set(String::New("z"), Object::New());

	// Create a string containing the JavaScript source code.
	Handle<String> source = String::New("var x = '123'; var y=23; z.a=-1;");
	Handle<String> source2 = String::New("var y=56;w=168;");
	Handle<String> source3 = ReadFile("web.json"); // ReadFile("utf8.js");
	// make result a JSON object
	// use JSON.parse('["strJSON"]'); won't accept newline and comments 
	//source3 = source3->Concat(String::New("JSON.parse('"), source3);
	//source3 = source3->Concat(source3, String::New("')"));
	// use 'eval(["strJSON"])' or '(["strJSON"])' can accept newline and comments
	source3 = source3->Concat(String::New("("), source3);
	source3 = source3->Concat(source3, String::New(")"));
	// make result a JSON string
	source3 = source3->Concat(String::New("JSON.stringify("), source3);
	source3 = source3->Concat(source3, String::New(")"));

	// Compile the source code.
	Handle<Script> script = Script::Compile(source);
	Handle<Script> script2 = Script::Compile(source2);
	Handle<Script> script3 = Script::Compile(source3);

	// Run the script to get the result.
	Handle<Value> result = script->Run();
	Handle<Value> result2 = script2->Run();
	Handle<Value> result3 = script3->Run();

	
	ACE_OS::printf("Global() is: %d\n", g->IsObject());
	Handle<Array> keys = g->GetPropertyNames();
	for(size_t i=0; i < keys->Length(); ++i)
	{
		//Handle<Value> v = g->Get(keys->Get(i));

		Handle<Value> k = keys->Get(i);
		String::Utf8Value k_str(k);

		Handle<Value> v = g->Get(k);
		String::Utf8Value v_str(v);

		ACE_OS::printf("(%d) v['%s'].toString(): %s\n", i, *k_str, *v_str);
	}

	
	Handle<Value> x = context->Global()->Get(String::New("x"));
	String::AsciiValue x_str(x);
	ACE_OS::printf("x is: %d\n", x->IsString());
	ACE_OS::printf("x.toString(): %s\n", *x_str);

	// Convert the result to an ASCII string and print it.
	String::AsciiValue ascii(result3);
	ACE_OS::printf("script: %s\n", *ascii); //@

	// Dispose the persistent context.
	context.Dispose();
	//*/

	return rc;
}

int run_v8_test1(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	// variables holder
	std::map< std::string, std::string > vars;
	vars.insert(std::make_pair("key", "value"));

	///*
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
	map_tpl->SetNamedPropertyHandler(::map_getter, ::map_setter);

	// Create an empty map wrapper.
	Handle<Object> obj = map_tpl->NewInstance();
	// Wrap the raw C++ pointer in an External so it can be referenced
	// from within JavaScript.
	obj->SetInternalField(0, External::New(&vars));

	// Set the options object as a property on the global object.
	context->Global()->Set(String::New("VARS"), obj);

	//for(int i=0; i<100000;++i)
	//{

	// Create a string containing the JavaScript source code.
	//Handle<String> source = String::New("'Hello' + ', World!'");
	Handle<String> source = ReadFile("_main_.js");

	// Compile the source code.
	Handle<Script> script = Script::Compile(source, v8::String::New("(shell)"));
	// Run the script to get the result.
	Handle<Value> result = script->Run();

	// Read variable in javascript, Object, RegExp, String, Array, Date, etc are native objects
	//Handle<Object> jsCtx = context->Global();
	Handle<Value> jsVal = context->Global()->Get(String::New("obj"));
	Handle<Array> jsValProp = jsVal->ToObject()->GetPropertyNames();
	for(uint32_t i = 0; i < jsValProp->Length(); ++i)
	{
		Handle<String> propKey = jsValProp->Get(Uint32::New(i))->ToString();
		String::Utf8Value utf8_key(propKey);
		printf("%s:", *utf8_key);
		Handle<Value> propVal = jsVal->ToObject()->Get(propKey);
		String::Utf8Value utf8_val(propVal);
		printf("%s\n", *utf8_val);
	}
	String::AsciiValue a_val(jsValProp);
	printf("%s\n", *a_val);

	// Convert the result to an ASCII string and print it.
	String::AsciiValue ascii_str(result);
	printf("%s\n", *ascii_str);

	//}

	// Dispose the persistent context.
	context.Dispose();
	//*/

	::PrintMap(&vars);

	return rc;
}

