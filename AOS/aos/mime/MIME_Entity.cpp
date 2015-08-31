#include "aos/mime/MIME_Entity.h"

#include "aos/String.h"
#include "aos/Codec.h"

//#include <boost/regex.hpp>
//using namespace boost;

namespace aos {

const std::string
MIME_Entity::NULL_STRING(""); // MIME_Entity::NULL_STRING = "";

MIME_Entity::MIME_Entity()
:
preamble_(0),
body_(0),
epilogue_(0),
parent_(0),
status_(0),
pstate_(Parse::HEADER)
{
}

MIME_Entity::~MIME_Entity()
{
	this->fini_();
}

void
MIME_Entity::init_()
{
}

void
MIME_Entity::fini_()
{
	this->clear_header();
	delete preamble_; preamble_ = 0;
	delete body_; body_ = 0;
	delete epilogue_; epilogue_ = 0;
	this->clear_child();
	this->parent_ = 0;
	this->status_ = 0;

	this->pstate_ = Parse::HEADER;
	this->separator_.resize(0);
}

void
MIME_Entity::reset()
{
	fini_();
};

void
MIME_Entity::clear_header()
{
	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
		delete *it; // delete 0 is ok in c++
	header_.clear();
}

MIME_Header_List::iterator
MIME_Entity::find_header(const char* name, MIME_Header_List::iterator pos, int case_sensitive)
{
	MIME_Header_List::iterator it = header_.end();
	std::string find_me(name);
	if ( find_me.empty() ) return it;
	find_me += ':';
	
	if ( case_sensitive )
	{
		for(it = pos; it != header_.end(); ++it)
		{
			if ( ::strncmp((*it)->c_str(), find_me.c_str(), find_me.size()) == 0 )
				break;
		}
	}
	else
	{
		for(it = pos; it != header_.end(); ++it)
		{
			if ( aos::strnicmp((*it)->c_str(), find_me.c_str(), find_me.size()) == 0 )
				break;
		}
	}

	return it;
}

void
MIME_Entity::add_first_header(std::string* field)
{
	if ( field )
		header_.push_front(field);
}

void
MIME_Entity::add_last_header(std::string* field)
{
	if ( field )
	{
		MIME_Header_List::reverse_iterator rit = header_.rbegin();
		for(; rit != header_.rend(); ++rit)
		{
			if ( !(*rit)->empty() && (*(*rit))[0] != '\r' && (*rit)->operator[](0) != '\n' ) {
				break;
			}
		}
		add_header(field, rit.base()); //? if ( rit != header_.rend() )
	}
}

int
MIME_Entity::is_header_completed()
{
	if ( this->empty_header() ) return 0;

	char ch;
	MIME_Header_List::reverse_iterator it = header_.rbegin();
	if ( (*(*it)).size() > 2 ) return 0;
	ch = (*(*it))[0];

	return ( ch == '\r' || ch == '\n' );
}

std::string
MIME_Entity::get_parameter(const std::string& header, const char* name)
{
	size_t colon = header.find(':');
	if ( colon == std::string::npos ) return "";

	const char* buf = header.c_str()+colon+1;
	size_t len = header.size()-colon-1;

	int ch;
	aos::Tokenizer toker(buf, len);
	toker.set_separator(";\r\n");
	ch = toker.next();

	std::string key(header, 0, colon+1); // include ':'
	std::string val(toker.token(), toker.size());

	if ( aos::stricmp(name, key.c_str()) == 0 )
	{
		aos::trim(val);
		if ( val.size() > 1 && val[0] == '"' && val[0] == val[val.size()-1] )
			aos::trim(val, '"');
		return val;
	}

	while( ch > aos::Tokenizer::End )
	{
		// get key
		toker.set_separator("=\r\n");
		ch = toker.next();
		key.assign(toker.token(), toker.size());
		aos::trim(key);

		// get value
		toker.set_separator(";\r\n");
		ch = toker.next();
		val.assign(toker.token(), toker.size());

		if ( aos::stricmp(name, key.c_str()) == 0 )
		{
			aos::trim(val);
			if ( val.size() > 1 && val[0] == '"' && val[0] == val[val.size()-1] )
				aos::trim(val, '"');
			return val;
		}
	}

	return "";
}

std::string
MIME_Entity::get_content_type()
{
	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-type:", 13) == 0 )
		{
			return get_parameter(*(*it), "content-type:");
		}
	}

