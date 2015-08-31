#include "Quoted_String_Converter.h"

#include <cassert>

namespace aos {

/// class Quoted_String_Converter

/*
Escape 	Meaning
\\	0x5C Literal backslash
\"	0x22 Double quote
\'	0x27 Single quote
\n 	0x0A Newline (line feed)
\r 	0x0C Carriage return
\b 	0x08 Backspace
\t 	0x09 Horizontal tab
\f 	0x0C Form feed
\a 	0x07 Alert (bell)
\v 	0x0B Vertical tab
\? 	0x3F Question mark (used to escape trigraphs)
\nnn 	Character with octal value nnn
\xhh 	Character with hexadecimal value hh

should check for
n, r, b, t, f, a, v, x, 0-9
//*/

Quoted_String_Converter::Quoted_String_Converter()
:
result_(CONVERT_CHAR),
ustr_(0)
{
}

Quoted_String_Converter::~Quoted_String_Converter()
{
}

const char*
Quoted_String_Converter::quoted_to_str(const char* cstr, char quote, char escape)
{
	assert(cstr);
	std::string* dest = &buffer();

	dest->resize(0);
	const char* ptr = cstr;

	if ( *ptr != quote )
	{
		*dest += *ptr;
		return 0;
	}
	++ptr;

	const char* ch_buf = ptr;
	size_t ch_len = 0;
	while( *ptr /*!= '\0'*/ )
	{
		if ( result_ == CONVERT_CHAR )
		{
			if ( *ptr == quote )
			{
				result_ = CONVERT_QUOTE;
			}
			else if ( *ptr == escape )
			{
				result_ = CONVERT_ESCAPE;
			}
			else
			{
				//*dest += *ptr;
				++ch_len;
			}
		}
		else if ( result_ == CONVERT_ESCAPE )
		{
			if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
			ch_buf = ptr+1;
			ch_len = 0;

			*dest += *ptr;
			result_ = CONVERT_CHAR;
		}
		else // result_ == CONVERT_QUOTE
		{
			if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
			ch_buf = ptr+1;
			ch_len = 0;

			break;
		}
		++ptr;
	}

	if ( *ptr == 0 )
	{
		if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
		ch_buf = ptr+1;
		ch_len = 0;

		if ( result_ == CONVERT_QUOTE )
		{
			result_ = CONVERT_OK;
			return dest->c_str();
		}
	}

	return 0;
}

const char*
Quoted_String_Converter::quoted_to_str(const char* buf, size_t len, char quote, char escape)
{
	assert(buf && len > 0);
	std::string* dest = &buffer();

	dest->resize(0);
	const char* ptr = buf;
	const char* ptr_end = buf+len;

	if ( *ptr != quote )
	{
		*dest += *ptr;
		return 0;
	}
	++ptr;

	const char* ch_buf = ptr;
	size_t ch_len = 0;
	while( ptr < ptr_end )
	{
		if ( result_ == CONVERT_CHAR )
		{
			if ( *ptr == quote )
			{
				result_ = CONVERT_QUOTE;
			}
			else if ( *ptr == escape )
			{
				result_ = CONVERT_ESCAPE;
			}
			else
			{
				//*dest += *ptr;
				++ch_len;
			}
		}
		else if ( result_ == CONVERT_ESCAPE )
		{
			if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
			ch_buf = ptr+1;
			ch_len = 0;

			*dest += *ptr;
			result_ = CONVERT_CHAR;
		}
		else // result_ == CONVERT_QUOTE
		{
			if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
			ch_buf = ptr+1;
			ch_len = 0;

			break;
		}
		++ptr;
	}

	if ( ptr == ptr_end )
	{
		if ( ch_len > 0 ) dest->append(ch_buf, ch_len);
		ch_buf = ptr+1;
		ch_len = 0;

		if ( result_ == CONVERT_QUOTE )
		{
			result_ = CONVERT_OK;
			return dest->c_str();
		}
	}

	return 0;
}

const char*
Quoted_String_Converter::str_to_quoted(const char* cstr, char quote, char escape)
{
	std::string* dest = &buffer();

	dest->resize(0);
	const char* ptr = cstr;

	*dest += quote;
	while( *ptr != '\0' )
	{
		if ( *ptr == quote || *ptr == escape )
		{
			*dest += escape;
			*dest += *ptr;
		}
		else
		{
			*dest += *ptr;
		}

		++ptr;
	}
	*dest += quote;

	result_ = CONVERT_OK;
	return dest->c_str();
}

const char*
Quoted_String_Converter::str_to_quoted(const char* buf, size_t len, char quote, char escape)
{
	std::string* dest = &buffer();

	dest->resize(0);
	const char* ptr = buf;
	const char* ptr_end = buf+len;

	*dest += quote;
	while( ptr < ptr_end )
	{
		if ( *ptr == quote || *ptr == escape )
		{
			*dest += escape;
			*dest += *ptr;
		}
		else
		{
			*dest += *ptr;
		}

		++ptr;
	}
	*dest += quote;

	result_ = CONVERT_OK;
	return dest->c_str();
}

} // namespace aos

