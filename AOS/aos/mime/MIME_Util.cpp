#include "aos/mime/MIME_Util.h"

#include "aos/Codec.h"
//#include "aos/String.h"

#include "unicode/utypes.h" /* Basic ICU data types */
#include "unicode/ucnv.h" /* C Converter API */
#include "unicode/ucsdet.h" /* C Detector API */

namespace aos {

/// MIME_Header_Util

std::string&
MIME_Header_Util::only_field_value(std::string& str)
{
	size_t colon = str.find_first_of(':');
	if ( colon != std::string::npos )
		str.erase(0, colon + 1);
	aos::trim(str);

	return str;
}

std::string
MIME_Header_Util::copy_field_value(const std::string& str)
{
	std::string header(str);

	MIME_Header_Util::only_field_value(header);

	return header;
}

MIME_Header_Util::MIME_Header_Util(size_t line_size)
:
parser_(MIME_Header_Util::No_Parser)
{
	this->line_size(line_size);
}

MIME_Header_Util::~MIME_Header_Util()
{
}

void
MIME_Header_Util::parse_encoded(const std::string& str, int trim_crlf, int map_charset)
{
	pairs_.clear();
	parser_ = MIME_Header_Util::Encoded_Parser;

	std::string s;
	if ( trim_crlf )
	{
		aos::Tokenizer toker(str.c_str(), str.size());
		toker.set_separator("\r\n");

		int ch = toker.next();
		while( ch > aos::Tokenizer::End )
		{
			const char* buf = toker.token();
			size_t len = toker.size();
			if ( len > 0 && ( *buf == ' ' || *buf == '\t' ) )
			{
				++buf;
				--len;
			}
			s.append(buf, len);

			ch = toker.next();
		}
	}
	else
	{
		s.assign(str);
	}

	std::string charset;
	std::string text;
	size_t pos_beg = 0;
	size_t pos_end = 0;

	while( (pos_end = s.find("=?", pos_beg)) != std::string::npos )
	{
		text.assign(s, pos_beg, pos_end-pos_beg);
		if ( !text.empty() )
		{
			// text without charset will be merged into the last pair
			if ( !pairs_.empty() )
			{
				Pairs::iterator last = --(pairs_.end());
				last->second += text;
			}
			else
				pairs_.push_back(std::make_pair("", text));
		}
		pos_beg = pos_end;

		size_t qmark1 = s.find('?', pos_beg+2);
		pos_end = qmark1;
		if ( qmark1 != std::string::npos && qmark1-pos_beg > 2 )
		{
			size_t qmark2 = s.find('?', qmark1+1);
			pos_end = qmark2;
			if ( qmark2 != std::string::npos && qmark2-qmark1 == 2 )
			{
				char enc = ::toupper(s[qmark1+1]);
				if ( enc == 'B' || enc == 'Q' )
				{
					size_t qmark_end = s.find("?=", qmark2+1);
					pos_end = qmark_end;
					if ( qmark_end != std::string::npos )
					{
						charset.assign(s, pos_beg+2, qmark1-pos_beg-2);
						text.assign(s, qmark2+1, qmark_end-qmark2-1);

						if ( map_charset ) MIME_Util::map_charset(charset);

						switch(enc)
						{
							case 'B':
								{
								Base64::decode(text); //MIME_Util::from_Base64(text);
								//aos::Base64 base64;
								//size_t n_decode = base64.decode(text.c_str(), text.size(), (char*) text.c_str());
								//text.resize(n_decode);
								break;
								}
							case 'Q':
								{
								//RFC2047: 4.2. The "Q" encoding (2)
								//replace '_' with ' '
								size_t n_len = text.size();
								char* cstr = (char*) text.c_str();
								for(size_t i=0; i < n_len; ++i)
								{
									if ( *(cstr+i) == '_' ) *(cstr+i) = ' ';
								}

								QP::decode(text); //MIME_Util::from_QP(text);
								//aos::QP qp;
								//size_t n_decode = qp.decode(text.c_str(), text.size(), (char*) text.c_str());
								//text.resize(n_decode);
								break;
								}
							default:
								break;
						}

						// if the same charset as the last one, append text to the last one
						// else new entry
						if ( !pairs_.empty() )
						{
							Pairs::iterator last = --(pairs_.end());
							if ( aos::strnicmp(last->first.c_str(), charset.c_str(), charset.size()) == 0 )
								last->second += text;
							else
								pairs_.push_back(std::make_pair(charset, text));
						}
						else
							pairs_.push_back(std::make_pair(charset, text));

						pos_end += 2;
						pos_beg = pos_end;

						continue;
					}
				}
			}
		}

		text.assign(s, pos_beg, pos_end-pos_beg);
		if ( !text.empty() )
		{
			pairs_.push_back(std::make_pair("", text));

			//// text without charset will be merged into the last pair
			//if ( !pairs_.empty() )
			//{
			//	Pairs::iterator last = --(pairs_.end());
			//	last->second += text;
			//}
			//else
			//	pairs_.push_back(std::make_pair("", text));
		}
		pos_beg = pos_end;
	}

	if ( pos_beg != std::string::npos )
	{
		text.assign(s, pos_beg, std::string::npos);
		pairs_.push_back(std::make_pair("", text));

		//// text without charset will be merged into the last pair
		//if ( !pairs_.empty() )
		//{
		//	Pairs::iterator last = --(pairs_.end());
		//	last->second += text;
		//}
		//else
		//	pairs_.push_back(std::make_pair("", text));
	}
}

int
MIME_Header_Util::map_charset_decoded(const char* default_charset)
{
 	if ( parser_ != MIME_Header_Util::Encoded_Parser )
		return -1;

	for(MIME_Header_Util::Pairs::iterator it = pairs_.begin();
		it != pairs_.end();
		++it)
	{
		MIME_Util::map_charset(it->first, default_charset);
	}

	return 0;
}

UnicodeString&
MIME_Header_Util::build_unicode_decoded(UnicodeString& ustr)
{
	ustr.truncate(0);
	if ( parser_ != MIME_Header_Util::Encoded_Parser )
		return ustr;

	MIME_Header_Util::Pairs::iterator it = pairs_.begin();
	while( it != pairs_.end() )
	{
		if ( it->first.empty() )
		{
			ustr += it->second.c_str();
		}
		else
		{
			UErrorCode status;
			ustr += MIME_Util::to_unicode(it->second, it->first.c_str(), status);
		}
		++it;
	}

	return ustr;
}

std::string&
MIME_Header_Util::build_utf8_decoded(std::string& str)
{
	UnicodeString ustr;
	UErrorCode status = U_ZERO_ERROR;
	str = MIME_Util::from_unicode(MIME_Header_Util::build_unicode_decoded(ustr), "utf-8", status);
	return str;
}

void
MIME_Header_Util::parse_address(const std::string& str)
{
	pairs_.clear();
	parser_ = MIME_Header_Util::Address_Parser;

	const char* ptr = str.c_str();
	const char* start = ptr;
	const char* ptr_end = ptr + str.size();

	std::string item;
	std::string name;
	while( ptr < ptr_end )
	{
		if ( *ptr == '"' )
		{
			item.append(start, ptr-start);
			
			aos::QS qs;
			size_t n_decode = qs.decode(ptr, (char*) ptr);
			item.append(ptr, n_decode);

			ptr += qs.decode_read();
			start = ptr;
		}
		else if ( *ptr == ',' )
		{
			item.append(start, ptr-start);

			// process item
			bool angle_brackets = false;
			size_t angle_lpos = item.find_last_of('<');
			if ( angle_lpos != std::string::npos )
			{
				size_t angle_rpos = item.find_last_of('>');
				if ( angle_rpos != std::string::npos && angle_rpos > angle_lpos )
				{
					angle_brackets = true;
					name = item.substr(0, angle_lpos);
					pairs_.push_back(std::make_pair(aos::trim(name), item.substr(angle_lpos+1, angle_rpos-angle_lpos-1)));
				}
			}

			if ( !angle_brackets )
				pairs_.push_back(std::make_pair("", aos::trim(item)));

			start = ++ptr;

			// clear item
			item.resize(0);
		}
		else
		{
			++ptr;
		}
	}
	// process last item
	item.append(start, ptr-start);

	bool angle_brackets = false;
	size_t angle_lpos = item.find_last_of('<');
	if ( angle_lpos != std::string::npos )
	{
		size_t angle_rpos = item.find_last_of('>');
		if ( angle_rpos != std::string::npos && angle_rpos > angle_lpos )
		{
			angle_brackets = true;
			name = item.substr(0, angle_lpos);
			pairs_.push_back(std::make_pair(aos::trim(name), item.substr(angle_lpos+1, angle_rpos-angle_lpos-1)));
		}
	}

	if ( !angle_brackets )
		pairs_.push_back(std::make_pair("", aos::trim(item)));

	/*
	// aos::Tokenizer version
	int ch;
	aos::Tokenizer toker(str.c_str(), str.size());
	toker.set_separator("\",");

	std::string item;
	const char* start = str.c_str();
	while(true)
	{
		ch = toker.next(0, start);
		if ( ch <= aos::Tokenizer::End ) break;

		item.append(toker.token(), toker.size());
		size_t n_size = toker.size();
		if ( ch == '"' )
		{
			start = (toker.size())?toker.token_end():toker.token_end()-1; // zero-size contains '"'

			aos::QS qs;
			size_t n_decode = qs.decode(start, (char*) start);
			item.append(start, n_decode);

			start += qs.decode_read();
		}
		else
		{
			start = toker.token_end()+1;

			// process item
			//::printf("%s\n", item.c_str()); //@

			bool angle_brackets = false;
			size_t angle_lpos = item.find_last_of('<');
			if ( angle_lpos != std::string::npos )
			{
				size_t angle_rpos = item.find_last_of('>');
				if ( angle_rpos != std::string::npos && angle_rpos > angle_lpos )
				{
					angle_brackets = true;
					pairs_.push_back(std::make_pair(aos::trim(item.substr(0, angle_lpos)), item.substr(angle_lpos+1, angle_rpos-angle_lpos-1)));
				}
			}

			if ( !angle_brackets )
				pairs_.push_back(std::make_pair("", aos::trim(item)));

			// clear item
			item.resize(0);
		}
	}
	//*/
}



std::string&
MIME_Header_Util::build_address(std::string& str, const char* charset, char enc )
{
	str.resize(0);
	if ( parser_ != MIME_Header_Util::Address_Parser )
		return str;

	MIME_Header_Util::Pairs::iterator it = pairs_.begin();
	while( it != pairs_.end() )
	{
		//+ address name to be encoded with charset
		if ( !it->first.empty() )
		{
			//+ should use aos::QS to encode
			str += "\"";
			str += it->first; 
			str += "\" ";
		}
		if ( !it->second.empty() )
		{
			str += "<";
			str += it->second;
			str += ">, ";
		}

		++it;
	}

	if ( str.size() >= 2 )
		str.resize(str.size()-2);

	return str;
}

void
MIME_Header_Util::parse_attribute(const std::string& str)
{
	pairs_.clear();
	parser_ = MIME_Header_Util::Attribute_Parser;

	int ch;
	aos::Tokenizer toker;
	toker.str(str.c_str(), str.size());

	std::string attrib_name;
	std::string attrib_value;

	// get first attribute value followed by ':', e.g 'Content-Disposition: attachment;'
	toker.set_separator(";\r\n");
	ch = toker.next();
	if ( ch == ';' )
	{
		attrib_value.assign(toker.token(), toker.size());

		pairs_.push_back(std::make_pair("", trim_quotes(attrib_value)));
	}

	// get remaining attribute value(s) followed by '=', e.g 'filename="example.pdf";'
	while(true)
	{
		// read attribute name
		toker.set_separator("=\r\n"); //toker.set_separator("=\r\n\t ");
		ch = toker.next();
		if ( ch <= aos::Tokenizer::End ) break;
		attrib_name.assign(toker.token(), toker.size());
		aos::trim(attrib_name);

		// read attribute value
		toker.set_separator(";\r\n");
		ch = toker.next();
		if ( ch <= aos::Tokenizer::End ) break;
		attrib_value.assign(toker.token(), toker.size());

		pairs_.push_back(std::make_pair(attrib_name, trim_quotes(attrib_value)));
	}
}

std::string&
MIME_Header_Util::build_attribute(std::string& str, const char* charset, char enc)
{
	str.resize(0);
	if ( parser_ != MIME_Header_Util::Attribute_Parser )
		return str;

	MIME_Header_Util::Pairs::iterator it = pairs_.begin();
	int count = 0;
	while( it != pairs_.end() )
	{
		if ( count )
		{
			str += it->first;
			//+ attribute value to be encoded with charset
			str += "=\"";
			str += it->second;
			str += "\"; ";
		}
		else
		{
			str += it->second;
			str += "; ";
		}

		++count;
		++it;
	}

	if ( str.size() >= 2 )
		str.resize(str.size()-2);

	return str;
}

std::string&
MIME_Header_Util::trim_quoted_string(std::string& str)
{
	aos::trim(str);
	if ( str.size() > 1 )
	{
		if ( str[0] == '"' && str[0] == str[str.size()-1] )
		{
			aos::QS qs;
			size_t n_decode = qs.decode(str.c_str(), str.size(), (char*) str.c_str());
			str.resize(n_decode);
		}
	}

	return str;
}

std::string&
MIME_Header_Util::trim_quotes(std::string& str, char quote)
{
	aos::trim(str);
	if ( str.size() > 1 )
	{
		if ( str[0] == quote && str[0] == str[str.size()-1] )
		{
			str.erase(str.end()-1, str.end());
			str.erase(str.begin(), str.begin()+1);
		}
	}

	return str;
}

/// MIME_Util

UnicodeString
MIME_Util::to_unicode(const std::string& from, const char* from_charset, UErrorCode& status)
{
	status = U_ZERO_ERROR;
	UConverter* uconv = ucnv_open(from_charset, &status);
	//if ( !U_SUCCESS(status) )
	//	uconv = ucnv_open(NULL, &status);

	//UChar uc = 0x0040;
	//ucnv_setSubstString(uconv, &uc, 1, &status);

	// convert to unicode
	//status = U_ZERO_ERROR;
	UnicodeString ustr(from.c_str(), (::int32_t) from.size(), uconv, status);

	ucnv_close(uconv);

	return ustr;
}

std::string
MIME_Util::from_unicode(const UnicodeString& ustr, const char* to_charset, UErrorCode& status)
{
	status = U_ZERO_ERROR;
	UConverter* uconv = ucnv_open(to_charset, &status);
	std::string str;
	
	// convert from unicode
	if ( U_SUCCESS(status) )
	{
		size_t n_max = ucnv_getMaxCharSize(uconv) * ustr.length() + 1;
		str.resize(n_max);
		size_t n_len = ustr.extract((char*) str.c_str(), (::int32_t) str.size(), uconv, status);
		str.resize(n_len);
	}

	ucnv_close(uconv);

	return str;
}

std::string
MIME_Util::to_utf8(const std::string& from, const char* from_charset, UErrorCode& status)
{
	return MIME_Util::from_unicode(MIME_Util::to_unicode(from, from_charset, status), "utf-8", status);
}

std::string
MIME_Util::from_utf8(const std::string& utf8, const char* to_charset, UErrorCode& status)
{
	return MIME_Util::from_unicode(MIME_Util::to_unicode(utf8, "utf-8", status), to_charset, status);
}

//size_t
//MIME_Util::to_Base64(std::string& str, size_t line_size)
//{
//	aos::Base64 base64(line_size);
//	std::string tmp(str);
//
//	size_t n_enough_encode = base64.enough_encode_size(tmp.size());
//	str.resize(n_enough_encode);
//	size_t n_encode = base64.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
//	str.resize(n_encode);
//
//	return n_encode;
//}

//size_t
//MIME_Util::from_Base64(std::string& str)
//{
//	aos::Base64 base64;
//	size_t n_decode = base64.decode(str.c_str(), str.size(), (char*) str.c_str());
//	str.resize(n_decode);
//
//	return n_decode;
//}

//size_t
//MIME_Util::to_QP(std::string& str, size_t line_size)
//{
//	aos::QP qp(line_size);
//	std::string tmp(str);
//
//	size_t n_enough_encode = qp.enough_encode_size(tmp.size());
//	str.resize(n_enough_encode);
//	size_t n_encode = qp.encode(tmp.c_str(), tmp.size(), (char*) str.c_str());
//	str.resize(n_encode);
//
//	return n_encode;
//}

//size_t
//MIME_Util::from_QP(std::string& str)
//{
//	aos::QP qp;
//	size_t n_decode = qp.decode(str.c_str(), str.size(), (char*) str.c_str());
//	str.resize(n_decode);
//
//	return n_decode;
//}

void
MIME_Util::map_charset(std::string& charset, const char* default_charset)
{
	aos::tolower(charset);
	if ( charset.empty() && default_charset ) charset = default_charset;
	else if ( charset.find("utf-7") != std::string::npos ) charset = "utf-7";
	else if ( charset.find("gb2312") != std::string::npos ) charset = "gbk";
	else if ( charset.find("big5") != std::string::npos ) charset = "big5";
}

int
MIME_Util::check_charset(const char* buf, size_t len, const char* charset)
{
	std::string before(buf, len);

	UErrorCode status = U_ZERO_ERROR;
	UnicodeString ustr = MIME_Util::to_unicode(before, charset, status);
	std::string after = MIME_Util::from_unicode(ustr, charset, status);

	//std::string guess_charset = MIME_Util::guess_charset(buf, len); //@
	
	int n_match = 0;
	const char* longer;
	const char* shorter;
	longer = (after.size() > before.size())?after.c_str():before.c_str();
	shorter = (longer == after.c_str())?before.c_str():after.c_str();

	// compare first for match count
	if ( *longer == *shorter )
		++n_match;

	//// compare all for match count
	//while( *longer && *shorter )
	//{
	//	if ( *longer == *shorter )
	//	{
	//		++longer;
	//		++shorter;
	//		++n_match;
	//	}
	//	else
	//		++longer;
	//}

	return n_match;
}

std::string
MIME_Util::guess_charset(const char* buf, size_t size)
{
	UCharsetDetector* csd;
    const UCharsetMatch* csm;

    UErrorCode status = U_ZERO_ERROR;
	csd = ucsdet_open(&status);
	ucsdet_setText(csd, buf, (::int32_t) size, &status);
    csm = ucsdet_detect(csd, &status);

	if ( !csm || status != U_ZERO_ERROR )
		return "";

	status = U_ZERO_ERROR;
	const char *name = ucsdet_getName(csm, &status);
	//const char *language = ucsdet_getLanguage(csm, &status);
	//int32_t confidence = ucsdet_getConfidence(csm, &status);

	return std::string(name);
}

void
MIME_Util::decode_body(MIME_Entity& e, int process_child)
{
	e.decode_body();
	if ( e.has_child() && process_child )
	{
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			MIME_Util::decode_body(*(*it), process_child);
		}
	}
}

