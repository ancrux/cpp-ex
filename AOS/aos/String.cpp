#include "String.h"

//#include <ctype.h>

#include <cassert>

namespace aos {

int
strcasecmp(const char* cs, const char* ct)
{
	register signed char res;

	while(1)
	{
		if ((res = ::tolower(*cs) - ::tolower(*ct++)) != 0 || !*cs++)
			break;
	}

	return res;
}

int
strncasecmp(const char* cs, const char* ct, size_t count)
{
	register signed char res = 0;

	while(count)
	{
		if ((res = ::tolower(*cs) - ::tolower(*ct++) ) != 0 || !*cs++)
			break;
		count--;
	}

	return res;
}

int
nat_cmp_right_(const char* a, const char* b)
{
	int bias = 0;

	/* The longest run of digits wins.  That aside, the greatest
	value wins, but we can't know that it will until we've scanned
	both numbers to know that they have the same magnitude, so we
	remember it in BIAS. */
	for (;; a++, b++) 
	{
		if (!::isdigit(*a) && !::isdigit(*b))
			return bias;
		else if (!::isdigit(*a))
			return -1;
		else if (!::isdigit(*b))
			return +1;
		else if (*a < *b) {
			if (!bias)
				bias = -1;
		} else if (*a > *b) {
			if (!bias)
				bias = +1;
		} else if (!*a && !*b)
			return bias;
	}

	return 0;
}


int
nat_cmp_left_(const char* a, const char* b)
{
	/* Compare two left-aligned numbers: the first to have a
	different value wins. */
	for (;; a++, b++) 
	{
		if (!::isdigit(*a) && !::isdigit(*b))
			return 0;
		else if (!::isdigit(*a))
			return -1;
		else if (!::isdigit(*b))
			return +1;
		else if (*a < *b)
			return -1;
		else if (*a > *b)
			return +1;
	}

	return 0;
}


int
strnatcmp_(const char* a, const char* b, int fold_case)
{
	int ai, bi;
	char ca, cb;
	int fractional, result;

	assert(a && b);
	ai = bi = 0;
	while(1) 
	{
		ca = a[ai]; cb = b[bi];

		/* skip over leading spaces or zeros */
		while(::isspace(ca))
			ca = a[++ai];

		while(::isspace(cb))
			cb = b[++bi];

		/* process run of digits */
		if (::isdigit(ca) && ::isdigit(cb)) 
		{
			fractional = (ca == '0' || cb == '0');

			if (fractional) 
			{
				if ((result = nat_cmp_left_(a+ai, b+bi)) != 0)
					return result;
			}
			else 
			{
				if ((result = nat_cmp_right_(a+ai, b+bi)) != 0)
					return result;
			}
		}

		if (!ca && !cb) 
		{
			/* The strings compare the same.  Perhaps the caller
			will want to call strcmp to break the tie. */
			/* garetxe: Use str[case]cmp if they compare the same
			here */
			return (fold_case ? ::strcmp(a, b) : aos::stricmp(a, b));
		}

		if (fold_case) 
		{
			ca = ::tolower(ca);
			cb = ::tolower(cb);
		}

		if (ca < cb)
			return -1;
		else if (ca > cb)
			return +1;

		++ai; ++bi;
	}
}

const char*
strnstr(const char* big, size_t n_big, const char* little, size_t n_little)
{
	if ( n_little > n_big )
		return 0;

	size_t len = n_big - n_little;
	register const char* ptr = big;
	register const char* ptr_end = big + len;
	while( ptr <= ptr_end )
	{
		if ( *ptr == *little && ::memcmp(ptr, little, n_little) == 0 )
			return ptr;

		++ptr;
	}

	return 0;
}

const char*
strncasestr(const char* big, size_t n_big, const char* little, size_t n_little)
{
	if ( n_little > n_big )
		return 0;

	size_t len = n_big - n_little;
	register const char* ptr = big;
	register const char* ptr_end = big + len;
	while( ptr <= ptr_end )
	{
		if ( *ptr == *little && aos::strncasecmp(ptr, little, n_little) == 0 )
			return ptr;

		++ptr;
	}

	return 0;
}

/*// line getter

// Usage (std::string) Better:

static const int BUF_SIZE = 4096;
char buf[BUF_SIZE]; // static memory allocation (stack)
//char* buf = new (std::nothrow) char[BUF_SIZE]; // dynamical memory allocation (heap)

static const int LINE_SIZE = 128; // <= increase this to improve performance
char line[LINE_SIZE];
std::string one_line;

int c = 0; // line count
char delimiter = 0; // line delimiter
while( 1 )
{
	// read buffer
	int n_buf = (int) ACE_OS::fread(buf, sizeof(char), BUF_SIZE, fp);
	if ( n_buf == 0 )
	{
		++c; // count for last line
		//ACE_OS::printf("%d:%s", one_line.size(), one_line.c_str()); //@
		break;
	}

	// get line delimiter
	if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, n_buf);

	// get lines
	char* left = buf;
	int n_left = n_buf;
	while( 1 )
	{
		int n_read = aos::get_line(line, LINE_SIZE, left, n_left, delimiter);
		left += n_read;
		n_left -= n_read;

		if ( line[n_read] == '\0' )
		{
			++c; // read one completed line
			one_line.append(line, n_read);

			//ACE_OS::printf("%d:%s", one_line.size(), one_line.c_str()); //@
			one_line.clear();

			if ( c % 200000 == 0 ) ACE_OS::printf("read:%d\n", c);
		}
		else
		{
			line[n_read] = '\0';
			one_line.append(line, n_read);

			//ACE_OS::printf("+%d:%s\n", n_read, line); //@
		}

		if ( n_left <= 0 )
			break; // buffer consumed, get next buffer!
	}

}
ACE_OS::printf("\ntotal lines:%d\n", c);

// Usage (std::string) Normal:

static const int BUF_SIZE = 4096;
char buf[BUF_SIZE]; // static memory allocation (stack)
//char* buf = new (std::nothrow) char[BUF_SIZE]; // dynamical memory allocation (heap)

static const int LINE_SIZE = 128; // <= increase this to improve performance
std::string line;
line.resize(LINE_SIZE);

int c = 0; // line count
char delimiter = 0; // line delimiter
int n_len = 0; // line start
while( 1 )
{
	// read buffer
	int n_buf = (int) ACE_OS::fread(buf, sizeof(char), BUF_SIZE, fp);
	if ( n_buf == 0 )
	{
		++c; // count for last line
		//ACE_OS::printf("%d:%s", one_line.size(), one_line.c_str()); //@
		break;
	}

	// get line delimiter
	if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, n_buf);

	// get lines
	char* left = buf;
	int n_left = n_buf;
	while( 1 )
	{
		int n_read = aos::get_line(&line[n_len], LINE_SIZE, left, n_left, delimiter);
		left += n_read;
		n_left -= n_read;

		n_len += n_read;
		if ( line[n_len] == '\0' )
		{
			++c; // read one completed line
			line.resize(n_len);

			//ACE_OS::printf("%d:%s", line.size(), line.c_str()); //@
			line.clear();
			line.resize(LINE_SIZE);
			n_len = 0;

			if ( c % 200000 == 0 ) ACE_OS::printf("read:%d\n", c);
		}
		else
		{
			line.resize(n_len + LINE_SIZE);

			//ACE_OS::printf("+%d:%s\n", n_read, line); //@
		}

		if ( n_left <= 0 )
			break; // buffer consumed, get next buffer!
	}

}
ACE_OS::printf("\ntotal lines:%d\n", c);

// Usage (char*) Better: 

static const int BUF_SIZE = 4096;
char buf[BUF_SIZE]; // static memory allocation (stack)
//char* buf = new (std::nothrow) char[BUF_SIZE]; // dynamical memory allocation (heap)
static const int LINE_SIZE = 128; // <= increase this to improve performance
char line[LINE_SIZE];

int c = 0; // line count
char delimiter = 0; // line delimiter
while( 1 )
{
	// read buffer
	int n_buf = (int) ACE_OS::fread(buf, sizeof(char), BUF_SIZE, fp);
	if ( n_buf == 0 )
	{
		++c; // count for last line
		//ACE_OS::printf("%d:%s", n_read, line); //@
		break;
	}

	// get line delimiter
	if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, n_buf);

	// get lines
	char* left = buf;
	int n_left = n_buf;
	while( 1 )
	{
		int n_read = aos::get_line(line, LINE_SIZE, left, n_left, delimiter);
		left += n_read;
		n_left -= n_read;
		if ( line[n_read] == '\0' )
		{
			++c; // read one line
			//ACE_OS::printf("%d:%s", n_read, line); //@

			if ( c % 200000 == 0 ) ACE_OS::printf("read:%d\n", c);
		}
		else
		{
			line[n_read] = '\0';
			//ACE_OS::printf("+%d:%s\n", n_read, line); //@
		}

		if ( n_left <= 0 )
			break; // buffer consumed, get next buffer!
	}

}
ACE_OS::printf("\ntotal lines:%d\n", c);

//*/

int
get_line(char* buf, int n_buf, const char* src, int n_src, char delimiter)
{
	register char* ptr = buf;
	register char* ptr_end = buf + n_buf - 1; // for '\0';
	int is_crlf = 0;
	while( ptr < ptr_end && n_src )
	{
		--n_src;
		if ( (*ptr++ = *src++) == delimiter )
		{
			is_crlf = 1;
			break;
		}
	}
	if ( is_crlf ) *ptr = '\x00';
	else *ptr = '\x01';

	return int(ptr - buf);
}

char
get_line_delimiter(const char* buf, size_t len)
{
	register const char* ptr = buf;
	register const char* ptr_end = buf + len - 1; // for "\r\n"
	//int is_crlf = 0;
	while( ptr < ptr_end )
	{
		if ( *ptr == '\n' ) return *ptr;
		if ( *ptr == '\r' )
		{
			if ( *(ptr+1) == '\n' ) return *(ptr+1);
			return *ptr;
		}
		++ptr;
	}

	return '\xFF';
}

/*// Usage:

static const int BUF_SIZE = 4096;
char buf[BUF_SIZE]; // static memory allocation (stack)
//char* buf = new (std::nothrow) char[BUF_SIZE]; // dynamical memory allocation (heap)

aos::bcstr line;
std::string sline;

int c = 0; // line count
char delimiter = 0; // line delimiter
while( 1 )
{
	// read buffer
	int n_buf = (int) ACE_OS::fread(buf, sizeof(char), BUF_SIZE, fp);
	if ( n_buf == 0 )
	{
		++c; // count for last line

		e.parse_line(sline);
		// not needed! //sline.assign(line.buf, line.len); ACE_OS::printf("%d:%s", line.len, sline.c_str()); //@
		break;
	}

	// get line delimiter
	if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, n_buf);

	// get lines
	char* left = buf;
	int n_left = n_buf;
	while( 1 )
	{
		line = aos::get_line(left, n_left, delimiter);
		left += int(line.len);
		n_left -= int(line.len);
		if ( line.buf[line.len-1] == delimiter )
		{
			++c; // read one line

			sline.append(line.buf, line.len); //ACE_OS::printf("%d:%s", line.len, sline.c_str()); //@
			
			e.parse_line(sline);

			sline.resize(0);
			if ( c % 200000 == 0 ) ACE_OS::printf("read:%d\n", c);
		}
		else
		{
			sline.append(line.buf, line.len); //ACE_OS::printf("%d:%s", line.len, sline.c_str()); //@
		}

		if ( n_left <= 0 )
			break; // buffer consumed, get next buffer!
	}

}
ACE_OS::printf("\ntotal lines:%d\n", c);

//e.dump();
printf("total_bytes: %d\n", e.size());

//*/

bcstr
get_line(const char* buf, size_t len, char delimiter)
{
	bcstr str;
	str.buf = buf;
	str.len = 0;
	while( str.len < len )
	{
		++str.len;
		if ( *buf++ == delimiter ) break;
	}

	return str;
}

/// class QS (Quoted String) encode/decode

QS::QS(char quote, char escape)
{
	reset(quote, escape);
}

QS::~QS()
{
}

size_t
QS::encode(const char* in, size_t in_size, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	register const char* ptr_end = in + in_size;
	char* o = out;

	// encode loop
	*out++ = quote_;
	for(;ptr < ptr_end; ++ptr)
	{
		if ( *ptr == quote_ || *ptr == escape_ )
		{
			*out++ = escape_;
			*out++ = *ptr;
		}
		else
		{
			*out++ = *ptr;
		}
	}
	*out++ = quote_;

	state_ = ENCODE_OK;

	*out = '\0';
	return (out - o);
}

size_t
QS::encode(const char* in, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	char* o = out;

	// encode loop
	*out++ = quote_;
	while( *ptr )
	{
		if ( *ptr == quote_ || *ptr == escape_ )
		{
			*out++ = escape_;
			*out++ = *ptr;
		}
		else
		{
			*out++ = *ptr;
		}

		++ptr;
	}
	*out++ = quote_;

	state_ = ENCODE_OK;

	*out = '\0';
	return (out - o);
}

size_t
QS::decode(const char* in, size_t in_size, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	register const char* ptr_end = in + in_size;
	char* o = out;

	state_ = DECODE_CHAR;

	// decode loop
	if ( in_size > 0 && *ptr == quote_ )
	{
		++ptr;

		for(;ptr < ptr_end; ++ptr)
		{
			if ( state_ == DECODE_CHAR )
			{
				if ( *ptr == quote_ )
				{
					state_ = DECODE_QUOTE;
				}
				else if ( *ptr == escape_ )
				{
					state_ = DECODE_ESCAPE;
				}
				else
				{
					*out++ = *ptr;
				}
			}
			else if ( state_ == DECODE_ESCAPE )
			{
				*out++ = *ptr;
				state_ = DECODE_CHAR;
			}
			else // state_ == DECODE_QUOTE
			{
				break;
			}
		}

		if ( state_ == DECODE_QUOTE )
		{
			state_ = DECODE_OK;
		}
	}

	n_read_ = ptr - in;
	*out = '\0';
	return (out - o);
}

size_t
QS::decode(const char* in, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	char* o = out;

	state_ = DECODE_CHAR;

	// decode loop
	if ( *ptr == quote_ )
	{
		++ptr;

		while( *in )
		{
			if ( state_ == DECODE_CHAR )
			{
				if ( *ptr == quote_ )
				{
					state_ = DECODE_QUOTE;
				}
				else if ( *ptr == escape_ )
				{
					state_ = DECODE_ESCAPE;
				}
				else
				{
					*out++ = *ptr;
				}
			}
			else if ( state_ == DECODE_ESCAPE )
			{
				*out++ = *ptr;
				state_ = DECODE_CHAR;
			}
			else // state_ == DECODE_QUOTE
			{
				break;
			}

			++ptr;
		}

		if ( state_ == DECODE_QUOTE )
		{
			state_ = DECODE_OK;
		}
	}

	n_read_ = ptr - in;
	*out = '\0';
	return (out - o);
}

/// class QCS (Quoted C-String) encode/decode

QCS::QCS(char quote, char escape)
{
	reset(quote, escape);
}

QCS::~QCS()
{
}

size_t
QCS::encode(const char* in, size_t in_size, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	register const char* ptr_end = in + in_size;
	char* o = out;

	// encode loop
	*out++ = quote_;
	for(;ptr < ptr_end; ++ptr)
	{
		if ( *ptr == quote_ || *ptr == escape_ )
		{
			*out++ = escape_;
			*out++ = *ptr;
		}
		else
		{
			*out++ = *ptr;
		}
	}
	*out++ = quote_;

	state_ = ENCODE_OK;

	*out = '\0';
	return (out - o);
}

size_t
QCS::encode(const char* in, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	char* o = out;

	// encode loop
	*out++ = quote_;
	while( *ptr )
	{
		if ( *ptr == quote_ || *ptr == escape_ )
		{
			*out++ = escape_;
			*out++ = *ptr;
		}
		else
		{
			*out++ = *ptr;
		}

		++ptr;
	}
	*out++ = quote_;

	state_ = ENCODE_OK;

	*out = '\0';
	return (out - o);
}

size_t
QCS::decode(const char* in, size_t in_size, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	register const char* ptr_end = in + in_size;
	char* o = out;

	state_ = DECODE_CHAR;

	// decode loop
	if ( in_size > 0 && *ptr == quote_ )
	{
		++ptr;

		for(;ptr < ptr_end; ++ptr)
		{
			if ( state_ == DECODE_CHAR )
			{
				if ( *ptr == quote_ )
				{
					state_ = DECODE_QUOTE;
				}
				else if ( *ptr == escape_ )
				{
					state_ = DECODE_ESCAPE;
				}
				else
				{
					*out++ = *ptr;
				}
			}
			else if ( state_ == DECODE_ESCAPE )
			{
				*out++ = *ptr;
				state_ = DECODE_CHAR;
			}
			else // state_ == DECODE_QUOTE
			{
				break;
			}
		}

		if ( state_ == DECODE_QUOTE )
		{
			state_ = DECODE_OK;
		}
	}

	n_read_ = ptr - in;
	*out = '\0';
	return (out - o);
}

size_t
QCS::decode(const char* in, char* out)
{
	assert(in && out);

	register const char* ptr = in;
	char* o = out;

	state_ = DECODE_CHAR;

	// decode loop
	if ( *ptr == quote_ )
	{
		++ptr;

		while( *in )
		{
			if ( state_ == DECODE_CHAR )
			{
				if ( *ptr == quote_ )
				{
					state_ = DECODE_QUOTE;
				}
				else if ( *ptr == escape_ )
				{
					state_ = DECODE_ESCAPE;
				}
				else
				{
					*out++ = *ptr;
				}
			}
			else if ( state_ == DECODE_ESCAPE )
			{
				*out++ = *ptr;
				state_ = DECODE_CHAR;
			}
			else // state_ == DECODE_QUOTE
			{
				break;
			}

			++ptr;
		}

		if ( state_ == DECODE_QUOTE )
		{
			state_ = DECODE_OK;
		}
	}

	n_read_ = ptr - in;
	*out = '\0';
	return (out - o);
}

/// class Multi_String

Multi_String::Multi_String()
{
}

Multi_String::~Multi_String()
{
}

void
Multi_String::push_back(const char* cstr)
{
	offs_.push_back(strs_.size());
	strs_.append(cstr); strs_ += '\0';
}

void
Multi_String::push_back(const char* buf, size_t len)
{
	offs_.push_back(strs_.size());
	strs_.append(buf, len); strs_ += '\0';
}

void
Multi_String::append_to_last(const char* cstr)
{
	if ( !empty() )
	{
		strs_.resize(strs_.size()-1); // take off the last string's '\0'
		strs_.append(cstr); strs_ += '\0';
	}
	else
		push_back(cstr);

}

void
Multi_String::append_to_last(const char* buf, size_t len)
{
	if ( !empty() )
	{
		strs_.resize(strs_.size()-1); // take off the last string's '\0'
		strs_.append(buf, len); strs_ += '\0';
	}
	else
		push_back(buf, len);
}

void
Multi_String::erase(size_t pos)
{
	size_t clen = capacity(pos);
	if ( clen ) // if pos exists
	{
		// erase internal string
		size_t start = offs_[pos];
		strs_.erase(start, clen);

		// adjust offset
		int offset = 0 - (int) clen;
		adjust_offsets(pos, offset);
		offs_.erase(offs_.begin()+pos);
	}
}

void
Multi_String::insert(size_t pos, const char* cstr)
{
	size_t clen = capacity(pos);
	if ( clen ) // if pos exists
	{
		std::string str(cstr); str += '\0';

		// insert internal string
		size_t start = offs_[pos];
		strs_.insert(start, str);

		// adjust offset
		offs_.insert(offs_.begin()+pos, start);
		adjust_offsets(pos, (int) str.size());
	}
}

void
Multi_String::insert(size_t pos, const char* buf, size_t len)
{
	size_t clen = capacity(pos);
	if ( clen ) // if pos exists
	{
		std::string str(buf, len); str += '\0';

		// insert internal string
		size_t start = offs_[pos];
		strs_.insert(start, str);

		// adjust offset
		offs_.insert(offs_.begin()+pos, start);
		adjust_offsets(pos, (int) str.size());
	}
}

void
Multi_String::replace(size_t pos, const char* cstr)
{
	size_t clen = capacity(pos);
	if ( clen ) // if pos exists
	{
		std::string str(cstr); str += '\0';

		// replace internal string
		size_t start = offs_[pos];
		strs_.replace(start, clen, str);

		// adjust offset
		int offset = (int) str.size() - (int) clen;
		adjust_offsets(pos, offset);
	}
}

void
Multi_String::replace(size_t pos, const char* buf, size_t len)
{
	size_t clen = capacity(pos);
	if ( clen ) // if pos exists
	{
		std::string str(buf, len); str += '\0';

		// replace internal string
		size_t start = offs_[pos];
		strs_.replace(start, clen, str);

		// adjust offset
		int offset = (int) str.size() - (int) clen;
		adjust_offsets(pos, offset);
	}
}

void
Multi_String::dump(char sep, char quote_begin, char quote_end) const
{
	size_t n = this->size();
	::printf("%c", quote_begin);
	for(size_t i=0; i<n; ++i)
	{
		(i)?::printf("%c%s", sep, this->operator[](i)):
			::printf("%s", this->operator[](i));
	}
	::printf("%c\n", quote_end);
}

void
Multi_String::explode(const char separator, const char* cstr, int start, int size)
{
	assert(cstr);

	int max = start + size;
	this->clear();

	const char* beg;
	const char* end;
	beg = end = cstr;
	int i = 0;
	while( *end )
	{
		if ( *end == separator )
		{
			if ( i >= start && (size < 0 || i < max) )
				this->push_back(beg, end-beg);
			beg = ++end;
			++i;
		}
		else
			++end;
	}
	this->push_back(beg, end-beg);
}

void
Multi_String::explode(const char* separator, const char* cstr, int start, int size)
{
	assert(separator && cstr);

	int max = start + size;
	this->clear();

	size_t n_separator = ::strlen(separator);
	if ( n_separator == 0 )
	{
		this->push_back(cstr, ::strlen(cstr));
		return;
	}

	const char* beg;
	const char* end;
	beg = cstr;
	int i = 0;
	while( (end = ::strstr(beg, separator)) )
	{
		if ( i >= start && (size < 0 || i < max) )
			this->push_back(beg, end-beg);

		beg = end + n_separator;
		++i;
	}
	this->push_back(beg, ::strlen(beg));
}

void
Multi_String::explode_token(const char* delimiter, const char* cstr, int start, int size)
{
	assert(delimiter && cstr);

	int max = start + size;
	this->clear();

	aos::Tokenizer toker(cstr);
	toker.set_separator(delimiter);

	int ch = toker.next();
	int i = 0;
	while( ch > aos::Tokenizer::End )
	{
		if ( i >= start && (size < 0 || i < max) )
			this->push_back(toker.token(), toker.size());
		ch = toker.next();
		++i;
	}
}

std::string
Multi_String::implode(const char* glue, int start, int size)
{
	int end = start + size;
	std::string str;
	
	int n = (int) this->size();
	if ( size < 0 || end > n ) end = n;
	
	for(int i=start; i<end; ++i)
	{
		if ( i != start ) str += glue;
		str += this->operator[](i);
	}
	
	return str;
}

/// class Char_Tokenizer

Char_Tokenizer::Char_Tokenizer(const char* str)
:
beg_(str),
end_(str),
min_(str),
max_(str),
cm_(0)
{
	if (str) max_ += ::strlen(str);
}

Char_Tokenizer::Char_Tokenizer(const char* str, size_t size)
:
beg_(str),
end_(str),
min_(str),
max_(str),
cm_(0)
{
	if (str) max_ += size;
}

void
Char_Tokenizer::set_separator(const char* sep, int invert)
{
	// Clear control map
	cm_.clear();

	// Set bits in delimiter table
	while( *sep )
	{
		cm_ |= *sep;
		++sep;
	}

	// invert if needed
	if ( invert )
	{
		cm_.invert();
	}
}

int
Char_Tokenizer::next(int skip_leading_separator, const char* begin)
{
	//if ( end_ >= max_ ) return Tokenizer::End;

	int found = 0;
	if ( begin ) beg_ = end_ = begin;
	if ( beg_ < end_ ) beg_ = end_ + 1;
	end_ = beg_;

	// Find beginning of token (skip over leading delimiters). Note that
	// there is no token iff this loop sets str to point to the terminal
	// null (*str == '\0')
	if ( skip_leading_separator )
	{
		while( cm_.has(*end_) && end_ < max_ )
			++end_;
		beg_ = end_;
	}
	else
	{
		if ( cm_.has(*end_) && end_ < max_ )
		{
			beg_ = ++end_;
			return *(beg_-1);
		}
	}

	// Find the end of the token
	for(; end_ < max_; ++end_)
	{
		if ( cm_.has(*end_) )
		{
			found = 1;
			break;
		}
	}

	if ( found ) return (unsigned char) *end_; // found token ended with separators
	if ( end_ == max_ && end_ != beg_ ) return Tokenizer::Last; // found token that ends up to string end.
	return Tokenizer::End;
}

int
Char_Tokenizer::prev(int skip_leading_separator, const char* begin)
{
	//if ( beg_ < min_ ) return Tokenizer::End;
	
	int found = 0;
	if ( begin ) beg_ = end_ = begin;
	if ( beg_ < end_ ) end_ = beg_ - 1;
	beg_ = end_;

	// Find beginning of token (skip over leading delimiters). Note that
	// there is no token iff this loop sets str to point to the terminal
	// null (*str == '\0')

	--beg_;
	if ( skip_leading_separator )
	{
		while( cm_.has(*beg_) && beg_ >= min_ )
			--beg_;
		end_ = beg_ + 1;
	}
	else
	{
		if ( cm_.has(*beg_) && beg_ >= min_ )
		{
			end_ = beg_;
			return *(beg_);
		}
	}

	// Find the end of the token
	for(; beg_ >= min_; --beg_)
	{
		if ( cm_.has(*beg_) )
		{
			found = 1;
			break;
		}
	}
	++beg_;

	if ( found ) return (unsigned char) *(beg_-1); // found token ended with separators
	if ( beg_ == min_ && end_ != beg_ ) return Tokenizer::Last; // found token that ends up to string end.
	return Tokenizer::End;
}

/// class Char_Map

Char_Map::Char_Map(int set_zero)
{
	if ( set_zero ) clear();
}

Char_Map::Char_Map(const char ch)
{
	set(ch);
}

Char_Map::Char_Map(const char* cstr)
{
	set(cstr);
}

Char_Map::Char_Map(const char* buf, size_t len)
{
	set(buf, len);
}

//Char_Map::Char_Map(const Char_Map& rhs) // copy constructor
//{
//	::memcpy(map_, rhs.map_, 32);
//}

void
Char_Map::dump() const throw()
{
	::printf("[");
	for(int i=0; i < 32; ++i)
	{
		::printf("%.2X", map_[i]);
		if ( i != 31 ) ::printf("-");
	}
	::printf("]");
}

////
//aos::trim() performance for the following code snippet
//on the same computer
////
//std::string str;
//for(int i=0; i < 10000000; ++i)
//{
//	str = "\t\t\t\t\t12dex\t\t\tsfxdc\t\t\t\t\t\t";
//	aos::trim(str); // 2859ms
//	//aos::trim(str, " \t\r\n\v"); //3688ms
//	//aos::trim(str, 0x09); // 3203ms
//}
//::printf("%s\n", str.c_str());
//
//const char* str = "\t\t\t\t\t12dex\t\t\tsfxdc\t\t\t\t\t\t";
//char buf[128];
//char* cstr;
//for(int i=0; i < 10000000; ++i)
//{
//	::strcpy(buf, str);
//	cstr = aos::trim(buf); // 937ms
//	//cstr = aos::trim(buf, " \t\r\n\v"); // 1484ms
//	//cstr = aos::trim(buf, 0x09); // 734ms
//}
//::printf("%s\n", cstr);
////
//<?php
//$time_start = microtime_float();
//
//for($i = 0; $i < 10000000; ++$i)
//{
//$str = trim("\t\t\t\t\t12dex\t\t\tsfxdc\t\t\t\t\t\t"); // 12118ms
////$str = trim("\t\t\t\t\t12dex\t\t\tsfxdc\t\t\t\t\t\t", " \t\r\n\v"); // 14376ms
////$str = trim("\t\t\t\t\t12dex\t\t\tsfxdc\t\t\t\t\t\t", "\t"); // 13760ms
//}
//
//$time_end = microtime_float();
//$time = $time_end - $time_start;
//
//echo '<pre>';
//echo $str;
//echo "\r\n";
//echo "time-total={$time}";
//echo '</pre>';
//
//function microtime_float() 
//{ 
//   list($usec, $sec) = explode(" ", microtime()); 
//   return ((float)$usec + (float)$sec); 
//}
//?>
////

std::string&
ltrim(std::string& str, const aos::Char_Map* cm)
{
	if ( !cm )
		cm = &TRIM_CM;

	register const char* ptr = str.c_str();
	register const char* ptr_end = ptr + str.size();
	int n_trim = 0;
	while( ptr < ptr_end )
	{
		if ( !cm->has(*ptr) ) break;
		++ptr;
		++n_trim;
	}

	if ( n_trim ) str.erase(0, n_trim);

	return str;
}

char*
ltrim(char* cstr, const aos::Char_Map* cm)
{
	if ( !cm )
		cm = &TRIM_CM;

	while( *cstr && cm->has(*cstr) )
	{
		++cstr;
	}

	return cstr;
}

std::string&
rtrim(std::string& str, const aos::Char_Map* cm)
{
	if ( !cm )
		cm = &TRIM_CM;

	register const char* ptr = str.c_str();
	register const char* ptr_end = ptr + str.size();
	int n_trim = 0;
	while( ptr < ptr_end )
	{
		--ptr_end;
		if ( !cm->has(*ptr_end) ) break;
		++n_trim;
	}

	if ( n_trim ) str.erase(str.end()-n_trim, str.end());

	return str;
}

char*
rtrim(char* cstr, const aos::Char_Map* cm)
{
	if ( !cm )
		cm = &TRIM_CM;

	register char* ptr = cstr + ::strlen(cstr);
	while( cstr < ptr )
	{
		--ptr;
		if ( !cm->has(*ptr) )
		{
			*(ptr+1) = '\0';
			break;
		}
	}

	return cstr;
}

char*
ltrim(char* cstr, const char* trim)
{
	Char_Map cm;
	while( *trim )
	{
		cm |= *trim;
		++trim;
	}
	return ltrim(cstr, &cm);
}

char*
rtrim(char* cstr, const char* trim)
{
	Char_Map cm;
	while( *trim )
	{
		cm |= *trim;
		++trim;
	}
	return rtrim(cstr, &cm);
}

char*
ltrim(char* cstr, const char trim)
{
	while( *cstr && *cstr == trim )
	{
		++cstr;
	}

	return cstr;
}

char*
rtrim(char* cstr, const char trim)
{
	register char* ptr = cstr + ::strlen(cstr);
	while( cstr < ptr )
	{
		--ptr;
		if ( *ptr == trim )
		{
			*(ptr+1) = '\0';
			break;
		}
	}

	return cstr;
}

std::string&
toupper(std::string& str)
{
	size_t n = str.size();
	for(size_t i=0; i < n; ++i)
		str[i] = ::toupper(str[i]);

	return str;
}

std::string&
tolower(std::string& str)
{
	size_t n = str.size();
	for(size_t i=0; i < n; ++i)
		str[i] = ::tolower(str[i]);

	return str;
}

char*
toupper(char* cstr)
{
	while( *cstr )
	{
		*cstr = ::toupper(*cstr);
		++cstr;
	}

	return cstr;
}

char*
tolower(char* cstr)
{
	while( *cstr )
	{
		*cstr = ::tolower(*cstr);
		++cstr;
	}

	return cstr;
}

// example:
//	std::map< std::string, std::string > vars;
//	vars.insert(std::make_pair("name", "'%real_name%'"));
//	vars.insert(std::make_pair("real_name", "ANGUS"));
//	
//	std::string str = "Hi! My name is %name%.";
//	int n_replace = 0;
//	n_replace = aos::var_replace(str, vars);
//	printf("(%d)%s\n", n_replace, str.c_str());
//	n_replace = aos::var_replace(str, vars);
//	printf("(%d)%s\n", n_replace, str.c_str());

int
var_replace(std::string& str, std::map< std::string, std::string > vars, char var_symbol)
{
	int n_replace = 0;

	size_t beg = 0;
	size_t end = 0;

	while( (beg = str.find(var_symbol, end)) != std::string::npos )
	{
		if ( (end = str.find(var_symbol, beg+1)) != std::string::npos && end-beg > 1 )
		{
			std::map< std::string, std::string >::const_iterator iter;
			iter = vars.find(str.substr(beg+1, end-beg-1));
			if ( iter != vars.end() )
			{
				str.replace(beg, end-beg+1, iter->second);
				end = beg + iter->second.size();
				++n_replace;
			}
		}
	}

	return n_replace;
}

int
var_replace_n(std::string& str, std::map< std::string, std::string > vars, int n, char var_symbol)
{
	int n_replace = 0;

	for(int i=0; i < n; ++i)
	{
		if ( (n_replace = var_replace(str, vars, var_symbol)) == 0 )
			break;
	}

	return n_replace;
}

} // namespace aos
