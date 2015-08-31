// Begin of the file
#ifndef _STRING_H_
#define _STRING_H_

// This file will be included only once by the compiler in a build.
#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "aos/Config.h"

#include <stdio.h> // for ::printf()
#include <string>
#include <vector> // for class Multi_String
#include <map> // for var_replace()

//typedef unsigned char byte;
#ifdef _UNICODE
typedef wchar_t char_t;
typedef std::wstring string_t;
#else
typedef char char_t;
typedef std::string string_t;
#endif

namespace aos {

// struct bcstr: buffered char string
struct bcstr
{
	const char* buf;
	size_t len;
};

// struct bwstr: buffered wchar_t string
struct bwstr
{
	const wchar_t* buf;
	size_t len;
};

// return '\n', if "\r\n" or '\n' is read
// return '\r', if '\r' is read
// else return '\xFF'
AOS_API char get_line_delimiter(const char* buf, size_t len);

// for accurate result, open fp in binary mode!
// return (int) n_read: # of bytes read, n_src decreases automatically
// but the user have to update src manually!
// if a complete line read, line[n_read] == '\0' else '\x01'
// bcstr version of getline() is RECOMMENDED! faster and easier to use
AOS_API bcstr get_line(const char* buf, size_t len, char delimiter = '\n');
AOS_API int get_line(char* buf, int n_buf, const char* src, int n_src, char delimiter = '\n');

// case-insensitive compare
AOS_API int strcasecmp(const char* cs, const char* ct);
AOS_API int strncasecmp(const char* cs, const char* ct, size_t count);
AOS_API inline int stricmp(const char* cs, const char* ct) { return strcasecmp(cs, ct); }; // alias
AOS_API inline int strnicmp(const char* cs, const char* ct, size_t count) { return strncasecmp(cs, ct, count); }; // alias

// nature compare
int nat_cmp_right_(const char* a, const char* b); // internal
int nat_cmp_left_(const char* a, const char* b); // internal
int strnatcmp_(const char* a, const char* b, int fold_case); // internal
AOS_API inline int strnatcmp(const char* a, const char* b) { return strnatcmp_(a, b, 0); };
AOS_API inline int strcasenatcmp(const char* a, const char* b) { return strnatcmp_(a, b, 1); };
AOS_API inline int strinatcmp(const char* a, const char* b) { return strcasenatcmp(a, b); }; // alias

// strnstr
AOS_API const char* strnstr(const char* big, size_t n_big, const char* little, size_t n_little);
AOS_API const char* strncasestr(const char* big, size_t n_big, const char* little, size_t n_little);
AOS_API inline const char* strstr(const char* big, const char* little) { return strnstr(big, ::strlen(big), little, ::strlen(little)); };
AOS_API inline const char* strnstr(const char* big, size_t n_big, const char* little) { return strnstr(big, n_big, little, ::strlen(little)); };
AOS_API inline const char* strcasestr(const char* big, const char* little) { return strncasestr(big, ::strlen(big), little, ::strlen(little)); };
AOS_API inline const char* strncasestr(const char* big, size_t n_big, const char* little) { return strncasestr(big, n_big, little, ::strlen(little)); };

//// class xstring<char> = cstring
//// class xstring<wchar_t> = wstring | ustring
//class cstring
//{
//protected:
//	bcstr str_;
//};
//
//class wstring
//{
//protected:
//	bwstr str_;
//};

class AOS_API QS
{
public:
	static size_t encode(std::string& str, char quote = '"', char escape = '\\')
	{
		QS qs(quote, escape);
		std::string tmp(str);
		str.resize(qs.enough_encode_size(tmp.size()));
		size_t n_encode = qs.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
		str.resize(n_encode);

		return n_encode;
	};
	static size_t decode(std::string& str, char quote = '"', char escape = '\\')
	{
		QS qs(quote, escape);
		size_t n_decode = qs.decode(str.c_str(), str.size(), (char*) str.c_str());
		str.resize(n_decode);

		return n_decode;
	};

public:
	//enum // encode/decode flag
	//{
	//	// or NOT_ESCAPE_LF
	//	ESCAPE_LF = 0x01, // '\n'
	//	ESCAPE_CR = 0x02, // '\r'
	//	ESCAPE_TAB = 0x04, // '\t'
	//	ESCAPE_HEX = 0x08, // '\xFF'
	//	ESCAPE_OCT = 0x10 // '\012'
	//};

