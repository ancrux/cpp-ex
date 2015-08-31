// Begin of the file
#ifndef _CODEC_H_
#define _CODEC_H_

#include "aos/Config.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>

#include "aos/String.h"

namespace aos {

// Binary To Text Encoder/Decoder: 
// Base64,
// QuotedPrintable,
// UUEncode/UUDecode (Unix-Unix)

class AOS_API Base64
{
public:
	static const size_t DEFAULT_LINE_SIZE = 76;

public:
	static const char enc_tbl[];
	static const char dec_tbl[];

public:
	Base64(size_t line_size = DEFAULT_LINE_SIZE)
	{
		this->line_size(line_size);
	};
	~Base64()
	{
	};
	
public:
	size_t line_size() const { return line_size_; };
	void line_size(size_t line_size)
	{
		line_size_ = line_size;
	};

public: // easy encode/decode API
	static size_t encode(std::string& str, size_t line_size = 0);
	static size_t decode(std::string& str);
	
public:
	size_t encode(const char* in, size_t in_size, char* out);
	size_t decode(const char* in, size_t in_size, char* out);

	inline size_t enough_encode_size(size_t size)
	{
		size = ((size / 3) + 1) * 4 + 1; // 1 == '\0';
		if ( line_size_ )
		{
			size_t n_line = (size / line_size_) + 1;
			size += n_line * 2; // 2 == strlen("\r\n");
		}

		return size;
	};
	inline size_t enough_decode_size(size_t size)
	{
		return ((size / 4) + 1) * 3 + 1; // 1 == '\0';
	};
	
protected:
	size_t line_size_;
};

class AOS_API QP
{
public:
	static const size_t DEFAULT_LINE_SIZE = 72;

public:
	QP(size_t line_size = DEFAULT_LINE_SIZE)
	{
		this->line_size(line_size);
		//const char* alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		const char* hex = "0123456789ABCDEFabcdef";
		//while( *alphanum )
		//{
		//	cm_alphanum_ |= *alphanum;
		//	++alphanum;
		//}
		while( *hex )
		{
			cm_hex_ |= *hex;
			++hex;
		}
	};
	~QP()
	{
	};
	
public:
	size_t line_size() const { return line_size_; };
	void line_size(size_t line_size)
	{
		line_size_ = line_size;
	};

public: // easy encode/decode API
	static size_t encode(std::string& str, size_t line_size = 0);
	static size_t decode(std::string& str);
	
public:
	size_t encode(const char* in, size_t in_size, char* out);
	size_t decode(const char* in, size_t in_size, char* out);

	inline size_t enough_encode_size(size_t size)
	{
		size = (size * 3) + 1; // 1 == '\0';
		if ( line_size_ )
		{
			size_t n_line = (size / line_size_) + 1;
			size += n_line * 3; // 3 == strlen("=\r\n"); soft line break
		}

		return size;
	};
	inline size_t enough_decode_size(size_t size)
	{
		return size + 1;
	};
	
protected:
	size_t line_size_;
	//Char_Map cm_alphanum_;
	Char_Map cm_hex_;
};

class AOS_API UU
{
public:
	static const size_t DEFAULT_LINE_SIZE = 60;

public:
	UU(size_t line_size = DEFAULT_LINE_SIZE)
	{
		this->line_size(line_size);
	};
	~UU()
	{
	};
	
public:
	size_t line_size() const { return line_size_; };
	void line_size(size_t line_size)
	{
		line_size_ = line_size;
		if ( line_size_ < 15 ) line_size_ = 15;
		if ( line_size_ > 75 ) line_size_ = 75;
	};

public: // easy encode/decode API
	static size_t encode(std::string& str, size_t line_size = 0);
	static size_t decode(std::string& str);
	
public:
	size_t encode(const char* in, size_t in_size, char* out);
	size_t decode(const char* in, size_t in_size, char* out);

	inline size_t enough_encode_size(size_t size)
	{
		size = ((size / 3) + 1) * 4 + 1; // 1 == '\0';
		size_t n_line = (size / line_size_) + 1;
		size += n_line * 3; // 3 = 2 + 1; 2 == strlen("\r\n"); 1 == 'M' in the beginning of line

		return size;
	};
	inline size_t enough_decode_size(size_t size)
	{
		return ((size / 4) + 1) * 3 + 1; // 1 == '\0';
	};

	inline char enc(char c)
	{
		return ((c) ? ((c) & 077) + ' ': '`');
	};
	inline char dec(char c)
	{
		return (((c) - ' ') & 077);
	};
	
protected:
	size_t line_size_;
};

} // namespace aos

#endif //_CODEC_H_