void
MIME_Util::encode_body(MIME_Entity& e, int process_child)
{
	e.encode_body();
	if ( e.has_child() && process_child )
	{
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			MIME_Util::encode_body(*(*it), process_child);
		}
	}
}

ssize_t
MIME_Util::import_file_handle(MIME_Entity& e, ACE_HANDLE fh, int flag, bool no_hidden_header)
{
	bool with_body = (flag == MIME_Entity::Flag::NO_BODY)?false:true;

	if ( fh == ACE_INVALID_HANDLE ) return -1;

	ssize_t n_byte = 0;

	static const int BUF_SIZE = 16384;
	char buf[BUF_SIZE]; // static memory allocation (stack)

	aos::bcstr bstr;
	std::string line; //line.reserve(128);
	
	int n_line = 0; // line count
	char delimiter = 0; // line delimiter

	int parsing = 1;
	while( parsing )
	{
		// read buffer
		ssize_t n_buf = ACE_OS::read(fh, buf, BUF_SIZE);
		if ( n_buf == 0 )
		{
			++n_line; // count for last line
			e.parse_line(line, no_hidden_header, with_body);
			n_byte += (int) line.size();
			// not needed! //sline.assign(line.buf, line.len); ::printf("%d:%s", line.len, sline.c_str()); //@
			break;
		}

		// get line delimiter
		if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, n_buf);

		// get lines
		const char* left = buf;
		size_t n_left = n_buf;
		while( 1 )
		{
			bstr = aos::get_line(left, n_left, delimiter);
			left += bstr.len;
			n_left -= bstr.len;
			if ( bstr.buf[bstr.len-1] == delimiter )
			{
				++n_line; // read one line
				line.append(bstr.buf, bstr.len); 

				e.parse_line(line, no_hidden_header, with_body); //::printf("%d:%s", pstate_, line.c_str()); //@
				
				// check if only parse header AND header completed
				if ( flag == MIME_Entity::Flag::HEADER && e.pstate_ >= MIME_Entity::Parse::HEADER_DONE )
				{
					parsing = 0;
					break;
				}
				n_byte += (int) line.size();

				line.resize(0);
			}
			else
			{
				line.append(bstr.buf, bstr.len); //::printf("%d:%s", line.len, sline.c_str()); //@
			}

			if ( n_left <= 0 )
				break; // buffer consumed, get next buffer!
		} // while( 1 )
	} // while( parsing )

	return n_byte;
}