	return "";
}

std::string
MIME_Entity::get_primary_type()
{
	std::string str(get_content_type());
	std::string::size_type pos = str.find_first_of('/');
	if ( pos != std::string::npos )
		str.erase(pos, std::string::npos);

	return str;
}

std::string
MIME_Entity::get_sub_type()
{
	std::string str(get_content_type());
	std::string::size_type pos = str.find_last_of('/');
	if ( pos != std::string::npos )
		str.erase(0, pos+1);
	else
		str.clear();

	return str;
}

std::string
MIME_Entity::get_boundary()
{
	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-type:", 13) == 0 )
		{
			return get_parameter(*(*it), "boundary");
		}
	}

	return "";
}

std::string
MIME_Entity::get_charset()
{
	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-type:", 13) == 0 )
		{
			return get_parameter(*(*it), "charset");
		}
	}

	return "";
}

std::string
MIME_Entity::get_encoding()
{
	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-transfer-encoding:", 26) == 0 )
		{
			return get_parameter(*(*it), "content-transfer-encoding:");
		}
	}

	return "";
}

int
MIME_Entity::set_charset(const char* charset)
{
	bool has_ctype = false;
	bool has_ctype_charset = false;

	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-type:", 13) == 0 )
		{
			has_ctype = true;
			std::string lcase(*(*it));
			aos::tolower(lcase);
			
			size_t pos_beg = lcase.find("charset=");
			if ( pos_beg != std::string::npos )
			{
				has_ctype_charset = true;
				pos_beg += 8; //::strlen("charset=")
			}
			
			if ( has_ctype_charset )
			{
				size_t pos_end = lcase.find_first_of(";\r\n", pos_beg);
				if ( pos_end != std::string::npos )
					(*it)->replace(pos_beg, pos_end-pos_beg, charset);
				else
				{
					// append to tail
					(*it)->replace(pos_beg, lcase.size()-pos_beg, charset);
					(*it)->append(";\r\n");
				}
			}
			else
			{
				size_t pos = (*it)->find_last_not_of("\r\n");
				if ( pos != std::string::npos )
				{
					std::string cs;
					if ( (*(*it))[pos] != ';' && (*(*it))[pos] != ':' )
					{
						cs += ';';
					}
					cs += " charset=";
					cs += charset;
					cs += ';';

					(*it)->insert(pos+1, cs);
				}
				else
				{
					std::string cs;
					if ( (*(*it))[pos] != ';' && (*(*it))[pos] != ':' )
					{
						cs += ';';
					}
					cs += " charset=";
					cs += charset;
					cs += ";\r\n";

					(*it)->append(cs);
				}
			}

			/*
			static const regex re("charset=(\\S+)(;|\\s)", regex::icase);
			
			smatch match;
			std::string::const_iterator start = (*it)->begin();
			std::string::const_iterator end = (*it)->end();

			if ( regex_search(start, end, match, re) )
			{
				std::string replace;
				
				replace.assign(start, match[1].first);
				replace.append(charset);
				replace.append(match[2].first, match[2].second);
				replace.append(match[0].second, end);
				//::printf("replace: %s(%d)\n", replace.c_str(), replace.size()); //@
				
				(*it)->assign(replace);
			}
			else
			{
				// insert new charset after "Content-Type:"
				std::string::size_type pos = (*it)->find(';');
				if ( pos == std::string::npos )
				{
					pos = (*it)->find('\r');
					if ( pos != std::string::npos ) (*it)->insert(pos, ";");
				}
				if ( pos == std::string::npos )
				{
					pos = (*it)->find('\n');
					if ( pos != std::string::npos ) (*it)->insert(pos, ";");
				}

				std::string new_cs(" charset=");
				new_cs += charset;
				new_cs += ";";

				(*it)->insert(pos+1, new_cs);
			}
			//*/

			//::printf("set_charset: %s\n", (*it)->c_str()); //@

			break;
		}
	}

	if ( !has_ctype )
	{
		// append to tail
		std::string* hdr = new (std::nothrow) std::string("Content-Type: charset=");
		*hdr += charset;
		*hdr += ";\r\n";
		this->add_last_header(hdr);
	}

	return 0;
}

