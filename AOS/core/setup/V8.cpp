#include "V8.h"

std::string
V8_Util::obj_to_str(Local<Value> value)
{
	String::Utf8Value utf8_value(value);
	return std::string(*utf8_value);
}

Handle<String>
V8_Util::read_file(const string& name)
{
	FILE* file = fopen(name.c_str(), "rb");
	//if (file == NULL) return Handle<String>();
	if (file == NULL)
	{
		Handle<String> empty = String::New("", 0);
		return empty;
	}

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

Handle<Value>
V8_Map::getter(Local<String> name, const AccessorInfo& info)
{
	Handle<External> field = Handle<External>::Cast((info.Holder())->GetInternalField(0));
	void* ptr = field->Value();
	map<string, string>* obj = static_cast<map<string, string>*>(ptr);

	// Convert the JavaScript string to a std::string.
	std::string key = V8_Util::obj_to_str(name);

	// Look up the value if it exists using the standard STL ideom.
	map<string, string>::iterator iter = obj->find(key);

	// If the key is not present return an empty handle as signal
	if (iter == obj->end()) return Handle<Value>();

	// Otherwise fetch the value and wrap it in a JavaScript string
	const std::string& value = (*iter).second;

	return String::New(value.c_str(), (int) value.length());
}

Handle<Value>
V8_Map::setter(Local<String> name, Local<Value> value_obj, const AccessorInfo& info)
{
	Handle<External> field = Handle<External>::Cast((info.Holder())->GetInternalField(0));
	void* ptr = field->Value();
	map<string, string>* obj = static_cast<map<string, string>*>(ptr);

	// Convert the key and value to std::strings.
	std::string key = V8_Util::obj_to_str(name);
	std::string value = V8_Util::obj_to_str(value_obj);

	// Update the map.
	(*obj)[key] = value;

	// Return the value; any non-empty handle will work.
	return value_obj;
}

void
V8_Map::dump(map<string, string>* m)
{
	for (map<string, string>::iterator i = m->begin(); i != m->end(); i++)
	{
		pair<string, string> entry = *i;
		printf("%s: %s\n", entry.first.c_str(), entry.second.c_str());
	}
}