ssize_t
MIME_Util::import_file(MIME_Entity& e, const char* filename, int flag, bool no_hidden_header)
{
	e.reset();

	ACE_HANDLE fh = ACE_OS::open(filename, O_BINARY | O_RDONLY);
	if ( fh == ACE_INVALID_HANDLE ) return -1;

	ssize_t n_byte = MIME_Util::import_file_handle(e, fh, flag, no_hidden_header);

	ACE_OS::close(fh);

	return n_byte;
}

ssize_t
MIME_Util::export_file_handle(MIME_Entity& e, ACE_HANDLE fh, int flag, bool no_hidden_header)
{
	if ( fh == ACE_INVALID_HANDLE ) return -1;

	ssize_t n_byte = 0;

	if ( !e.header_.empty() )
	{
		for(MIME_Header_List::iterator it = e.header_.begin();
			it != e.header_.end();
			++it)
		{
			if ( no_hidden_header && (*it)->size() && (*(*it))[0] == MIME_Entity::HIDDEN_HEADER_CHAR )
				continue;

			n_byte += ACE_OS::write(fh, (*it)->c_str(), (*it)->size());
		}
	}

	if ( flag == MIME_Entity::Flag::HEADER )
		return n_byte;
	
	if ( e.preamble_ )
	{
		n_byte += ACE_OS::write(fh, e.preamble().c_str(), e.preamble().size());
	}
	
	if ( e.child_.empty() )
	{
		if ( flag != MIME_Entity::Flag::NO_BODY && e.body_ )
			n_byte += ACE_OS::write(fh, e.body().c_str(), e.body().size());
	}
	else
	{
		// dump child
		std::string boundary(e.get_boundary());
		for(MIME_Entity_List::iterator it = e.child_.begin();
			it != e.child_.end();
			++it)
		{
			if ( !boundary.empty() )
			{
				ACE_OS::write(fh, "--", 2);
				ACE_OS::write(fh, boundary.c_str(), boundary.size());
				ACE_OS::write(fh, "\r\n", 2);
			}
			n_byte += MIME_Util::export_file_handle(*(*it), fh, flag, no_hidden_header); // recursive call!
		}
		if ( !boundary.empty() )
		{
			n_byte += ACE_OS::write(fh, "--", 2);
			n_byte += ACE_OS::write(fh, boundary.c_str(), boundary.size());
			n_byte += ACE_OS::write(fh, "--\r\n", 4);
		}
	}

	if ( e.epilogue_ )
	{
		n_byte += ACE_OS::write(fh, e.epilogue().c_str(), e.epilogue().size());
	}

	return n_byte;
}

