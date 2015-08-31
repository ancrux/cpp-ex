#ifndef _MIME_UTIL_H_
#define _MIME_UTIL_H_

#include "aos/Config.h"
#include "aos/String.h"
#include "aos/mime/MIME_Entity.h"

#include "ace/OS.h"

#include "unicode/unistr.h" /* UnicodeString class */

#include <list>

namespace aos {

class AOS_API MIME_Header_Util
{
public:
	typedef std::list< std::pair< std::string, std::string > > Pairs;

public:
	static const size_t DEFAULT_LINE_SIZE = 76;

public:
	static std::string& only_field_value(std::string& str);
	static std::string copy_field_value(const std::string& str);

public:
	enum
	{
		No_Parser= 0,
		Encoded_Parser,
		Address_Parser,
		Attribute_Parser
	};

public:
	MIME_Header_Util(size_t line_size = DEFAULT_LINE_SIZE);
	~MIME_Header_Util();

public:
	Pairs::iterator begin() { return pairs_.begin(); };
	Pairs::iterator end() { return pairs_.end(); };

public:
	void parse_encoded(const std::string& str, int trim_crlf = 1, int map_charset = 1);
	int map_charset_decoded(const char* default_charset = 0);
	//- void parse_encoded2(const std::string& str, int trim_crlf = 1, int map_charset = 0); // using boost:regex, about 40% slower!
	//- void parse_encoded3(const std::string& str, int trim_crlf = 1, int map_charset = 0); // using boost:xpressive, very slow!
	std::string& build_utf8_decoded(std::string& str);
	UnicodeString& build_unicode_decoded(UnicodeString& ustr);
	//+ std::string& build_utf8_encoded(std::string& utf8, const char* charset = "utf-8", char enc = 'B');
	//+ UnicodeString& build_unicode_encoded(UnicodeString& ustr, const char* charset = "utf-8", char enc = 'B');

	void parse_address(const std::string& str);
	//- void parse_address2(const std::string& str); // using boost::regex
	std::string& build_address(std::string& str, const char* charset = "utf-8", char enc = 'B');

	void parse_attribute(const std::string& str);
	std::string& build_attribute(std::string& str, const char* charset = "utf-8", char enc = 'B');

public:
	std::string& trim_quoted_string(std::string& str);
	std::string& trim_quotes(std::string& str, char quote = '"');

public:
	size_t line_size() const { return line_size_; };
	void line_size(size_t line_size)
	{
		line_size_ = line_size;
	};

protected:
	int parser_; // parser type
	Pairs pairs_; // parse result

protected:
	size_t line_size_; // max line size allowed in MIME header
};

class AOS_API MIME_Util
{
public: // unicode converter
	static UnicodeString to_unicode(const std::string& from, const char* from_charset, UErrorCode& status);
	static std::string from_unicode(const UnicodeString& ustr, const char* to_charset, UErrorCode& status);
	static std::string to_utf8(const std::string& from, const char* from_charset, UErrorCode& status);
	static std::string from_utf8(const std::string& utf8, const char* to_charset, UErrorCode& status);
	
//public: // encoder/decoder
//	static size_t to_Base64(std::string& str, size_t line_size = 0);
//	static size_t from_Base64(std::string& str);
//	static size_t to_QP(std::string& str, size_t line_size = 0);
//	static size_t from_QP(std::string& str);

public:
	static void map_charset(std::string& charset, const char* default_charset = 0);
	static std::string guess_charset(const char* buf, size_t size); // guess text charset
	//+ add a bad ratio for validating charset, if mis-matched # > bad_ratio, exit check loop and return false.
	//+ static int check_charset(const char* buf, size_t len, const char* charset, double bad_ratio = 0.1);
	static int check_charset(const char* buf, size_t len, const char* charset); // check a charset's match ratio

public:
	static void decode_body(MIME_Entity& e, int process_child = 1);
	static void encode_body(MIME_Entity& e, int process_child = 1);
	//static void decode_body_ctype_match(MIME_Entity& e, const char* ctype, const char* stype, int decode_child = 1);
	//static void encode_body_ctype_match(MIME_Entity& e, const char* ctype, const char* stype, int encode_child = 1);
	//static void text_to_utf8(MIME_Entity& e);
	//static void text_to_utf16(MIME_Entity& e);

public: // UUEncode functions
	//static void get_uuencode_data(const char* buf, size_t len);

public: // import/export with ACE_HANDLE, return bytes read/write
	static ssize_t import_file_handle(MIME_Entity& e, ACE_HANDLE fh, int flag = MIME_Entity::Flag::ALL, bool no_hidden_header = true);
	static ssize_t export_file_handle(MIME_Entity& e, ACE_HANDLE fh, int flag = MIME_Entity::Flag::ALL, bool no_hidden_header = true);
	static ssize_t import_file(MIME_Entity& e, const char* filename, int flag = MIME_Entity::Flag::ALL, bool no_hidden_header = true);
	static ssize_t export_file(MIME_Entity& e, const char* filename, int flag = MIME_Entity::Flag::ALL, bool no_hidden_header = true);
};

// MIME UUEncode Object Parser
class AOS_API MIME_UU_Util
{
public:

protected:
	aos::Multi_String type_; // if UU type="0666 filename", else "text" or empty string
	aos::Multi_String data_; // if UU data=[binary data], else [text data]
};

// MIME Date
class AOS_API MIME_Date
{
public:
	MIME_Date();
	MIME_Date(const char* cstr);
	MIME_Date(const std::string& str);
	~MIME_Date();
	void reset();

public:
	int from_string(const char* cstr);
	int from_string(const std::string& str) { return from_string(str.c_str()); };
	std::string to_string(const char* format = 0);
	time_t gmt_mktime(int apply_tz = 1);

public:
	void dump()
	{
		ACE_OS::printf("MIME_Date:%d-%d-%d %d:%d:%d (%d) [%d]\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm_gmtoff, tm.tm_wday);
	};
	
public:
	struct tm tm;
	long tm_gmtoff; // e.g. GMT+0800 = -28800 = -8 * 60 * 60
};

} // namespace aos

#endif // _MIME_UTIL_H_