int
MIME_Entity::set_encoding(const char* encoding)
{
	bool is_encoding_found = false;

	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-transfer-encoding:", 26) == 0 )
		{
			(*it)->assign("Content-Transfer-Encoding: ");
			(*it)->append(encoding);
			(*it)->append("\r\n");

			is_encoding_found = true;
			break;
		}
	}

	if ( is_encoding_found == false )
	{
		std::string* hdr = new (std::nothrow) std::string("Content-Transfer-Encoding: ");
		*hdr += encoding;
		*hdr += "\r\n";
		this->head_header(hdr);
	}

	return 0;
}

// parse with decode_body() has very little impact to performance
// in a test with 7621 * < 100K eml, time spent without/with decode_body() is 1700ms v.s. 1800ms
int
MIME_Entity::decode_body()
{
	// if empty body ,or already decoded, return 0 (OK);
	if ( this->body_ == 0 || this->body().empty() || (this->status_ & Status::BODY_ENCODED) == 0 ) return 0;

	std::string encoding = this->get_encoding();
	if ( aos::strnicmp(encoding.c_str(), "base64", 6) == 0 )
	{
		aos::Base64 base64;
		size_t n_decode = base64.decode((this->body()).c_str(), (this->body()).size(), (char*) (this->body()).c_str());
		this->body().resize(n_decode);
		if ( this->status_ & Status::BODY_ENCODED ) this->status_ ^= Status::BODY_ENCODED; // erase BODY_ENCODED bit
	}
	else if ( aos::strnicmp(encoding.c_str(), "quoted-printable", 16) == 0 )
	{
		aos::QP qp;
		size_t n_decode = qp.decode((this->body()).c_str(), (this->body()).size(), (char*) (this->body()).c_str());
		this->body().resize(n_decode);
		if ( this->status_ & Status::BODY_ENCODED ) this->status_ ^= Status::BODY_ENCODED; // erase BODY_ENCODED bit
	}
	else
	{
		// default for 7bit/8bit/binary encoding
		if ( this->status_ & Status::BODY_ENCODED ) this->status_ ^= Status::BODY_ENCODED; // erase BODY_ENCODED bit
	}

	return 0;
}

int
MIME_Entity::encode_body()
{
	// if empty body ,or already encoded, return 0 (OK);
	if ( this->body_ == 0 || this->body().empty() || (this->status_ & Status::BODY_ENCODED) ) return 0;

	// encode with Content-Transfer-Encoding
	std::string encoding = this->get_encoding();
	if ( aos::strnicmp(encoding.c_str(), "base64", 6) == 0 )
	{
		aos::Base64 base64;
		std::string tmp(this->body()); // body copy

		size_t n_enough_encode = base64.enough_encode_size(tmp.size());
		this->body().resize(n_enough_encode);
		size_t n_encode = base64.encode(tmp.c_str(), tmp.size(), (char*) (this->body()).c_str());
		this->body().resize(n_encode);
		this->body() += "\r\n";

		this->status_ |= Status::BODY_ENCODED; // set BODY_ENCODED bit
	}
	else if ( aos::strnicmp(encoding.c_str(), "quoted-printable", 16) == 0 )
	{
		aos::QP qp;
		std::string tmp(this->body()); // body copy

		size_t n_enough_encode = qp.enough_encode_size(tmp.size());
		this->body().resize(n_enough_encode);
		size_t n_encode = qp.encode(tmp.c_str(), tmp.size(), (char*) (this->body()).c_str());
		this->body().resize(n_encode);
		//this->body() += "\r\n";

		this->status_ |= Status::BODY_ENCODED; // set BODY_ENCODED bit
	}
	else
	{
		// default for 7bit/8bit/binary encoding, or anything else
		this->status_ |= Status::BODY_ENCODED; // set BODY_ENCODED bit
	}

	return 0;
}

void
MIME_Entity::copy_decoded_body(std::string& str)
{
	str.resize(0);
	// if empty body, just return
	if ( this->body_ == 0 || this->body().empty() ) return;

	// already decoded, just copy;
	if ( (this->status_ & Status::BODY_ENCODED) == 0 )
	{
		str.assign(this->body());
	}
	else
	{
		std::string encoding = this->get_encoding();
		if ( aos::strnicmp(encoding.c_str(), "base64", 6) == 0 )
		{
			str.resize((this->body()).size());

			aos::Base64 base64;
			size_t n_decode = base64.decode((this->body()).c_str(), (this->body()).size(), (char*) str.c_str());
			str.resize(n_decode);
		}
		else if ( aos::strnicmp(encoding.c_str(), "quoted-printable", 16) == 0 )
		{
			str.resize((this->body()).size());

			aos::QP qp;
			size_t n_decode = qp.decode((this->body()).c_str(), (this->body()).size(), (char*) str.c_str());
			str.resize(n_decode);
		}
		else
		{
			// default for 7bit/8bit/binary encoding
			str.assign(this->body());
		}
	}
}

