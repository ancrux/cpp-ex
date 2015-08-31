#ifndef _QUOTED_STRING_CONVERTER_H_
#define _QUOTED_STRING_CONVERTER_H_

#include <string>

namespace aos {

//+ class Quoted_String_Converter:
// to handle c-style string with quote and escape character for both import/export/validation
// e.g. "abc\"'\xFF\r\n\t"
class Quoted_String_Converter
{
public:
	enum // convert flag
	{
		ESCAPE_LF = 0x01, // '\n'
		ESCAPE_CR = 0x02, // '\r'
		ESCAPE_TAB = 0x04, // '\t'
		ESCAPE_HEX = 0x08, // '\xFF'
		ESCAPE_OCT = 0x10 // '\012'
	};

	enum // convert result
	{
		CONVERT_OK = 0,
		CONVERT_CHAR,
		CONVERT_QUOTE,
		CONVERT_ESCAPE
	};

public:
	Quoted_String_Converter();
	~Quoted_String_Converter();

public:
	// return string buffer and cause convert result to be invalid
	std::string& string() { return buffer(); }; // alias of buffer()
	std::string& buffer()
	{ 
		result_ = CONVERT_CHAR;
		if ( ustr_ ) return *ustr_;
		return str_;
	};
	// supply your own buffer instead of using internal buffer
	void attach_buffer(std::string& str) { ustr_ = &str; };
	void detach_buffer() { ustr_ = 0; };
	int result() const { return result_; };

public:
	// if return 0, convert fails, else return converted const char*
	// use string() to get last converted string whether it's ok or not
	const char* quoted_to_str(const char* cstr, char quote = '"', char escape = '\\');
	const char* quoted_to_str(const char* buf, size_t len, char quote = '"', char escape = '\\');
	const char* quoted_to_str(std::string& str, char quote = '"', char escape = '\\')
	{
		return quoted_to_str(str.c_str(), str.size(), quote, escape);
	};

public:
	// if return 0, convert fails, else return converted const char*
	// use string() to get last converted string whether it's ok or not
	const char* str_to_quoted(const char* cstr, char quote = '"', char escape = '\\');
	const char* str_to_quoted(const char* buf, size_t len, char quote = '"', char escape = '\\');
	const char* str_to_quoted(std::string& str, char quote = '"', char escape = '\\')
	{
		return str_to_quoted(str.c_str(), str.size(), quote, escape);
	};

protected:
	std::string str_; // internal string buffer
	int conv_; // convert flags
	int result_; // convert result
	std::string* ustr_; // user string buffer
};

} // namespace aos

#endif

