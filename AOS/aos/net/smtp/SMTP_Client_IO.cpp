#include "SMTP_Client_IO.h"

#include "aos/Codec.h"


SMTP_Client_IO::SMTP_Client_IO()
{
}

SMTP_Client_IO::~SMTP_Client_IO()
{
}

aos::bcstr
SMTP_Client_IO::read_line(ACE_Message_Block& mb, size_t max_size)
{
	if ( rd_state_ != RD_MORE ) buf_.resize(0);
	rd_state_ = RD_MORE;

	char delimiter = '\n';
	aos::bcstr line;
	line.len = 0;

	if ( mb.length() > 0 )
	{
		buf_.append(mb.rd_ptr(), mb.length());
		mb.rd_ptr(mb.length());

		if ( buf_[buf_.size()-1] == delimiter || ( max_size && buf_.size() > max_size ) )
		{
			// read '\n' or oversized
			rd_state_ = RD_OK;

			if ( buf_[buf_.size()-1] == delimiter )
			{
				// test if it's multi-line response
				size_t n_first = 0;
				while( buf_[n_first] >= 0 && buf_[n_first] <= 9 )
					++n_first;
				if ( buf_[n_first] == '-' )
				{
					// last line starts with digits plus '-', read more
					size_t n_last = buf_.rfind(delimiter, buf_.size()-1);
					if ( n_last != std::string::npos )
					{
						++n_last;
						while( buf_[n_last] >= 0 && buf_[n_last] <= 9 )
							++n_last;
						if ( buf_[n_last] == '-' )
							rd_state_ = RD_MORE;
					}
				}
			}

			if ( rd_state_ == RD_OK )
			{
				line.buf = (char*) buf_.c_str();
				line.len = buf_.size();
			}
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
SMTP_Client_IO::parse_cmd_response()
{
	//SMTP reply code
	//1xx connection ERR
	//2xx OK
	//3xx DATA
	//4xx ERR (temp)
	//5xx ERR (perm)
	return ACE_OS::atoi(buf_.c_str());
}

int
SMTP_Client_IO::cmd_helo(ACE_Message_Block& mb, const char* host)
{
	static const char* cmd_fmt = "HELO %s\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, host));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_mail_from(ACE_Message_Block& mb, const char* email, ACE_INT64 size)
{
	//static const char* cmd_fmt = "MAIL FROM:<%s>\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "MAIL FROM:<%s>", email));
	if ( size >= 0 ) mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), " SIZE=%lld", size));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_rcpt_to(ACE_Message_Block& mb, const char* email)
{
	static const char* cmd_fmt = "RCPT TO:<%s>\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, email));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_data(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "DATA \r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_data_completed(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "\r\n.\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_noop(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "NOOP \r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_rset(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "RSET \r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_quit(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "QUIT \r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}


int
SMTP_Client_IO::cmd_ehlo(ACE_Message_Block& mb, const char* host)
{
	static const char* cmd_fmt = "EHLO %s\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, host));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_starttls(ACE_Message_Block& mb)
{
	static const char* cmd_fmt = "STARTTLS \r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_auth(ACE_Message_Block& mb, const char* scheme, const char* arg)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "AUTH %s", scheme));
	if ( arg ) mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), " %s", arg));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::cmd_bdat(ACE_Message_Block& mb, size_t size, bool last)
{
	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "BDAT %u", size));
	if ( last ) mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), " %s", "LAST"));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
SMTP_Client_IO::greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_recv = -1;

	/// read greetings
	int n_io = -1;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::helo(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* host)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_helo(mb, host) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::ehlo(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* host)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_ehlo(mb, host) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::starttls(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_starttls(mb) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::mail_from(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* email, ACE_INT64 size)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_mail_from(mb, email, size) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::rcpt_to(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* email)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_rcpt_to(mb, email) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_data(mb) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::data_completed(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_data_completed(mb) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::rset(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	n_send = ( this->cmd_rset(mb) == SMTP_Client_IO::WR_OK )?stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout):-1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::noop(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	n_send = ( this->cmd_noop(mb) == SMTP_Client_IO::WR_OK )?stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout):-1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::quit(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	n_send = ( this->cmd_quit(mb) == SMTP_Client_IO::WR_OK )?stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout):-1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::auth(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* scheme, const char* arg)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write command
	int n_io = -1;
	if ( this->cmd_auth(mb, scheme, arg) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io;

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io;

	return parse_cmd_response();
}

int
SMTP_Client_IO::auth_login(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* user, const char* pass)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;
	int rc = 0;

	// write AUTH command
	int n_io = -1;
	if ( this->cmd_auth(mb, "LOGIN") == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io; // send() or write buffer error!

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io; // recv() error!

	rc = parse_cmd_response();

	// write USER
	--n_io;
	std::string usr(user);
	aos::Base64::encode(usr);
	usr += "\r\n";

	n_send = stream.send_n(usr.c_str(), usr.size(), flags, timeout);
	if ( n_send < 1 )
		return n_io; // send() error!

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io; // recv() error!

	rc = parse_cmd_response();

	// write PASS
	--n_io;
	std::string pwd(pass);
	aos::Base64::encode(pwd);
	pwd += "\r\n";

	n_send = stream.send_n(pwd.c_str(), pwd.size(), flags, timeout);
	if ( n_send < 1 )
		return n_io; // send() error!

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io; // recv() error!

	rc = parse_cmd_response();

	return rc;
}

int
SMTP_Client_IO::bdat(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const void* buf, size_t len, bool last)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// write BDAT command
	int n_io = -1;
	if ( this->cmd_bdat(mb, len, last) == SMTP_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
		n_send = -1;
	if ( n_send < 1 )
		return n_io; // send() or write buffer error!

	// write data
	--n_io;
	n_send = write_data(stream, flags, timeout, buf, len);
	if ( n_send < 1 )
		return n_io; // send() error!

	// read response
	--n_io;
	this->read_reset(mb);
	while( this->read_state() != SMTP_Client_IO::RD_OK )
	{
		// buffer consumed, read more...
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			else
				mb.wr_ptr(n_recv);
		}
		// process buffer
		if ( mb.length() > 0 )
			aos::bcstr line = this->read_line(mb);
	}
	if ( n_recv < 1 )
		return n_io; // recv() error!

	return parse_cmd_response();
}