void
MIME_Entity::copy_encoded_body(std::string& str)
{
	str.resize(0);
	// if empty body, just return
	if ( this->body_ == 0 || this->body().empty() ) return;

	// already encoded, just copy
	if ( this->status_ & Status::BODY_ENCODED )
	{
		str.assign(this->body());
	}
	else
	{
		std::string encoding = this->get_encoding();
		if ( aos::strnicmp(encoding.c_str(), "base64", 6) == 0 )
		{
			aos::Base64 base64;
			size_t n_enough_encode = base64.enough_encode_size((this->body()).size());
			str.resize(n_enough_encode);
			size_t n_encode = base64.encode((this->body()).c_str(), (this->body()).size(), (char*) str.c_str());
			str.resize(n_encode);
			str += "\r\n";
		}
		else if ( aos::strnicmp(encoding.c_str(), "quoted-printable", 16) == 0 )
		{
			aos::QP qp;
			size_t n_enough_encode = qp.enough_encode_size((this->body()).size());
			str.resize(n_enough_encode);
			size_t n_encode = qp.encode((this->body()).c_str(), (this->body()).size(), (char*) str.c_str());
			str.resize(n_encode);
			//str += "\r\n";
		}
		else
		{
			// default for 7bit/8bit/binary encoding, or anything else
			str.assign(this->body());
		}
	}
}

void
MIME_Entity::clear_child()
{
	for(MIME_Entity_List::iterator it = child_.begin();
		it != child_.end();
		++it)
		delete *it; // delete 0 is ok in c++
	child_.clear();
}

size_t
MIME_Entity::size(bool no_hidden_header)
{
	size_t n_total = 0;

	// sizeof header_
	for(MIME_Header_List::const_iterator it = header_.begin();
	it != header_.end();
	++it)
	{
		if ( no_hidden_header && (*it)->size() && (*(*it))[0] == HIDDEN_HEADER_CHAR )
			continue;
		
		n_total += (*it)->size();
	}

	// sizeof preamble_
	if ( preamble_ ) n_total += preamble_->size();

	// sizeof body_
	if ( body_ ) n_total += body_->size();

	// sizeof child_
	if ( has_child() )
	{
		size_t n_boundary = get_boundary().size();
		for(MIME_Entity_List::const_iterator it = child_.begin();
		it != child_.end();
		++it)
		{
			if ( n_boundary ) n_total += n_boundary + 2 + 2; // prefix("--") & CRLF
			n_total += (*it)->size();
		}
		if ( n_boundary ) n_total += n_boundary + 4 + 2; // prefix("--") & suffix("--") & CRLF
	}

	// sizeof preamble_
	if ( epilogue_ ) n_total += epilogue_->size();

	return n_total;
}

void
MIME_Entity::dump_header()
{
	if ( has_header() )
	{
		// dump header
		::printf("[HEADER]\n");
		for(MIME_Header_List::iterator it = header_.begin();
			it != header_.end();
			++it)
		{
			::printf("%s", (*it)->c_str());
		}
		::printf("[/HEADER]\n");
	}
}

void
MIME_Entity::dump_body()
{
	if ( child_.empty() )
	{
		//// dump body
		::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", this->get_content_type().c_str(), this->get_encoding().c_str(), this->get_charset().c_str());
		if ( this->get_primary_type() == "text" )
		{
		//getchar();
		int res = this->decode_body();
		if ( res == 0 ) ::printf("%s", body().c_str());
		}
		//::printf("[/BODY]\n");
	}
	else
	{
		// dump child
		size_t n_boundary = get_boundary().size();
		AOS_UNUSE(n_boundary); //@
		for(MIME_Entity_List::iterator it = child_.begin();
			it != child_.end();
			++it)
		{
			//if ( n_boundary ) ::printf("--%s\n", get_boundary().c_str());
			(*it)->dump_body(); // recursive call!
		}
		//if ( n_boundary ) ::printf("--%s--\n", get_boundary().c_str());
	}
}