	enum // encode state
	{
		ENCODE_OK = 0
	};

	enum // decode state
	{
		DECODE_OK = 0,
		DECODE_CHAR,
		DECODE_QUOTE,
		DECODE_ESCAPE
	};

public:
	QS(char quote = '"', char escape = '\\');
	~QS();

public:
	void reset(char quote = '"', char escape = '\\') 
	{
		quote_ = quote;
		escape_ = escape;
		state_ = 0;
		//flag_ = 0;
		n_read_ = 0;
	};

	char quote() const { return quote_; };
	void quote(char quote) { reset(quote, escape()); };

	char escape() const { return escape_; };
	void escape(char escape) { reset(quote(), escape); };

public:
	size_t encode(const char* in, size_t in_size, char* out);
	size_t decode(const char* in, size_t in_size, char* out); // decode can be in-placed
	//size_t test_decode(const char* in, size_t in_size); //? test decode
	inline size_t enough_encode_size(size_t size)
	{
		return size * 2 + 3; // 2 = '\"' or '\\' and 3 = 2 * quote + '\0'
	};
	inline size_t enough_decode_size(size_t size)
	{
		return size;
	};
	int state() const { return state_; };
	size_t decode_read() const { return n_read_; };

	// null-terminated string version
	size_t encode(const char* in, char* out);
	size_t decode(const char* in, char* out);
	//size_t test_decode(const char* in); //? test decode

protected:
	char quote_;
	char escape_;
	int state_; // encode/decode state
	//int flag_; // encode/decode flag
	size_t n_read_; // decode read
};

class AOS_API QCS
{
public:
	enum // encode/decode flag
	{
		// or NOT_ESCAPE_LF
		ESCAPE_LF = 0x01, // '\n'
		ESCAPE_CR = 0x02, // '\r'
		ESCAPE_TAB = 0x04, // '\t'
		ESCAPE_HEX = 0x08, // '\xFF'
		ESCAPE_OCT = 0x10 // '\012'
	};

	enum // encode state
	{
		ENCODE_OK = 0
	};

	enum // decode state
	{
		DECODE_OK = 0,
		DECODE_CHAR,
		DECODE_QUOTE,
		DECODE_ESCAPE
	};

public:
	QCS(char quote = '"', char escape = '\\');
	~QCS();

public:
	void reset(char quote = '"', char escape = '\\') 
	{
		quote_ = quote;
		escape_ = escape;
		state_ = 0;
		flag_ = 0;
		n_read_ = 0;
	};

	char quote() const { return quote_; };
	void quote(char quote) { reset(quote, escape()); };

	char escape() const { return escape_; };
	void escape(char escape) { reset(quote(), escape); };

public:
	size_t encode(const char* in, size_t in_size, char* out);
	size_t decode(const char* in, size_t in_size, char* out); // decode can be in-placed
	inline size_t enough_encode_size(size_t size)
	{
		return size * 4 + 3; // 4 = '\xFF' and 3 = 2 * quote + '\0'
	};
	inline size_t enough_decode_size(size_t size)
	{
		return size;
	};
	int state() const { return state_; };
	size_t decode_read() const { return n_read_; };

