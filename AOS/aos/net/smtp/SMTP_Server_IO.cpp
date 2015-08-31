#include "SMTP_Server_IO.h"

SMTP_Server_IO::SMTP_Server_IO()
{
}

SMTP_Server_IO::~SMTP_Server_IO()
{
}

aos::bcstr
SMTP_Server_IO::read_line(ACE_Message_Block& mb, size_t max_size)
{
	if ( rd_state_ != RD_MORE ) buf_.resize(0);
	rd_state_ = RD_MORE;

	char delimiter = '\n';
	aos::bcstr line;

	line = aos::get_line(mb.rd_ptr(), (int) mb.length(), delimiter);
	if ( line.len > 0 )
	{
		buf_.append(line.buf, line.len);
		mb.rd_ptr(line.len);

		if ( line.buf[line.len-1] == delimiter || ( max_size && buf_.size() > max_size ) )
		{
			// one completed line read
			line.buf = (char*) buf_.c_str();
			line.len = buf_.size();
			rd_state_ = RD_OK;
		}
		else
		{
			// incompleted line
			rd_state_ = RD_MORE;
		}
	}

	return line;
}

aos::bcstr
SMTP_Server_IO::read_data(ACE_Message_Block& mb)
{
	aos::bcstr data;
	data.buf = mb.rd_ptr();
	data.len = 0;


	if ( state_ == State::DATA )
	{
		data.len = mb.length();
		if ( data.len >= 5 )
		{
			buf_.assign(mb.rd_ptr()+data.len-5, 5);
		}
		else
		{
			buf_.append(mb.rd_ptr(), data.len);
		}
		mb.rd_ptr(data.len);
		rd_byte_ += data.len; // increase read size counter

		// data end with <CR><LF>.<CR><LF>
		if ( buf_.size() >= 5 && ACE_OS::strncmp(buf_.c_str()+buf_.size()-5, "\r\n.\r\n", 5) == 0 )
		{
			rd_state_ = RD_OK;
		}
	}

	return data;
}

int
SMTP_Server_IO::parse_cmd(aos::Multi_String& params, const char* buf, size_t len)
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

int
SMTP_Server_IO::exec_cmd(ACE_Message_Block& mb)
{
	parse_cmd(params_, buf_.c_str(), buf_.size());

	int rc = 0;
	if ( state_ != State::DATA )
	{
		if ( params_.size() )
		{
			if ( ACE_OS::strncasecmp(params_[0], "HELO", 4) == 0 )
			{
				rc = helo(mb);
			}
			else if ( ACE_OS::strncasecmp(params_[0], "MAIL", 4) == 0 )
			{
				rc = mail(mb);
			}
			else if ( ACE_OS::strncasecmp(params_[0], "RCPT", 4) == 0 )
			{
				rc = rcpt(mb);
			}
			else if ( ACE_OS::strncasecmp(params_[0], "DATA", 4) == 0 )
			{
				rc = data(mb);
			}
			else if ( ACE_OS::strncasecmp(params_[0], "NOOP", 4) == 0 )
			{
				rc = noop(mb);
			}
			else if ( ACE_OS::strncasecmp(params_[0], "QUIT", 4) == 0 )
			{
				rc = quit(mb);
			}
			else
			{
				// unknown command!
				rc = write_line(mb, 502, "Error: command not recognized");
			}
		}
		else
		{
			// null command!
			rc = write_line(mb, 500, "Error: bad syntax");
		}
	}
	else
	{
		// DATA completed
		rc = data_completed(mb);
	}

	return rc;
}

int
SMTP_Server_IO::write_line(ACE_Message_Block& mb, int code, const char* msg)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%d %s\r\n", code, msg));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::greetings(ACE_Message_Block& mb, const char* greetings)
{
	static const char* msg_fmt = "220 %s\r\n";

	mb.reset();
	if ( greetings )
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), msg_fmt, greetings));
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "220 domain.com ESMTP product_name\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::helo(ACE_Message_Block& mb)
{
	mb.reset();

	if ( params_.size() >= 2 )
	{
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 domain.com\r\n"));
		state_ = State::RESET; // state changed
	}
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "501 Syntax: HELO hostname\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::mail(ACE_Message_Block& mb)
{
	// if ( state_ != State::RESET ) error!

	mb.reset();

	if ( state_ == State::RESET && params_.size() >= 2 )
	{
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 OK\r\n"));
		state_ = State::MAIL; // state changed
		emails_.clear(); // clear any previous addr
		emails_.push_back(params_[1]); // store "MAIL FROM:" addr
	}
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "501 Syntax: MAIL FROM\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::rcpt(ACE_Message_Block& mb)
{
	// if ( state_ != State::MAIL ) error!

	mb.reset();

	if ( state_ == State::MAIL && params_.size() >= 2 )
	{
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 OK\r\n"));
		state_ = State::MAIL; // state changed
		emails_.push_back(params_[1]); // store "RCPT TO:" addr
	}
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "501 Syntax: RCPT TO\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::data(ACE_Message_Block& mb)
{
	// if ( state_ != State::MAIL ) error!

	mb.reset();

	if ( state_ == State::MAIL && params_.size() >= 1 )
	{
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "354 End data with <CR><LF>.<CR><LF>\r\n"));
		state_ = State::DATA; // state changed
		rd_byte_ = 0; // reset read size counter
	}
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "501 Syntax: DATA\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::data_completed(ACE_Message_Block& mb)
{
	mb.reset();

	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 OK: queued as <mail_id>\r\n"));
	state_ = State::RESET; // state changed

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::noop(ACE_Message_Block& mb)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 OK\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::rset(ACE_Message_Block& mb)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "250 OK\r\n"));
	state_ = State::RESET; // state changed

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return 0;
}

int
SMTP_Server_IO::quit(ACE_Message_Block& mb)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "221 BYE\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;
	else wr_state_ = WR_MORE;

	return -1; // close connection
}

/// Stream command

int
SMTP_Server_IO::greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	//ssize_t n_recv = -1;

	//// read greetings
	//this->read_reset();
	//while( this->read_state() != IMAP4_Client_IO::RD_OK )
	//{
	//	if ( mb.length() == 0 )
	//	{
	//		mb.reset();
	//		n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
	//		if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
	//			break;
	//		if ( n_recv > 0 ) mb.wr_ptr(n_recv);
	//	}
	//	if ( mb.length() > 0 )
	//	{
	//		aos::bcstr line = this->read_greeting(mb);
	//		ACE_OS::printf("%s", this->buf().c_str());
	//	}
	//}

	return 0;
}