void
MIME_Entity::dump()
{
	if ( has_header() )
	{
		// dump header
		::printf("[HEADER]\n");
		for(MIME_Header_List::iterator it = header_.begin();
			it != header_.end();
			++it)
		{
			::printf("%s", (*it)->c_str());
		}
		::printf("[/HEADER]\n");
	}

	if ( preamble_ )
	{
		// dump preamble
		::printf("[PREAMBLE]\n");
		::printf("%s", preamble().c_str());
		::printf("[/PREAMBLE]\n");
	}
	
	if ( child_.empty() )
	{
		//// dump body
		::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", this->get_content_type().c_str(), this->get_encoding().c_str(), this->get_charset().c_str());
		::printf("%s", body().c_str());
		::printf("[/BODY]\n");
		//getchar();
	}
	else
	{
		// dump child
		size_t n_boundary = get_boundary().size();
		for(MIME_Entity_List::iterator it = child_.begin();
			it != child_.end();
			++it)
		{
			 if ( n_boundary ) ::printf("--%s\n", get_boundary().c_str());
			(*it)->dump(); // recursive call!
		}
		if ( n_boundary ) ::printf("--%s--\n", get_boundary().c_str());
	}

	if ( epilogue_ )
	{
		// dump epilogue
		::printf("[EPILOGUE]\n");
		::printf("%s", epilogue().c_str());
		::printf("[/EPILOGUE]\n");
	}
}

int
MIME_Entity::parse_line(std::string& line, bool no_hidden_header, bool with_body)
{
	if ( pstate_ < Parse::HEADER_DONE )
	{
		pstate_ = parse_header_line(line, no_hidden_header);
		if ( pstate_ == Parse::HEADER_DONE )
		{
			// on header done
		}
	}
	if ( pstate_ >= Parse::HEADER_DONE )
	{
		pstate_ = parse_content_line(line, no_hidden_header, with_body);
	}

	return pstate_;
}

int
MIME_Entity::parse_header_line(std::string& line, bool no_hidden_header)
{
	if ( pstate_ == Parse::HEADER )
	{
		// in-header
		if ( line[0] == '\r' || line[0] == '\n' )
		{
			// HEADER_END;
			pstate_ = Parse::HEADER_CRLF;
			separator_ = this->get_boundary();
			if ( !separator_.empty() ) separator_ = "--" + separator_;

			if ( this->get_content_type() == "message/rfc822" )
			{
				MIME_Entity* entity = new (std::nothrow) MIME_Entity();
				this->append_child(entity);
			}

			this->append_header(new (std::nothrow) std::string(line));
		}
		else if ( line[0] == '\t' || line[0] == ' ')
		{
			// HEADER_CONT;
			if ( this->has_header() )
			{
				MIME_Header_List::reverse_iterator it = this->header().rbegin();
				(*it)->append(line);
			}
		}
		else
		{
			// HEADER_NEW;
			if ( !no_hidden_header || line[0] != HIDDEN_HEADER_CHAR )
				this->append_header(new (std::nothrow) std::string(line));
		}
	}
	else if ( pstate_ == Parse::HEADER_CRLF && ( line[0] == '\r' || line[0] == '\n' ) )
	{
		pstate_ = Parse::HEADER_CRLF;
		this->append_header(new (std::nothrow) std::string(line));
	}
	else
	{
		pstate_ = Parse::HEADER_DONE;
		status_ |= Status::BODY_ENCODED;
	}

	return pstate_;
}