	// null-terminated string version
	size_t encode(const char* in, char* out);
	size_t decode(const char* in, char* out);

protected:
	char quote_;
	char escape_;
	int state_; // encode/decode state
	int flag_; // encode/decode flag
	size_t n_read_; // decode read
};


//+ class Multi_String/String_Array
// use std::vector< size_t > and std::string to store multiple short string separate by '\0'
// e.g. vector(0, 6, 12, 18) string("text1\0text2\0text3\0")
// scenario: use two Multi_Strings to store key/value string pairs and provide fast access
// to each element at the same time.

// Export template class instantiation as well
template class AOS_API std::allocator< size_t >;
template class AOS_API std::vector< size_t >;

class AOS_API Multi_String
{
public:
	Multi_String();
	~Multi_String();

public:
	bool empty() const { return offs_.empty(); };
	size_t size() const { return offs_.size(); };
	size_t count() const { return size(); }; // alias for size()
	size_t capacity(size_t idx) const
	{
		if ( idx < offs_.size() )
		{
			size_t off1 = offs_[idx++];
			size_t off2 = strs_.size();
			if ( idx < offs_.size() ) off2 = offs_[idx];

			return off2-off1;
		}
		return 0;
	};
	size_t size(size_t idx) const
	{
		size_t size = capacity(idx);
		return (size)?(size-1):0; // don't count terminated '\0'
	};
	const char* operator[] (size_t idx) const
	{
		return (idx < offs_.size())?(strs_.c_str() + offs_[idx]):0;
	};

public:
	void push_back(const char* cstr);
	void push_back(const char* buf, size_t len);
	void append_to_last(const char* cstr);
	void append_to_last(const char* buf, size_t len);
	void clear(int resize_only = 1)
	{
		(resize_only)?offs_.resize(0):offs_.clear();
		(resize_only)?strs_.resize(0):strs_.clear();
	};
	void erase(size_t pos);
	void pop_back() { erase(offs_.size()-1); };
	void insert(size_t pos, const char* cstr);
	void insert(size_t pos, const char* buf, size_t len);
	void replace(size_t pos, const char* cstr);
	void replace(size_t pos, const char* buf, size_t len);

public:
	size_t offsets_capacity() const { return offs_.capacity(); }; // *sizeof(size_t)
	size_t strings_capacity() const { return strs_.capacity(); }; // *sizeof(char)
	size_t strings_size() const { return strs_.size(); }; // *sizeof(char)
	void dump(char sep = ',', char quote_begin = '[', char quote_end = ']') const;
	
public:
	//? Multi_String& explode() for convenience of mstr.explode()[0]
	void explode(const char separator, const char* cstr, int start = 0, int size = -1);
	void explode(const char* separator, const char* cstr, int start = 0, int size = -1);
	//+ void explode_first(); // explode by first separator
	//+ void explode_last(); // explode by last separator
	void explode_token(const char* delimiter, const char* cstr, int start = 0, int size = -1); // explode with tokenizer
	//+ void explode_regex();
	std::string implode(const char* glue, int start = 0, int size = -1);

protected:
	// adjust char* pointer offset from item[pos] to the end
	void adjust_offsets(size_t pos, int offset)
	{
		size_t size = offs_.size();
		for(size_t i=pos+1; i < size; ++i)
		{
			offs_[i] += offset;
		}
	};

protected:
	std::vector< size_t > offs_;
	std::string strs_;
};

/// class Char_Map

class AOS_API Char_Map
{
public:
	Char_Map(int set_zero = 1); // default constructor
	Char_Map(const char ch);
	Char_Map(const char* cstr);
	Char_Map(const char* buf, size_t len);
	//Char_Map(const Char_Map& rhs); // copy constructor: compiler will generate this for us

public:
	inline void clear() { ::memset(map_, 0, sizeof(map_)); };
	inline void set(const char ch)
	{ 
		clear();
		const unsigned char* ptr = (const unsigned char*) &ch;
		map_[*ptr >> 3] = (1 << (*ptr & 7));
	};
	inline void set(const char* cstr)
	{
		clear();
		const unsigned char* ptr = (const unsigned char*) cstr;
		while( *ptr )
		{
			(*this) |= *ptr;
			++ptr;
		}
	};
	inline void set(const char* buf, size_t len)
	{
		clear();
		const unsigned char* ptr = (const unsigned char*) buf;
		for(size_t i=0; i < len;++i)
			(*this) |= *(ptr+i);
	};
	inline void invert()
	{
		for(int i=0; i < 32; ++i)
			map_[i] ^= '\xFF';
	};

public:
	inline int operator==(const Char_Map& rhs) const throw()
	{
		return ::memcmp(map_, rhs.map_, sizeof(map_)) == 0;
	};
	inline int operator!=(const Char_Map& rhs) const throw()
	{
		return !( (*this) == rhs );
	};
	inline int has(const unsigned char ch) const throw()
	{
		return map_[ch >> 3] & (1 << (ch & 7));
	};

public:
	//inline Char_Map& operator=(const Char_Map& rhs) // assignment operator: compiler will generate this for us
	//{
	//	::memcpy(map_, rhs.map_, 32);
	//	return (*this);
	//};
	inline Char_Map& operator&=(const Char_Map& rhs) throw()
	{
		for(int i=0; i < 32; ++i)
			map_[i] &= rhs.map_[i];
		return (*this);
	};
	inline Char_Map& operator&=(const unsigned char rhs) throw()
	{
		map_[rhs >> 3] &= (1 << (rhs & 7));
		return (*this);
	};
	inline Char_Map& operator|=(const Char_Map& rhs) throw()
	{
		for(int i=0; i < 32; ++i)
			map_[i] |= rhs.map_[i];
		return (*this);
	};
	inline Char_Map& operator|=(const unsigned char rhs) throw()
	{
		map_[rhs >> 3] |= (1 << (rhs & 7));
		return (*this);
	};
	inline const Char_Map operator&(const Char_Map& rhs) const throw()
	{
		return Char_Map(*this) &= rhs;
	};
	inline const Char_Map operator&(const unsigned char rhs) const throw()
	{
		return Char_Map(*this) &= rhs;
	};
	inline const Char_Map operator|(const Char_Map& rhs) const throw()
	{
		return Char_Map(*this) |= rhs;
	};
	inline const Char_Map operator|(const unsigned char rhs) const throw()
	{
		return Char_Map(*this) |= rhs;
	};

public:
	void dump() const throw();

protected:
	unsigned char map_[32]; // 256-bit byte map
};

// static const trimmable Char_Map
static const aos::Char_Map TRIM_CM(" \t\r\n\0\v", 6);

// ltrim() std:;string & c-string version
AOS_API std::string& ltrim(std::string& str, const aos::Char_Map* cm = 0);
AOS_API inline std::string& ltrim(std::string& str, const char* trim) { return str.erase(0, str.find_first_not_of(trim)); };
AOS_API inline std::string& ltrim(std::string& str, const char trim) { return str.erase(0, str.find_first_not_of(trim)); };
AOS_API char* ltrim(char* cstr, const aos::Char_Map* cm = 0);
AOS_API char* ltrim(char* cstr, const char* trim);
AOS_API char* ltrim(char* cstr, const char trim);

// rtrim() std:;string & c-string version
AOS_API std::string& rtrim(std::string& str, const aos::Char_Map* cm = 0);
AOS_API inline std::string& rtrim(std::string& str, const char* trim) { return str.erase(str.find_last_not_of(trim)+1, std::string::npos); };
AOS_API inline std::string& rtrim(std::string& str, const char trim) { return str.erase(str.find_last_not_of(trim)+1, std::string::npos); };
AOS_API char* rtrim(char* cstr, const aos::Char_Map* cm = 0);
AOS_API char* rtrim(char* cstr, const char* trim);
AOS_API char* rtrim(char* cstr, const char trim);

// trim() std:;string & c-string version
AOS_API inline std::string& trim(std::string& str, const aos::Char_Map* cm = 0) { return ltrim(rtrim(str, cm), cm); };
AOS_API inline std::string& trim(std::string& str, const char* trim) { return ltrim(rtrim(str, trim), trim); };
AOS_API inline std::string& trim(std::string& str, const char trim) { return ltrim(rtrim(str, trim), trim); };
AOS_API inline char* trim(char* cstr, const aos::Char_Map* cm = 0) { return rtrim(ltrim(cstr, cm), cm); };
AOS_API inline char* trim(char* cstr, const char* trim) { return rtrim(ltrim(cstr, trim), trim); };
AOS_API inline char* trim(char* cstr, const char trim) { return rtrim(ltrim(cstr, trim), trim); };

// toupper() std:;string & c-string version
AOS_API std::string& toupper(std::string& str);
AOS_API char* toupper(char* cstr);

// tolower() std:;string & c-string version
AOS_API std::string& tolower(std::string& str);
AOS_API char* tolower(char* cstr);

// var_replace()
// variables must in format like '%var%' or '$var$' depends on the var_symbol '%' or '$' in this case.
AOS_API int var_replace(std::string& str, std::map< std::string, std::string > vars, char var_symbol = '%');
AOS_API int var_replace_n(std::string& str, std::map< std::string, std::string > vars, int n = 1, char var_symbol = '%');

/// class Char_Tokenizer

class AOS_API Char_Tokenizer
{
public:
	enum
	{
		Last = -1, // last token terminated with the end of the string
		End = -2 // reach the end of the string, should stop
	};

public:
	Char_Tokenizer(const char* str = 0);
	Char_Tokenizer(const char* str, size_t size);
	inline void c_str(const char* cstr)
	{
		str(cstr, ::strlen(cstr));
	};
	inline void str(const char* str, size_t size)
	{
		beg_ = end_ = max_ = min_ = str;
		if (str) max_ = str + size;
	};
	inline const char* str_begin() const { return min_; }; // return string beg
	inline const char* str_end() const { return max_; }; // return string end

public:
	void set_separator(const char* sep, int invert = 0);
	inline void set_character(const char* sep) { set_separator(sep, 1); };
	int next(int skip_leading_separator = 1, const char* begin = 0);
	int prev(int skip_leading_separator = 1, const char* begin = 0);

public:
	inline const char* token() { return beg_; }; // note: to be used with size(), not null-terminated
	inline size_t size() { return (end_-beg_); };
	inline const char* token_end() { return end_; }

public:
	inline char* copy_token(char* buf)
	{ 
		return ::strncpy(buf, beg_, this->size());
	};
	inline char* copy_token(char* buf, size_t size)
	{
		return ::strncpy(buf, beg_, size);
	};

protected:
	const char* beg_; // beg of token
	const char* end_; // end of token
	const char* min_; // beg of str
	const char* max_; // end of str
	Char_Map cm_; // 256-bit separator map
};

typedef Char_Tokenizer Tokenizer;

} // namespace aos

