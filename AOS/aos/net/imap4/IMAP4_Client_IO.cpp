#include "IMAP4_Client_IO.h"

IMAP4_Client_IO::IMAP4_Client_IO()
{
	read_reset();
}

IMAP4_Client_IO::~IMAP4_Client_IO()
{
}

aos::bcstr
IMAP4_Client_IO::read_greeting(ACE_Message_Block& mb)
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

aos::bcstr
IMAP4_Client_IO::read_line(ACE_Message_Block& mb)
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
			rd_state_ = RD_LINE;

			if ( line.buf[0] != '*' )
			{
				// no more line
				rd_state_ = RD_OK;
			}
			else
			{
				// has data to read
				std::string number = buf_.substr(buf_.find_last_of('{')+1);
				int n_byte = ACE_OS::atoi(number.c_str());
				if ( n_byte > 0 )
				{
					rd_byte_ = n_byte;
					rd_state_ = RD_DATA;
				}
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

aos::bcstr
IMAP4_Client_IO::read_data(ACE_Message_Block& mb)
{
	aos::bcstr data;
	data.buf = mb.rd_ptr();
	data.len = 0;

	if ( rd_state_ == RD_DATA )
	{
		if ( rd_byte_ > 0 )
		{
			data.buf = mb.rd_ptr();
			data.len = (rd_byte_>mb.length())?mb.length():rd_byte_;
			rd_byte_ -= data.len;
			mb.rd_ptr(data.len);
		}
		if ( mb.length() > 0 )
		{
			char delimiter = '\n';
			aos::bcstr line;

			line = aos::get_line(mb.rd_ptr(), (int) mb.length(), delimiter);
			mb.rd_ptr(line.len);
			if ( line.buf[line.len-1] == delimiter )
			{
				rd_state_ = RD_DATA_OK;
			}
		}
	}

	return data;
}

int
IMAP4_Client_IO::parse_cmd_response()
{
	size_t pos = buf_.find(tag_);
	if ( pos == std::string::npos ) return UNKNOWN;
	pos += tag_.size();
	pos = buf_.find_first_not_of(" ", pos);
	if ( pos == std::string::npos ) return UNKNOWN;

	const char* status = buf_.c_str() + pos;
	if ( ACE_OS::strncmp(status, "OK", 2) == 0 )
		return OK;
	else if ( ACE_OS::strncmp(status, "NO", 2) == 0 )
		return NO;
	else if ( ACE_OS::strncmp(status, "BAD", 3) == 0 )
		return BAD;

	return UNKNOWN;
}

int
IMAP4_Client_IO::cmd_login(ACE_Message_Block& mb, const char* user, const char* pass)
{
	const char* cmd_fmt = "%s LOGIN %s %s\r\n";

	mb.reset();
	int n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), user, pass);
	mb.wr_ptr(n);

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_logout(ACE_Message_Block& mb)
{
	const char* cmd_fmt = "%s LOGOUT\r\n";

	mb.reset();
	int n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str());
	mb.wr_ptr(n);

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_noop(ACE_Message_Block& mb)
{
	const char* cmd_fmt = "%s NOOP\r\n";

	mb.reset();
	int n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str());
	mb.wr_ptr(n);

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_capability(ACE_Message_Block& mb)
{
	const char* cmd_fmt = "%s CAPABILITY\r\n";

	mb.reset();
	int n = ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str());
	mb.wr_ptr(n);

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_list(ACE_Message_Block& mb, const char* reference, const char* mbox)
{
	const char* cmd_fmt = "%s LIST %s %s\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), reference, mbox));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_select(ACE_Message_Block& mb, const char* mbox)
{
	static const char* cmd_fmt = "%s SELECT %s\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), mbox));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_create(ACE_Message_Block& mb, const char* mbox)
{
	static const char* cmd_fmt = "%s CREATE %s\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), mbox));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_append(ACE_Message_Block& mb, const char* mbox, size_t n_byte)
{
	const char* cmd_fmt = "%s APPEND %s {%d}\r\n"; // can be static

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), mbox, n_byte));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

int
IMAP4_Client_IO::cmd_fetch(ACE_Message_Block& mb, const char* sequence, const char* item)
{
	const char* cmd_fmt = "%s FETCH %s %s\r\n";

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), cmd_fmt, tag_.c_str(), sequence, item));

	if ( mb.space() > 0 ) return WR_OK;

	return WR_MORE;
}