ssize_t
MIME_Util::export_file(MIME_Entity& e, const char* filename, int flag, bool no_hidden_header)
{
	ACE_HANDLE fh = ACE_OS::open(filename, O_CREAT | O_BINARY | O_WRONLY);
	if ( fh == ACE_INVALID_HANDLE ) return -1;

	ssize_t n_byte = MIME_Util::export_file_handle(e, fh, flag, no_hidden_header);

	//ACE_OFF_T n_byte = ACE_OS::lseek(fh, 0, SEEK_END);
	ACE_OS::close(fh);

	return n_byte;
}

//#include <boost/regex.hpp>
//#include <boost/regex/icu.hpp>
//#include <boost/xpressive/xpressive.hpp>
//using namespace boost;

/*
void
MIME_Header_Util::parse_encoded3(const std::string& str, int trim_crlf, int map_charset)
{
	using namespace boost::xpressive;

	pairs_.clear();
	parser_ = MIME_Header_Util::Encoded_Parser;

	std::string s;
	if ( trim_crlf )
	{
		aos::Tokenizer toker(str.c_str(), str.size());
		toker.set_separator("\r\n");

		int ch = toker.next();
		while( ch > aos::Tokenizer::End )
		{
			const char* buf = toker.token();
			size_t len = toker.size();
			if ( len > 0 && ( *buf == ' ' || *buf == '\t' ) )
			{
				++buf;
				--len;
			}
			s.append(buf, len);

			ch = toker.next();
		}
	}
	else
	{
		s.assign(str);
	}

	sregex re = sregex::compile("=\\?([^?]+)\\?(B|Q)\\?([^?]+)\\?=", boost::xpressive::regex_constants::icase );
	//static const regex re("=\\?([^?]+)\\?(B|Q)\\?([^?]+)\\?=", regex::icase);
	boost::xpressive::smatch match;

	std::string::const_iterator start = s.begin();
	std::string::const_iterator end = s.end();

	char enc = '\0';
	std::string charset;
	std::string text;
	while( regex_search(start, end, match, re) )
	{
		text.assign(start, match[0].first);
		if ( !text.empty() )
		{
			// text without charset will be merged into the last pair
			if ( !pairs_.empty() )
			{
				Pairs::iterator last = --(pairs_.end());
				last->second += text;
			}
			else
				pairs_.push_back(std::make_pair("", text));
		}
		
		charset.assign(match[1].first, match[1].second);
		enc = *(match[2].first);
		text.assign(match[3].first, match[3].second);

		if ( map_charset ) MIME_Util::map_charset(charset);

		switch(enc)
		{
			case 'B':
			case 'b':
				{
				aos::Base64 base64;
				size_t n_decode = base64.decode(text.c_str(), text.size(), (char*) text.c_str());
				text.resize(n_decode);
				break;
				}
			case 'Q':
			case 'q':
				{
				//RFC2047: 4.2. The "Q" encoding (2)
				//replace '_' with ' '
				size_t n_len = text.size();
				char* cstr = (char*) text.c_str();
				for(size_t i=0; i < n_len; ++i)
				{
					if ( *(cstr+i) == '_' ) *(cstr+i) = ' ';
				}

				aos::QP qp;
				size_t n_decode = qp.decode(text.c_str(), text.size(), (char*) text.c_str());
				text.resize(n_decode);
				break;
				}
			default:
				break;
		}

		// if the same charset as the last one, append text to the last one
		// else new entry
		if ( !pairs_.empty() )
		{
			Pairs::iterator last = --(pairs_.end());
			if ( aos::strnicmp(last->first.c_str(), charset.c_str(), charset.size()) == 0 )
				last->second += text;
			else
				pairs_.push_back(std::make_pair(charset, text));
		}
		else
			pairs_.push_back(std::make_pair(charset, text));

		start = match[0].second;
	}
	text.assign(start, end);
	if ( !text.empty() )
	{
		// text without charset will be merged into the last pair
		if ( !pairs_.empty() )
		{
			Pairs::iterator last = --(pairs_.end());
			last->second += text;
		}
		else
			pairs_.push_back(std::make_pair("", text));
	}
}
//*/