int
MIME_Entity::parse_content_line(std::string& line, bool no_hidden_header, bool with_body)
{
	// multipart
	if ( !separator_.empty() )
	{
		// is boundary
		if ( strncmp(line.c_str(), separator_.c_str(), separator_.size()) == 0 )
		{
			// is boundary end
			size_t bsize = separator_.size();
			if ( line.size() >= bsize + 2 && line[bsize] == line[bsize+1] && line[bsize] == '-' )
			{
				// boundary end
				pstate_ = Parse::BOUNDARY_END;
			}

			// mark between-boundary
			if ( pstate_ == Parse::HEADER_DONE )
			{
				pstate_ = Parse::BOUNDARY;
			}

			// boundary start
			if ( pstate_ == Parse::BOUNDARY )
			{
				MIME_Entity* entity = new (std::nothrow) MIME_Entity();
				this->append_child(entity);
			}

			return pstate_;
		}
		// before boundary
		if ( pstate_ == Parse::HEADER_DONE )
		{
			this->preamble() += line;
		}
		// after boundary
		else if ( pstate_ == Parse::BOUNDARY_END )
		{
			this->epilogue() += line;
		}
		// between boundary
		else
		{
			if ( !this->empty_child() )
			{
				MIME_Entity_List::reverse_iterator it = this->child().rbegin();
				(*(*it)).parse_line(line, no_hidden_header, with_body);
			}
		}
	}
	// message
	else if ( this->get_content_type() == "message/rfc822" )
	{
		if ( !this->empty_child() )
		{
			MIME_Entity_List::reverse_iterator it = this->child().rbegin();
			(*(*it)).parse_line(line, no_hidden_header, with_body);
		}
	}
	// non-multipart
	else
	{
		if ( with_body )
			this->body() += line;
	}

	return pstate_;
}

int
MIME_Entity::import_data(const char* buf, size_t len, int flag, bool no_hidden_header)
{
	this->reset();
	bool with_body = (flag == Flag::NO_BODY)?false:true;

	int n_byte = 0;

	aos::bcstr bstr;
	std::string line; //line.reserve(4096);
	
	int c = 0; // line count
	char delimiter = 0; // line delimiter

	// get line delimiter
	if ( !delimiter ) delimiter = aos::get_line_delimiter(buf, len);

	// get lines
	const char* left = buf;
	size_t n_left = len;

	int parsing = 1;
	while( parsing )
	{
		bstr = aos::get_line(left, n_left, delimiter);
		left += bstr.len;
		n_left -= bstr.len;
		if ( bstr.buf[bstr.len-1] == delimiter )
		{
			++c; // read one line

			line.append(bstr.buf, bstr.len);
			
			this->parse_line(line, no_hidden_header, with_body); //::printf("%d:%s", line.len, sline.c_str()); //@

			// check if only parse header AND header completed
			if ( flag == Flag::HEADER && pstate_ >= Parse::HEADER_DONE )
			{
				parsing = 0;
				break;
			}
			n_byte += (int) line.size();

			line.resize(0);

			//if ( c % 200000 == 0 ) ::printf("read:%d\n", c);
		}
		else
		{
			line.append(bstr.buf, bstr.len); //::printf("%d:%s", line.len, sline.c_str()); //@
		}

		if ( n_left <= 0 )
			break; // buffer consumed, get next buffer!
	}

	++c; // count for last line
	this->parse_line(line, no_hidden_header, with_body);
	n_byte += (int) line.size();

	return n_byte;
}

int
MIME_Entity::import_file(const char* filename, int flag, bool no_hidden_header)
{
	this->reset();
	bool with_body = (flag == Flag::NO_BODY)?false:true;

	FILE* fp = ::fopen(filename, "rb");
	if ( !fp ) return -1;

	int n_byte = 0;

	static const int BUF_SIZE = 4096;
	char buf[BUF_SIZE]; // static memory allocation (stack)

	aos::bcstr bstr;
	std::string line; // line.reserve(4096);
	
	int c = 0; // line count
	char delimiter = 0; // line delimiter

	int parsing = 1;
	while( parsing )
	{
		// read buffer
		size_t n_buf = ::fread(buf, sizeof(char), BUF_SIZE, fp);
		if ( n_buf == 0 )
		{
			++c; // count for last line
			this->parse_line(line, no_hidden_header, with_body);
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
				++c; // read one line

				line.append(bstr.buf, bstr.len); 

				this->parse_line(line, no_hidden_header, with_body); //::printf("%d:%s", pstate_, line.c_str()); //@

				// check if only parse header AND header completed
				if ( flag == Flag::HEADER && pstate_ >= Parse::HEADER_DONE )
				{
					parsing = 0;
					break;
				}
				n_byte += (int) line.size();

				line.resize(0);

				//if ( c % 200000 == 0 ) ::printf("read:%d\n", c);
			}
			else
			{
				line.append(bstr.buf, bstr.len); //::printf("%d:%s", line.len, sline.c_str()); //@
			}

			if ( n_left <= 0 )
				break; // buffer consumed, get next buffer!
		}

	}

	::fclose(fp);

	//::printf("\ntotal lines:%d\n", c);
	//this->dump();
	//::printf("total_bytes:%d\n", this->size());

	return n_byte;
}

