#include "SBRS_Server_IO.h"

SBRS_Server_IO::SBRS_Server_IO()
{
}

SBRS_Server_IO::~SBRS_Server_IO()
{
}

aos::bcstr
SBRS_Server_IO::read_request(ACE_Message_Block& mb)
{
	if ( rd_state_ != RD_MORE ) buf_.resize(0);
	rd_state_ = RD_MORE;

	const char* delimiter = "\r\n\r\n";
	size_t n_delimiter = ACE_OS::strlen(delimiter);

	aos::bcstr data;
	data.buf = mb.rd_ptr();
	data.len = mb.length();

	if ( data.len > 0 )
	{
		buf_.append(data.buf, data.len);
		mb.rd_ptr(data.len);
	}
	if ( buf_.size() >= n_delimiter &&
		ACE_OS::strncmp(buf_.c_str() + buf_.size() - n_delimiter, delimiter, n_delimiter) == 0 )
	{
		rd_state_ = RD_OK;
	}

	return data;
}

int
SBRS_Server_IO::write_response(ACE_Message_Block& mb, const char* msg)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%s\r\n\r\n", msg));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SBRS_Server_IO::write_score(ACE_Message_Block& mb, double score)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%f\r\n\r\n", score));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SBRS_Server_IO::parse_cmd(aos::Multi_String& params, const char* buf, size_t len)
{
	int rc = -1; // or Command::NULL

	params.clear();

	aos::Tokenizer toker(buf, len); // buf_.c_str(), buf_.size()
	toker.set_separator(" \r\n");
	int ch = toker.next();

	if ( toker.size() )
	{
		// store command as params_[0]
		params.push_back(toker.token(), toker.size());

		if ( ACE_OS::strncasecmp(params[0], "HELO", 4) == 0 )
		{
			// parse HELO parameters here!
			const char* ptr = toker.token_end();
			const char* ptr_end = toker.str_end();
			while( ptr < ptr_end )
			{
				if ( *ptr != ' ' ) break;
				++ptr;
			}
			const char* host = ptr;
			while( ptr < ptr_end )
			{
				if ( *ptr == '\r' || *ptr == '\n' ) break;
				++ptr;
			}
			if ( ptr > host )
			{
				params.push_back(host, ptr-host);
				rc = 0; // or Command::HELO
				//::printf("host:%s\n", params_[1]); //@
			}
		}
		else if ( ACE_OS::strncasecmp(params[0], "MAIL", 4) == 0 )
		{
			// parse MAIL FROM parameters here!
			const char* ptr = toker.token_end();
			if ( ACE_OS::strncasecmp(ptr, " FROM:", 6) == 0 )
			{
				ptr += 6;
				const char* ptr_end = toker.str_end();
				const char* str = ptr;
				while( ptr < ptr_end )
				{
					if ( *ptr == '\r' || *ptr == '\n' ) break;
					++ptr;
				}
				if ( ptr > str )
				{
					std::string email(str, ptr-str);
					aos::trim(email, " <>");
					if ( !email.empty() )
					{
						params.push_back(email.c_str(), email.size());
						rc = 0; // or Command::MAIL
						//::printf("%sFrom:%s\n", buf_.c_str(), params[1]); //@
					}
				}
			}
		}
		else if ( ACE_OS::strncasecmp(params[0], "RCPT", 4) == 0 )
		{
			// parse RCPT TO parameters here!
			const char* ptr = toker.token_end();
			if ( ACE_OS::strncasecmp(ptr, " TO:", 4) == 0 )
			{
				ptr += 4;
				const char* ptr_end = toker.str_end();
				const char* str = ptr;
				while( ptr < ptr_end )
				{
					if ( *ptr == '\r' || *ptr == '\n' ) break;
					++ptr;
				}
				if ( ptr > str )
				{
					std::string email(str, ptr-str);
					aos::trim(email, " <>");
					if ( !email.empty() )
					{
						params.push_back(email.c_str(), email.size());
						rc = 0; // or Command::RCPT
						//::printf("%sTo:%s\n", buf_.c_str(), params[1]); //@
					}
				}
			}
		}
		else
		{
			// default command parser
			rc = 0;
		}
	}
	else
	{
		// null command!
	}

	return rc;
}
