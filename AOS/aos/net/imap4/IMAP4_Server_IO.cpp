#include "IMAP4_Server_IO.h"

IMAP4_Server_IO::IMAP4_Server_IO()
:
state_(State::NOT_AUTH)
{
}

IMAP4_Server_IO::~IMAP4_Server_IO()
{
}

aos::bcstr
IMAP4_Server_IO::read_cmd(ACE_Message_Block& mb)
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

		if ( line.buf[line.len-1] == delimiter )
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

int
IMAP4_Server_IO::exec_cmd(ACE_Message_Block& mb)
{
	parse_cmd(params_, buf_.c_str(), buf_.size());
	buf_.resize(0);
	params_.dump();

	int rc = 0;

	if ( params_.count() < 2 ) // missing arguments
	{
		rc = bad_cmd(mb);
		return rc;
	}

	if ( ACE_OS::strcmp(params_[1], "NOOP") == 0 )
	{
		rc = noop(mb);
	}
	else if ( ACE_OS::strcmp(params_[1], "CAPABILITY") == 0 )
	{
		rc = capability(mb);
	}
	else if ( ACE_OS::strcmp(params_[1], "LOGOUT") == 0 )
	{
		rc = logout(mb);
	}
	else if ( ACE_OS::strcmp(params_[1], "LOGIN") == 0 && params_.count() == 4 )
	{
		rc = login(mb, params_[2], params_[3]);
		if ( rc == 0 ) 	state_ = State::AUTH;
	}
	else if ( ACE_OS::strcmp(params_[1], "LIST") == 0 && params_.count() == 4 )
	{
		if ( state_ == State::AUTH )
		{
			rc = list(mb, params_[2], params_[3]);
		}
		else
		{
			rc = bad_cmd(mb);
		}
	}
	else
	{
		rc = bad_cmd(mb);
	}

	// should return 0 for ok, other for error! e.g. parsed fail, missing arguments
	return rc;
}

int
IMAP4_Server_IO::greetings(ACE_Message_Block& mb)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "* OK ready\r\n"));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 0;
}

int
IMAP4_Server_IO::login(ACE_Message_Block& mb, const char* user, const char* pass)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%s OK LOGIN completed\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 0;
}

int
IMAP4_Server_IO::logout(ACE_Message_Block& mb)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%s OK LOGOUT completed\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return -1; // -1 for close connection
}

int
IMAP4_Server_IO::noop(ACE_Message_Block& mb)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%s OK NOOP completed\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 0;
}

int
IMAP4_Server_IO::capability(ACE_Message_Block& mb)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "* CAPABILITY imap4rev1\r\n%s OK CAPABILITY completed\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 0;
}

int
IMAP4_Server_IO::list(ACE_Message_Block& mb, const char* param1, const char* param2)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "* LIST (\\HasNoChildren) \"\\\" \"INBOX\"\r\n%s OK LIST completed\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 0;
}

int
IMAP4_Server_IO::bad_cmd(ACE_Message_Block& mb)
{
	wr_state_ = WR_MORE;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "%s BAD bad command\r\n", tag()));

	if ( mb.space() > 0 ) wr_state_ = WR_OK;

	return 1; // 1 for error code #1
}

int
IMAP4_Server_IO::parse_cmd(aos::Multi_String& params, const char* buf, size_t len)
{
	params.clear();
	aos::QS qs;

	const char* left = buf;
	size_t n_left = len;
	char delimiter = ' ';
	while( n_left > 0 )
	{
		if ( left[0] == qs.quote() )
		{
			size_t qlen = qs.decode(left, n_left, (char*) left);
			if ( qs.state() == aos::QS::DECODE_OK && n_left > 0 )
			{
				params.push_back(left, qlen);
				// escape next character ' ' || '\r' || '\n'
				++left;
				--n_left;
			}

			left += qs.decode_read();
			n_left -= qs.decode_read();
		}
		else
		{
			aos::bcstr token = aos::get_line(left, n_left, delimiter);
			left += token.len;
			n_left -= token.len;

			if ( token.len > 1 )
			{
				if ( token.buf[token.len-1] == delimiter )
					params.push_back(token.buf, token.len-1); // the last char is ' ' || '\n'
				else
					params.push_back(token.buf, token.len-2); // the last char is '\r'
					
			}
		}
	}
	
	return 0;
}