/*
#include <algorithm>
#include <cctype>

// string toupper(), tolower()
static inline char string_toupper_functional(char c)
{
    return std::toupper(c);
}

static inline char string_tolower_functional(char c)
{
    return std::tolower(c);
}

static inline void string_upper_inplace(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), string_toupper_functional);
}

static inline void string_lower_inplace(std::string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), string_tolower_functional);
}

static inline std::string string_upper(const std::string &str)
{
    std::string strcopy(str.size(), 0);
	std::transform(str.begin(), str.end(), strcopy.begin(), string_toupper_functional);
    return strcopy;
}

static inline std::string string_lower(const std::string &str)
{
    std::string strcopy(str.size(), 0);
    transform(str.begin(), str.end(), strcopy.begin(), string_tolower_functional);
    return strcopy;
}

// string trim()
static inline void ltrim(std::string &str)
{
    str.erase(0, str.find_first_not_of(' '));
}

static inline void rtrim(std::string &str)
{
    str.erase(str.find_last_not_of(' ') + 1, std::string::npos);
}

static inline std::string ltrim_copy(const std::string &str)
{
    std::string::size_type pos = str.find_first_not_of(' ');
    if (pos == std::string::npos) return std::string();

    return str.substr(pos, std::string::npos);
}

static inline std::string rtrim_copy(const std::string &str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if (pos == std::string::npos) return std::string();

    return str.substr(0, pos + 1);
}

static inline std::string trim_copy(const std::string& str)
{
    std::string::size_type pos1 = str.find_first_not_of(' ');
    if (pos1 == std::string::npos) return std::string();

    std::string::size_type pos2 = str.find_last_not_of(' ');
    if (pos2 == std::string::npos) return std::string();

    return str.substr(pos1 == std::string::npos ? 0 : pos1,
                      pos2 == std::string::npos ? (str.length() - 1) : (pos2 - pos1 + 1));
}

static inline void trim(std::string& str)
{
    std::string::size_type pos = str.find_last_not_of(' ');
    if(pos != std::string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if(pos != std::string::npos) str.erase(0, pos);
    }
    else
        str.erase(str.begin(), str.end());
}
//*/

#endif // _STRING_H_