int
MIME_Entity::dump_data(std::string& data, int flag, bool no_hidden_header)
{
	int n_byte = (int) data.size();

	if ( has_header() )
	{
		for(MIME_Header_List::iterator it = header_.begin();
			it != header_.end();
			++it)
		{
			if ( no_hidden_header && (*it)->size() && (*(*it))[0] == HIDDEN_HEADER_CHAR )
				continue;
			
			data.append((*it)->c_str(), (*it)->size());
		}
	}

	if ( flag == Flag::HEADER )
		return ((int) data.size() - n_byte);

	if ( preamble_ )
	{
		data.append(preamble().c_str(), preamble().size());
	}
	
	if ( child_.empty() )
	{
		if ( flag != Flag::NO_BODY )
			data.append(body().c_str(), body().size());
	}
	else
	{
		// dump child
		size_t n_boundary = get_boundary().size();
		for(MIME_Entity_List::iterator it = child_.begin();
			it != child_.end();
			++it)
		{
			if ( n_boundary )
			{
				data.append("--", 2);
				data.append(get_boundary().c_str(), get_boundary().size());
				data.append("\r\n", 2);
			}
			(*it)->dump_data(data); // recursive call!
		}
		if ( n_boundary )
		{
			data.append("--", 2);
			data.append(get_boundary().c_str(), get_boundary().size());
			data.append("--\r\n", 4);
		}
	}

	if ( epilogue_ )
	{
		data.append(epilogue().c_str(), epilogue().size());
	}

	return ((int) data.size() - n_byte);
}

int
MIME_Entity::export_data(std::string& data, int flag, bool no_hidden_header)
{
	data.resize(0);
	int n_byte = this->dump_data(data, flag, no_hidden_header);

	return n_byte;
}

int
MIME_Entity::dump_file(FILE* fp, int flag, bool no_hidden_header)
{
	if ( !fp ) return -1;

	int n_byte = 0;

	if ( has_header() )
	{
		for(MIME_Header_List::iterator it = header_.begin();
			it != header_.end();
			++it)
		{
			if ( no_hidden_header && (*it)->size() && (*(*it))[0] == HIDDEN_HEADER_CHAR )
				continue;

			n_byte += (int) ::fwrite((*it)->c_str(), (*it)->size(), 1, fp);
		}
	}

	if ( flag == Flag::HEADER )
		return n_byte;
	
	if ( preamble_ )
	{
		n_byte += (int) ::fwrite(preamble().c_str(), preamble().size(), 1, fp);
	}
	
	if ( child_.empty() )
	{
		if ( flag != Flag::NO_BODY )
			n_byte += (int) ::fwrite(body().c_str(), body().size(), 1, fp);
	}
	else
	{
		// dump child
		size_t n_boundary = get_boundary().size();
		for(MIME_Entity_List::iterator it = child_.begin();
			it != child_.end();
			++it)
		{
			if ( n_boundary )
			{
				::fwrite("--", 2, 1, fp);
				::fwrite(get_boundary().c_str(), get_boundary().size(), 1, fp);
				::fwrite("\r\n", 2, 1, fp);
			}
			n_byte += (int) (*it)->dump_file(fp, flag, no_hidden_header); // recursive call!
		}
		if ( n_boundary )
		{
			n_byte += (int) ::fwrite("--", 2, 1, fp);
			n_byte += (int) ::fwrite(get_boundary().c_str(), get_boundary().size(), 1, fp);
			n_byte += (int) ::fwrite("--\r\n", 4, 1, fp);
		}
	}

	if ( epilogue_ )
	{
		n_byte += (int) ::fwrite(epilogue().c_str(), epilogue().size(), 1, fp);
	}

	return n_byte;
}

int
MIME_Entity::export_file(const char* filename, int flag, bool no_hidden_header)
{
	FILE* fp = ::fopen(filename, "wb");
	if ( !fp ) return -1;

	int n_byte = this->dump_file(fp, flag, no_hidden_header);

	//int n_byte = ::ftell(fp);
	::fclose(fp);

	return n_byte;
}

} // namespace aos