/// Stream command

int
IMAP4_Client_IO::greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_recv = -1;

	// read greetings
	this->read_reset();
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_greeting(mb);
			ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return 0;
}

int
IMAP4_Client_IO::login(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* user, const char* pass)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send login command
	if ( this->cmd_login(mb, user, pass) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read login response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return parse_cmd_response();
}

int
IMAP4_Client_IO::logout(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send logout command
	if ( this->cmd_logout(mb) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read logout response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return 0;
}

int
IMAP4_Client_IO::capability(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send logout command
	if ( this->cmd_capability(mb) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read logout response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return 0;
}

int
IMAP4_Client_IO::list(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* reference, const char* mbox)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send command
	if ( this->cmd_list(mb, reference, mbox) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return 0;
}

int
IMAP4_Client_IO::select(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send command
	if ( this->cmd_select(mb, mbox) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return parse_cmd_response();
}

int
IMAP4_Client_IO::create(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send command
	if ( this->cmd_create(mb, mbox) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return parse_cmd_response();
}

int
IMAP4_Client_IO::append_from_file(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox, const char* filename)
{
	ACE_HANDLE fh = ACE_OS::open(filename, O_BINARY | O_RDONLY);
	if ( fh == ACE_INVALID_HANDLE )
	{
		ACE_OS::printf("cannot open file!\n");
		return -1;
	}
	size_t fsize = (size_t) ACE_OS::filesize(fh); // ACE_OFF_T fsize = ACE_OS::filesize(ACE_HANDLE handle); 

	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send command
	if ( this->cmd_append(mb, mbox, fsize) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read command response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	if ( this->buf()[0] != '+' )
	{
		ACE_OS::printf("append response begins without '+'!\n");
		return -1;
	}

	// send data
	while(true)
	{
		n_recv = ACE_OS::read(fh, mb.base(), mb.capacity());
		if ( n_recv < 1 ) break;
		n_send = stream.send_n(mb.base(), n_recv, flags, timeout);
	}
	n_send = stream.send_n("\r\n", 2, flags, timeout);

	// read data response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return parse_cmd_response();
}

int
IMAP4_Client_IO::append_from_data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox, const char* buf, size_t len)
{
	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	// send append command
	if ( this->cmd_append(mb, mbox, len) == IMAP4_Client_IO::WR_OK )
	{
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
		if ( n_send != mb.length() )
		{
			ACE_OS::printf("send append command error!\n");
			return -1;
		}
	}
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read append response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
			{
				ACE_OS::printf("read append response error!\n");
				return -1;
			}
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	if ( this->buf()[0] != '+' )
	{
		ACE_OS::printf("append response begins without '+'!\n");
		return parse_cmd_response();
	}

	// send append data
	n_send = stream.send_n(buf, len, flags, timeout);
	if ( n_send != len )
	{
		ACE_OS::printf("send append data (%d) error (%d)!\n", n_send, ACE_OS::last_error());
		return -1;
	}
	n_send = stream.send_n("\r\n", 2, flags, timeout);
	if ( n_send != 2 )
	{
		ACE_OS::printf("send append data CRLF error!\n");
		return -1;
	}

	// read append data response
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout);
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
			{
				ACE_OS::printf("read append data response error!\n");
				return -1;
			}
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			aos::bcstr line = this->read_line(mb);
			//ACE_OS::printf("%s", this->buf().c_str());
		}
	}

	return parse_cmd_response();
}

int
IMAP4_Client_IO::fetch_to_data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* sequence, const char* item, std::string& data)
{
	ssize_t n_recv = -1;
	ssize_t n_send = -1;

	// send fetch command
	if ( this->cmd_fetch(mb, sequence, item) == IMAP4_Client_IO::WR_OK )
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	else
	{
		ACE_OS::printf("insufficient send buffer!\n");
		return -1;
	}

	// read fetch response
	data.resize(0);
	this->read_reset(mb);
	while( this->read_state() != IMAP4_Client_IO::RD_OK )
	{
		if ( mb.length() == 0 )
		{
			mb.reset();
			n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, timeout); // Non-blocking mode
			if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
				break;
			if ( n_recv > 0 ) mb.wr_ptr(n_recv);
		}
		if ( mb.length() > 0 )
		{
			if ( this->read_state() != IMAP4_Client_IO::RD_DATA )
			{
				aos::bcstr line = this->read_line(mb);
				ACE_OS::printf("%s", this->buf().c_str());
			}
			else
			{
				aos::bcstr bstr = this->read_data(mb);
				data.append(bstr.buf, bstr.len);
			}
		}
	}

	return 0;
}

