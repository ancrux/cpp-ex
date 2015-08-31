#include "Codec.h"

#include <cassert>

namespace aos {

// Symbols which form the Base64 alphabet (Defined as per RFC 2045)
const char Base64::enc_tbl[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
	"abcdefghijklmnopqrstuvwxyz" \
	"0123456789+/";
// The padding character used in the encoding
const char pad = '=';
const char Base64::dec_tbl[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,62,-1,-1,-1,63,52,53,
	54,55,56,57,58,59,60,61,-1,-1,
	-1,-1,-1,-1,-1, 0, 1, 2, 3, 4, 
	5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,
	25,-1,-1,-1,-1,-1,-1,26,27,28,
	29,30,31,32,33,34,35,36,37,38,
	39,40,41,42,43,44,45,46,47,48,
	49,50,51,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1
};

size_t
Base64::encode(std::string& str, size_t line_size)
{
	aos::Base64 base64(line_size);
	std::string tmp(str);

	size_t n_enough_encode = base64.enough_encode_size(tmp.size());
	str.resize(n_enough_encode);
	size_t n_encode = base64.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
	str.resize(n_encode);

	return n_encode;
}

size_t
Base64::decode(std::string& str)
{
	aos::Base64 base64;
	size_t n_decode = base64.decode(str.c_str(), str.size(), (char*) str.c_str());
	str.resize(n_decode);

	return n_decode;
}

size_t
Base64::encode(const char* in, size_t in_size, char* out)
{
	const char* ptr = in;
	const char* ptr_end = in + in_size;
	char* o = out;

	int char_count = 0;
	int bits = 0;
	size_t cols = 0;

	// encode loop
	for(;ptr < ptr_end; ++ptr)
	{
		bits += (unsigned char) *ptr;
		++char_count;

		if ( char_count == 3 )
		{
			*out++ = enc_tbl[bits >> 18];
			*out++ = enc_tbl[(bits >> 12) & 0x3f];
			*out++ = enc_tbl[(bits >> 6) & 0x3f];
			*out++ = enc_tbl[bits & 0x3f];
			cols += 4;
			if ( line_size_ && cols >= line_size_ ) {
				*out++ = '\r'; *out++ = '\n'; // insert line break
				cols = 0;
			}
			bits = 0;
			char_count = 0;
		}
		else
		{
			bits <<= 8;
		}
	}

	// encode padding
	if ( char_count != 0 )
	{
		bits <<= (16 - (8 * char_count));
		*out++ = enc_tbl[bits >> 18];
		*out++ = enc_tbl[(bits >> 12) & 0x3f];
		if (char_count == 1)
		{
			*out++ = pad;
			*out++ = pad;
		}
		else
		{
			*out++ = enc_tbl[(bits >> 6) & 0x3f];
			*out++ = pad;
		}
		if ( line_size_ && cols >= line_size_ )
		{
			*out++ = '\r'; *out++ = '\n'; // insert line break
		}
	}

	*out = '\0';

	return (out - o);
}

// Base64::decode() can be in-place. (target == source)
size_t
Base64::decode(const char* in, size_t in_size, char* out)
{
	const unsigned char* ptr = (unsigned char*) in;
	const unsigned char* ptr_end = (unsigned char*) in + in_size;
	char* o = out;

	int char_count = 0;
	int bits = 0;

	// decode loop
	for(;ptr < ptr_end; ++ptr)
	{
		if ( *ptr == pad ) break;

		int code = dec_tbl[*ptr];
		if ( code < 0 ) continue;

		bits += code;
		++char_count;

		if (char_count == 4)
		{
			*out++ = static_cast<char>(bits >> 16);
			*out++ = static_cast<char>((bits >> 8) & 0xff);
			*out++ = static_cast<char>(bits & 0xff);
			bits = 0;
			char_count = 0;
		}
		else
		{
			bits <<= 6;
		}
	}

	// decode ending
	switch(char_count)
	{
	case 1:
		// Error: decode incomplete: at least 2 bits missing!
		//++n_err_;
		break;
	case 2:
		*out++ = static_cast<char>(bits >> 10);
		break;
	case 3:
		*out++ = static_cast<char>(bits >> 16);
		*out++ = static_cast<char>((bits >> 8) & 0xff);
		break;
	}

	*out = '\0';

	return (out - o);
}

size_t
QP::encode(std::string& str, size_t line_size)
{
	aos::QP qp(line_size);
	std::string tmp(str);

	size_t n_enough_encode = qp.enough_encode_size(tmp.size());
	str.resize(n_enough_encode);
	size_t n_encode = qp.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
	str.resize(n_encode);

	return n_encode;
}

size_t
QP::decode(std::string& str)
{
	aos::QP qp;
	size_t n_decode = qp.decode(str.c_str(), str.size(), (char*) str.c_str());
	str.resize(n_decode);

	return n_decode;
}

size_t
QP::encode(const char* in, size_t in_size, char* out)
{
	const unsigned char* ptr = (unsigned char*) in;
	const unsigned char* ptr_end = (unsigned char*) in + in_size;
	char* o = out;

	int n;
	size_t cols = 0;

	// encode loop
	for(;ptr < ptr_end; ++ptr)
	{
		// insert soft line break if over line size
		if ( line_size_ && cols >= line_size_ )
		{
			*out++ = '='; *out++ = '\r'; *out++ = '\n'; // insert soft line break
			cols = 0;
		}

		//if ( cm_alphanum_.has(*ptr) )
		//if ( ::isgraph(*ptr) )
		if ( *ptr < 0x80 && *ptr != '=' )
		{
			*out++ = *ptr;
			++cols;

			if ( *ptr == '\n' )
				cols = 0;
		}
		else
		{
			*out++ = '=';
			n = (*ptr & 0xF0) >> 4; // high byte
			if (n < 10)
				*out++ = '0' + n;
			else
				*out++ = 'A' + n - 10;

			n = *ptr & 0x0F; // low byte
			if (n < 10)
				*out++ = '0' + n;
			else
				*out++ = 'A' + n - 10;

			cols += 3;
		}
	}

	*out = '\0';

	return (out - o);
}

// QP::decode() can be in-place. (target == source)
size_t
QP::decode(const char* in, size_t in_size, char* out)
{
	const char* ptr = in;
	const char* ptr_end = in + in_size;
	char* o = out;
	
	int token_size = 0;
	char token[3];

	// decode loop
	for(;ptr < ptr_end; ++ptr)
	{
		if ( token_size )
		{
			if ( *ptr == '\r' || *ptr == '\n' )
			{
				// handle soft line break (/=\r?\n/), clear token
				if ( token_size == 1 ) 
				{
					if ( ptr+1 < ptr_end && *ptr == '\r' && *(ptr+1) == '\n' ) ++ptr;
					token_size = 0;
				}
				continue; // igonre line break in between token
			}

			if ( cm_hex_.has(*ptr) )
			{
				token[token_size] = *ptr;
				++token_size;
				if ( token_size == 3 )
				{
					unsigned char ch = 0;

					// first hex digit
					if (token[1] >= '0' && token[1] <= '9')
						ch = token[1] - '0';
					else if (token[1] >= 'A' && token[1] <= 'F')
						ch = token[1] - ('A' - 10);
					else if (token[1] >= 'a' && token[1] <= 'f')
						ch = token[1] - ('a' - 10);
					else {}

					ch <<= 4;

					// second hex digit
					if (token[2] >= '0' && token[2] <= '9')
						ch += token[2] - '0';
					else if (token[2] >= 'A' && token[2] <= 'F')
						ch += token[2] - ('A' - 10);
					else if (token[2] >= 'a' && token[2] <= 'f')
						ch += token[2] - ('a' - 10);
					else {}

					*out++ = (char) ch;
					// clear token
					token_size = 0;
				}
			}
			else
			{
				// non-hex, flush & clear token
				for(int i=0; i < token_size; ++i)
				{
					*out++ = token[i];
				}
				token_size = 0;
				*out++ = *ptr; // write current char
			}
		}
		else if ( *ptr == '=' )
		{
			token[0] = *ptr;
			token_size = 1;
		}
		else
		{
			*out++ = *ptr; // any characters else

			//if ( *ptr == '\r' || *ptr == '\n' )
			//	continue; // line break
			//else 
			//	*out++ = *ptr; // any characters else
		}
	}

	*out = '\0';

	return (out - o);
}

size_t
UU::encode(std::string& str, size_t line_size)
{
	aos::UU uu(line_size);
	std::string tmp(str);

	size_t n_enough_encode = uu.enough_encode_size(tmp.size());
	str.resize(n_enough_encode);
	size_t n_encode = uu.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
	str.resize(n_encode);

	return n_encode;
}

size_t
UU::decode(std::string& str)
{
	aos::UU uu;
	size_t n_decode = uu.decode(str.c_str(), str.size(), (char*) str.c_str());
	str.resize(n_decode);

	return n_decode;
}

size_t
UU::encode(const char* in, size_t in_size, char* out)
{
	register const char* ptr = in;
	const char* ptr_end = in + in_size;
	char* o = out;
	char last[80]; // buffer for last line

	//::printf("\r\nbegin 0666 %s\r\n", basename(filepath)); 

	int ch, n;
	int n_max = (int) (line_size_ / 4) * 3;

	while ( ptr < ptr_end )
	{
		n = (int)(ptr_end - ptr);
		if ( n > n_max ) n = n_max;
		else
		{
			// copy last line to another buffer, because ptr increments by 3
			::memcpy(last, ptr, (n+1) * sizeof(char));
			ptr = last;
			ptr_end = last + n;
		}

		*out++ = enc(n);
		// encode loop
		for(;n > 0; ptr += 3, n -= 3)
		{
			ch = ptr[0] >> 2;
			*out++ = enc(ch);
			ch = (ptr[0] << 4) & 060 | (ptr[1] >> 4) & 017;
			*out++ = enc(ch);
			ch = (ptr[1] << 2) & 074 | (ptr[2] >> 6) & 03;
			*out++ = enc(ch);
			ch = ptr[2] & 077;
			*out++ = enc(ch);
		}

		if ( line_size_ )
		{
			*out++ = '\r'; *out++ = '\n'; // insert line break
		}
	}

	//::printf("%c\r\nend\r\n", enc(0));

	*out = '\0';

	return (out - o);
}

size_t
UU::decode(const char* in, size_t in_size, char* out)
{
	const char* ptr = in;
	const char* ptr_end = in + in_size;
	char* o = out;

	register char ch;
	int n;

	// decode loop
	while ( ptr < ptr_end )
	{
		// ignore line break
		//while( *ptr == '\r' || *ptr == '\n' ) ++ptr;

		// n is used to avoid writing out all the characters at the end of the file.
		n = dec(*ptr);
		if (n <= 0) break;

		for (++ptr; n > 0; ptr += 4, n -= 3)
		{
			if (n >= 3)
			{
				ch = dec(ptr[0]) << 2 | dec(ptr[1]) >> 4; 
				*out++ = ch;
				ch = dec(ptr[1]) << 4 | dec(ptr[2]) >> 2; 
				*out++ = ch;
				ch = dec(ptr[2]) << 6 | dec(ptr[3]);
				*out++ = ch;
			}
			else
			{
				if (n >= 1)
				{
					ch = dec(ptr[0]) << 2 | dec(ptr[1]) >> 4; 
					*out++ = ch;
				}
				if (n >= 2)
				{
					ch = dec(ptr[1]) << 4 | dec(ptr[2]) >> 2; 
					*out++ = ch;
				}
			}
		}
		// ignore line break
		while( *ptr == '\r' || *ptr == '\n' ) ++ptr;
	}

	*out = '\0';

	return (out - o);
}

} // namespace aos