/*
void
MIME_Header_Util::parse_encoded2(const std::string& str, int trim_crlf, int map_charset)
{
	pairs_.clear();
	parser_ = MIME_Header_Util::Encoded_Parser;

	std::string s;
	if ( trim_crlf )
	{
		aos::Tokenizer toker(str.c_str(), str.size());
		toker.set_separator("\r\n");

		int ch = toker.next();
		while( ch > aos::Tokenizer::End )
		{
			const char* buf = toker.token();
			size_t len = toker.size();
			if ( len > 0 && ( *buf == ' ' || *buf == '\t' ) )
			{
				++buf;
				--len;
			}
			s.append(buf, len);

			ch = toker.next();
		}
	}
	else
	{
		s.assign(str);
	}

	static const regex re("=\\?([^?]+)\\?(B|Q)\\?([^?]+)\\?=", regex::icase);
	smatch match;

	std::string::const_iterator start = s.begin();
	std::string::const_iterator end = s.end();

	char enc = '\0';
	std::string charset;
	std::string text;
	while( regex_search(start, end, match, re) )
	{
		text.assign(start, match[0].first);
		if ( !text.empty() )
		{
			// text without charset will be merged into the last pair
			if ( !pairs_.empty() )
			{
				Pairs::iterator last = --(pairs_.end());
				last->second += text;
			}
			else
				pairs_.push_back(std::make_pair("", text));
		}
		
		charset.assign(match[1].first, match[1].second);
		enc = *(match[2].first);
		text.assign(match[3].first, match[3].second);

		if ( map_charset ) MIME_Util::map_charset(charset);

		switch(enc)
		{
			case 'B':
			case 'b':
				{
				aos::Base64 base64;
				size_t n_decode = base64.decode(text.c_str(), text.size(), (char*) text.c_str());
				text.resize(n_decode);
				break;
				}
			case 'Q':
			case 'q':
				{
				//RFC2047: 4.2. The "Q" encoding (2)
				//replace '_' with ' '
				size_t n_len = text.size();
				char* cstr = (char*) text.c_str();
				for(size_t i=0; i < n_len; ++i)
				{
					if ( *(cstr+i) == '_' ) *(cstr+i) = ' ';
				}

				aos::QP qp;
				size_t n_decode = qp.decode(text.c_str(), text.size(), (char*) text.c_str());
				text.resize(n_decode);
				break;
				}
			default:
				break;
		}

		// if the same charset as the last one, append text to the last one
		// else new entry
		if ( !pairs_.empty() )
		{
			Pairs::iterator last = --(pairs_.end());
			if ( aos::strnicmp(last->first.c_str(), charset.c_str(), charset.size()) == 0 )
				last->second += text;
			else
				pairs_.push_back(std::make_pair(charset, text));
		}
		else
			pairs_.push_back(std::make_pair(charset, text));

		start = match[0].second;
	}
	text.assign(start, end);
	if ( !text.empty() )
	{
		// text without charset will be merged into the last pair
		if ( !pairs_.empty() )
		{
			Pairs::iterator last = --(pairs_.end());
			last->second += text;
		}
		else
			pairs_.push_back(std::make_pair("", text));
	}
}
//*/

