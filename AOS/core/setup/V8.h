#ifndef _V8_H_
#define _V8_H_

#include "aos/String.h"
using namespace std;

#include <v8.h>
using namespace v8;

class V8_Util
{
public:
	// Convert a JavaScript string to a std::string.  To not bother too
	// much with string encodings we just use ascii.
	static std::string obj_to_str(Local<Value> value);

	// Reads a file into a v8 string.
	static Handle<String> read_file(const string& name);
};

class V8_Map
{
public:
	static Handle<Value> getter(Local<String> name, const AccessorInfo& info);
	static Handle<Value> setter(Local<String> name, Local<Value> value_obj, const AccessorInfo& info);
	static void dump(map<string, string>* m);
};

#endif // _V8_H_