/*
// Test Function
int run_imap4_client_io(int argc, ACE_TCHAR* argv[])
{
	//ACE_INET_Addr server_addr(143, "zcsdemo.cellopoint.com");
	//ACE_SOCK_Connector connector;
	//ACE_SOCK_Stream stream;

	ACE_INET_Addr server_addr(993, "zcsdemo.cellopoint.com"); // SSL
	ACE_SSL_SOCK_Connector connector; // SSL
	ACE_SSL_SOCK_Stream stream; // SSL

	ACE_Time_Value timeout(3);
	int flags = 0;

	ACE_Message_Block mb(4096); // read/write buffer
	if ( connector.connect(stream, server_addr, &timeout) != -1 )
	{
		IMAP4_Client_IO io;
		io.open("A001");

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		// read greetings
		io.read_reset();
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_greeting(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		// login
		if ( io.cmd_login(mb, "ntut", "111111") == IMAP4_Client_IO::WR_OK )
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read login response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		// write command
		if ( io.cmd_capability(mb) == IMAP4_Client_IO::WR_OK &&
			stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout) > 0 )
		{
			// read response
			io.read_reset(mb);
			while( io.read_state() != IMAP4_Client_IO::RD_OK )
			{
				if ( mb.length() == 0 )
				{
					mb.reset();
					n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
					if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
						break;
					if ( n_recv > 0 ) mb.wr_ptr(n_recv);
				}
				if ( mb.length() > 0 )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
			}
		}

		// user-command
		mb.reset();
		mb.copy("A001 SELECT INBOX\r\n");
		mb.wr_ptr(-1); // take '\0' out!
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read command response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}
		
		// write command
		if ( io.cmd_noop(mb) == IMAP4_Client_IO::WR_OK &&
			stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout) > 0 )
		{
			// read response
			io.read_reset(mb);
			while( io.read_state() != IMAP4_Client_IO::RD_OK )
			{
				if ( mb.length() == 0 )
				{
					mb.reset();
					n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
					if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
						break;
					if ( n_recv > 0 ) mb.wr_ptr(n_recv);
				}
				if ( mb.length() > 0 )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
			}
		}

		// user-command
		mb.reset();
		mb.copy("A001 FETCH 1 BODY[]\r\n");
		mb.wr_ptr(-1); // take '\0' out!
		n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read command response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout); // Non-blocking mode
				//n_recv = stream.recv(mb.wr_ptr(), mb.space()); // Blocking mode
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				if ( io.read_state() != IMAP4_Client_IO::RD_DATA )
				{
					aos::bcstr line = io.read_line(mb);
					ACE_OS::printf("%s", io.buf().c_str());
				}
				else
				{
					aos::bcstr data = io.read_data(mb);
					std::string in_data(data.buf, data.len);
					ACE_OS::printf("%s", in_data.c_str());
				}
			}
		}

		// logout
		if ( io.cmd_logout(mb) == IMAP4_Client_IO::WR_OK )
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);

		// read logout response
		io.read_reset(mb);
		while( io.read_state() != IMAP4_Client_IO::RD_OK )
		{
			if ( mb.length() == 0 )
			{
				mb.reset();
				n_recv = stream.recv(mb.wr_ptr(), mb.space(), flags, &timeout);
				if ( n_recv < 0 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				if ( n_recv > 0 ) mb.wr_ptr(n_recv);
			}
			if ( mb.length() > 0 )
			{
				aos::bcstr line = io.read_line(mb);
				ACE_OS::printf("%s", io.buf().c_str());
			}
		}

		io.close();
		stream.close();
	}

	return 0;
}
//*/