/*
void
MIME_Header_Util::parse_address2(const std::string& str)
{
	pairs_.clear();
	parser_ = MIME_Header_Util::Address_Parser;

	int ch;
	aos::Tokenizer toker;
	toker.str(str.c_str(), str.size());
	toker.set_separator(",\r\n");

	while(true)
	{
		ch = toker.next();
		if ( ch <= aos::Tokenizer::End ) break;
		std::string address(toker.token(), toker.size());

		static const regex re_addr("(.*)(<.*@.*>)", regex::icase);
		smatch match;
		std::string::const_iterator start = address.begin();
		std::string::const_iterator end = address.end();
		if ( regex_search(start, end, match, re_addr) )
		{
			std::string name(match[1].first, match[1].second);
			//aos::trim(name);
			trim_quoted_string(name);
			std::string email(match[2].first, match[2].second);

			pairs_.push_back(std::make_pair(name, email));
		}
		else
		{
			pairs_.push_back(std::make_pair("", address));
		}
	}
}
//*/

MIME_Date::MIME_Date()
{
	reset();
}

MIME_Date::MIME_Date(const char* cstr)
{
	reset();
	this->from_string(cstr);
}

MIME_Date::MIME_Date(const std::string& str)
{
	reset();
	this->from_string(str);
}

MIME_Date::~MIME_Date()
{
}

void
MIME_Date::reset()
{
	// reset tm
	tm.tm_hour = -1;
	tm.tm_isdst = -1;
	tm.tm_mday = -1;
	tm.tm_min = -1;
	tm.tm_mon = -1;
	tm.tm_sec = -1;
	tm.tm_wday = -1;
	tm.tm_yday = -1;
	tm.tm_year = -1;

	// reset tz
	tm_gmtoff = -1;
}

int
MIME_Date::from_string(const char* cstr)
{
	static const char* day[7] = {
		"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday"
	}; // (first:SMTWF) (len:3-9) (last:y)
	static const char* abday[7] = {
		"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
	}; // (last:neduit)
	static const char* mon[12] = {
		"January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November", "December"
	}; // (first:JFMASOND + TW) (len:3-9) (last:yhletr)
	static const char* abmon[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	}; // (last:nbrylgptvc + he + dui) 
	static const char* am_pm[2] = {
		"AM", "PM"
	};

	reset();
	int count = 0; // # of item parsed
	Multi_String mstr;

	// tokenizer keep tokens with "0123456789" + "JFMASOND" + "TW" + "+-" && len >=3 && len <= 10
	aos::Tokenizer toker(cstr);
	toker.set_separator(" ,;\t\v\r\n");
	aos::Char_Map cm("JFMASONDTW+-");

	int ch = toker.next();
	int i = 0;
	while (ch > aos::Tokenizer::End) {
		char ch0 = ::toupper(*(toker.token()));
		if ( ch0 >= '0' && ch0 <= '9' )
			mstr.push_back(toker.token(), toker.size());
		if ( cm.has(ch0) && toker.size() >=3 && toker.size() <= 9 )
			mstr.push_back(toker.token(), toker.size());

		ch = toker.next();
		++i;
	}
	int n = (int) mstr.size();

	// find YYYY-MM-DD first // (len:<=10)
	for(int i = 0; i < n && tm.tm_year < 0; ++i) {
		if (!::isdigit(*(mstr[i])) || mstr.size(i) < 10)
			continue;

		Multi_String substr;
		substr.explode('-', mstr[i]);
		if ( substr.size() != 3 )
			continue;

		do {
			int year = ACE_OS::atoi(substr[0]);
			if (year < 1900 || year > 9999)
				break;

			if (!::isdigit(*(substr[1])))
				break;
			int month = ACE_OS::atoi(substr[1]);
			if (month < 1 || month > 12)
				break;

			if (!::isdigit(*(substr[2])))
				break;
			int day = ACE_OS::atoi(substr[2]);
			if (day < 1 || day > 31)
				break;

			tm.tm_year = year-1900;
			tm.tm_mon = month-1;
			tm.tm_mday = day;
			*((char*) mstr[i]) = '%'; // mark as parsed
			count += 3;
		} while (0);
	}

	// find HH:MM:SS or HH:MM
	for(int i = 0; i < n && tm.tm_hour < 0; ++i) {
		if (!::isdigit(*(mstr[i])) || mstr.size(i) < 5)
			continue;

		Multi_String substr;
		substr.explode(':', mstr[i]);
		if (!(substr.size() == 3 || substr.size() == 2))
			continue;

		do {
			int hour = ACE_OS::atoi(substr[0]);
			if (hour < 0 || hour > 23)
				break;

			if (!::isdigit(*(substr[1])))
				break;
			int minute = ACE_OS::atoi(substr[1]);
			if (minute < 0 || minute > 59)
				break;

			if (substr.size() == 3)
			{
				if ( !::isdigit(*(substr[2])) )
					break;
				int second = ACE_OS::atoi(substr[2]);
				if ( second < 0 || second > 59 ) // some say 0-61 to accommodate for leap seconds in certain systems.
					break;
				tm.tm_sec = second;
			}
			else
			{
				tm.tm_sec = 0;
			}
			tm.tm_min = minute;
			tm.tm_hour = hour;
			*((char*) mstr[i]) = '%'; // mark as parsed
			count += 3;
		} while(0);
	}

	// if no weekday, find it
	if ( tm.tm_wday == -1 ) {
		for(int i = 0; i < n; ++i) {
			if (!::isalpha(*(mstr[i])))
				continue;
			// if token_size == 3 check abbrev weekday name, else check full weekday name
			if (mstr.size(i) == 3) {
				for(int w = 0; w < 7; ++w) {
					if (ACE_OS::strcasecmp(mstr[i], abday[w]) == 0) {
						tm.tm_wday = w;
						*((char*) mstr[i]) = '%'; // mark as parsed
						break;
					}
				}
			}
			else {
				for(int w = 0; w < 7; ++w) {
					if (ACE_OS::strcasecmp(mstr[i], day[w]) == 0) {
						tm.tm_wday = w;
						*((char*) mstr[i]) = '%'; // mark as parsed
						break;
					}
				}
			}
		}	
	}

	// if tm_mon == -1, find month
	if ( tm.tm_mon == -1 ) {
		// find month
		int month_pos = -1;
		for(int i = 0; i < n; ++i) {
			if (!::isalpha(*(mstr[i])))
				continue;
			// if token_size == 3 check abbrev month name, else check full month name
			if (mstr.size(i) == 3) {
				for(int m = 0; m < 12; ++m) {
					if (ACE_OS::strcasecmp(mstr[i], abmon[m]) == 0) {
						tm.tm_mon = m;
						*((char*) mstr[i]) = '%'; // mark as parsed
						++count;
						month_pos = i;
						break;
					}
				}
			}
			else {
				for(int m = 0; m < 12; ++m) {
					if (ACE_OS::strcasecmp(mstr[i], mon[m]) == 0) {
						tm.tm_mon = m;
						*((char*) mstr[i]) = '%'; // mark as parsed
						++count;
						month_pos = i;
						break;
					}
				}
			}
		}
		// if month found
		if (month_pos != -1) {
			// check month_pos + 1 for year or day
			int pos = month_pos + 1;
			if (pos < n) {
				int val = ACE_OS::atoi(mstr[pos]);
				if (val >= 1900 && val <= 9999) {
					tm.tm_year = val-1900;
					*((char*) mstr[pos]) = '%'; // mark as parsed
					++count;
				}
				else if (val >= 1 && val <= 31) {
					tm.tm_mday = val;
					*((char*) mstr[pos]) = '%'; // mark as parsed
					++count;
				}
			}
			// check month_pos - 1 for year or day
			pos = month_pos - 1;
			if ( pos >= 0 ) {
				int val = ACE_OS::atoi(mstr[pos]);
				if (val >=1 && val <= 31 && tm.tm_year != -1) {
					tm.tm_mday = val;
					*((char*) mstr[pos]) = '%'; // mark as parsed
					++count;
				}
				else if (val >= 1900 && val <= 9999 && tm.tm_mday != -1) {
					tm.tm_year = val-1900;
					*((char*) mstr[pos]) = '%'; // mark as parsed
					++count;
				}
			}
			// check month_pos + 2 for year or day
			if (tm.tm_year == -1 || tm.tm_mday == -1) {
				pos = month_pos + 2;
				if (pos < n) {
					int val = ACE_OS::atoi(mstr[pos]);
					if (val >=1 && val <= 31 && tm.tm_year != -1) {
						tm.tm_mday = val;
						*((char*) mstr[pos]) = '%'; // mark as parsed
						++count;
					}
					else if (val >= 1900 && val <= 9999 && tm.tm_mday != -1) {
						tm.tm_year = val-1900;
						*((char*) mstr[pos]) = '%'; // mark as parsed
						++count;
					}
				}
				// check month_pos - 2 for year or day
				if (tm.tm_year == -1 || tm.tm_mday == -1) {
					pos = month_pos - 2;
					if ( pos >= 0 ) {
						int val = ACE_OS::atoi(mstr[pos]);
						if (val >=1 && val <= 31 && tm.tm_year != -1) {
							tm.tm_mday = val;
							*((char*) mstr[pos]) = '%'; // mark as parsed
							++count;
						}
						else if (val >= 1900 && val <= 9999 && tm.tm_mday != -1) {
							tm.tm_year = val-1900;
							*((char*) mstr[pos]) = '%'; // mark as parsed
							++count;
						}
					}
				} // month_pos - 2
			} // month_pos + 2
		} // if (month_pos != -1)
	} // find month

	// if tm_year == -1, find year
	if ( tm.tm_year == -1 ) {
		for(int i = 0; i < n; ++i) {
			if ( mstr.size(i) != 4 )
				continue;
			int year = ACE_OS::atoi(mstr[i]);
			if ( year >= 1900 && year <= 9999 ) {
				tm.tm_year = year-1900;
				*((char*) mstr[i]) = '%'; // mark as parsed
				++count;
				break;
			}
		}
	}

	// if tm_mday == -1, find day
	if ( tm.tm_mday == -1 ) {
		for(int i = 0; i < n; ++i) {
			if ( mstr.size(i) != 2 )
				continue;
			int day = ACE_OS::atoi(mstr[i]);
			if ( day > 12 && day <= 31 ) {
				tm.tm_mday = day;
				*((char*) mstr[i]) = '%'; // mark as parsed
				++count;
				break;
			}
		}
	}

	// if no tz, find timezone
	if ( tm_gmtoff == -1 ) {
		for(int i = 0; i < n; ++i) {
			if ( *mstr[i] == '+' || *mstr[i] == '-' ) {
				Multi_String substr;
				substr.explode(':', mstr[i]);

				if ( substr.size() > 2 )
					continue;
				
				if ( substr.size() ) {
					size_t n_substr0 = substr.size(0);
					if ( n_substr0 < 2 || !::isdigit(*(substr[0]+1)) )
						continue;

					char hms[9]; // HH:MM:SS
					ACE_OS::memset(hms, 0, 9);

					int v1 = ACE_OS::atoi(substr[0]);
					if ( substr.size() == 1 && v1 >= -1400 && v1 <= 1400 ) {
						if ( n_substr0 >= 2 ) {
							hms[0] = *(substr[0]+1);
							hms[1] = *(substr[0]+2);
						}
						if ( n_substr0 >= 4 ) {
							hms[3] = *(substr[0]+3);
							hms[4] = *(substr[0]+4);
						}
						tm_gmtoff = ACE_OS::atoi(hms) * 3600;
						tm_gmtoff += ACE_OS::atoi(hms+3) * 60;
						if ( *mstr[i] == '+' ) tm_gmtoff = 0 - tm_gmtoff;
						*((char*) mstr[i]) = '%'; // mark as parsed
						++count;
						break;
					}
					else if ( substr.size() == 2 ) {
						size_t n_substr1 = substr.size(1);
						if ( !::isdigit(*substr[1]) )
							continue;

						int v2 = ACE_OS::atoi(substr[1]);
						if ( v1 >= -14 && v1 <= 14 && v2 >= 0 && v2 <= 59 ) {
							if ( n_substr0 >= 2 ) {
								hms[0] = *(substr[0]+1);
								hms[1] = *(substr[0]+2);
							}
							if ( n_substr1 >= 2 ) {
								hms[3] = *(substr[1]);
								hms[4] = *(substr[1]+1);
							}
							tm_gmtoff = ACE_OS::atoi(hms) * 3600;
							tm_gmtoff += ACE_OS::atoi(hms+3) * 60;
							if ( *mstr[i] == '+' ) tm_gmtoff = 0 - tm_gmtoff;
							*((char*) mstr[i]) = '%'; // mark as parsed
							++count;
							break;
						}
					}
				}
			} // if ( '+' || '-' )
		} // for
	}

	//mstr.dump(); //@

	return (count - 7); // YYYY-MM-DD HH:MM:SS TZ
}

std::string
MIME_Date::to_string(const char* format)
{
	if ( !format ) format = "%Y-%m-%d %H:%M:%S";

	std::string str;

	static const int BUF_MAX = 4096;
	char buf[BUF_MAX+1];
	size_t len = ACE_OS::strftime(buf, BUF_MAX, format, &tm);
	if ( len > 0 ) {
		str.assign(buf, len);
	}

	return str;
}

time_t
MIME_Date::gmt_mktime(int apply_tz)
{
	time_t time = ACE_OS::mktime(&tm); // mktime take tm as localtime // or use mkgmtime(&tm) instead
	time -= ACE_OS::timezone(); // adjust to GMT time

	if ( apply_tz ) time += tm_gmtoff; // apply timezone offset

	return time;

	/* example:
	MIME_Date date;
	date.from_string("Date: Sat, 30 Apr 2011 16:19:01 +0800");
	time_t time = date.gmt_mktime();
	ACE_OS::printf("AS_LOCAL: %s", ACE_OS::ctime(&time));
	ACE_OS::printf("AS_GMT+0: %s", ACE_OS::asctime(ACE_OS::gmtime(&time)));
	//*/
}

} // namespace aos
