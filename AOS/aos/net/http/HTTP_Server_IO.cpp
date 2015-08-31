#include "HTTP_Server_IO.h"

HTTP_Server_IO::HTTP_Server_IO()
{
	read_reset();
}

HTTP_Server_IO::~HTTP_Server_IO()
{
}

void
HTTP_Server_IO::get_request(std::string* cmd, std::string* url, std::string* ver) const
{
	if ( !headers_.empty() )
	{
		aos::Tokenizer toker(headers_[0], headers_.size(0));
		toker.set_separator(" \t\r\n");
		int ch;
		ch = toker.next();
		if ( cmd )
		{
			cmd->resize(0);
			if ( ch > aos::Tokenizer::End )
				cmd->assign(toker.token(), toker.size());
		}
		ch = toker.next();
		if ( url )
		{
			url->resize(0);
			if ( ch > aos::Tokenizer::End )
				url->assign(toker.token(), toker.size());
		}
		ch = toker.next();
		if ( ver )
		{
			ver->resize(0);
			if ( ch > aos::Tokenizer::End )
				ver->assign(toker.token(), toker.size());
		}
	}
}

void
HTTP_Server_IO::parse_header()
{
	rd_byte_ = NO_CONTENT_LENGTH;
	for(size_t i = 1; i < headers_.count(); ++i) // skip headers_[0]
	{
		switch(*(headers_[i]))
		{
			case 'C':
			{
				if ( aos::strncasecmp(headers_[i], "content-length:", 15) == 0 )
				{
					rd_byte_ = ACE_OS::atoi(headers_[i]+15);
				}
				else if ( aos::strncasecmp(headers_[i], "connection:", 11) == 0 )
				{
					std::string val(headers_[i]+11);
					aos::ltrim(val);
					if ( aos::strncasecmp(val.c_str(), "keep-alive", 10) == 0 )
					{
						flags_ |= KEEP_ALIVE;
					}
				}
				break;
			}
		} // switch
	} // for
}

aos::bcstr
HTTP_Server_IO::read_header(ACE_Message_Block& mb)
{
	aos::bcstr line;
	line.buf = mb.rd_ptr();
	line.len = 0;

	char LF = '\n';
	char CR = '\r';
	if ( rd_state_ == RD_MORE || rd_state_ == RD_HEADER )
	{
		line = aos::get_line(mb.rd_ptr(), mb.length(), LF);
		mb.rd_ptr(line.len);
		if ( headers_.strings_size() + line.len > HEADER_MAX )
		{
			rd_state_ = RD_ERR_HEADER_OVERSIZE;
			return line;
		}
		if ( line.len && line.buf[line.len-1] == LF )
		{
			size_t n_crlf = 1;
			if ( line.len > 1 && line.buf[line.len-2] == CR )
				n_crlf = 2;

			if ( rd_state_ == RD_HEADER )
			{
				if ( line.len == n_crlf )
					rd_state_ = RD_OK;
				else
					headers_.push_back(line.buf, line.len-n_crlf);
			}
			else // rd_state_ == RD_MORE
			{
				headers_.append_to_last(line.buf, line.len-n_crlf);
			}
			
			if ( rd_state_ == RD_OK )
			{
				// header completed!
				this->parse_header();
				if ( aos::strncasecmp(headers_[0], "POST", 4) == 0 || aos::strncasecmp(headers_[0], "PUT", 3) == 0 )
				{
					// POST & PUT command has message body!
					rd_state_ = RD_DATA;
				}
			}
			else
			{
				rd_state_ = RD_HEADER;
			}
		}
		else
		{
			if ( rd_state_ == RD_HEADER )
				headers_.push_back(line.buf, line.len);
			else // rd_state_ == RD_MORE
				headers_.append_to_last(line.buf, line.len);

			rd_state_ = RD_MORE;
		}
	}

	return line;
}

aos::bcstr
HTTP_Server_IO::read_data(ACE_Message_Block& mb)
{
	aos::bcstr data;
	data.buf = mb.rd_ptr();
	data.len = 0;

	if ( rd_state_ == RD_DATA )
	{
		if ( rd_byte_ != NO_CONTENT_LENGTH )
		{
			data.len = (rd_byte_>mb.length())?mb.length():rd_byte_;
			rd_byte_ -= data.len;

			if ( rd_byte_ == 0 )
				rd_state_ = RD_OK;
		}
		else
		{
			// will skip reading message-body
			data.len = mb.length();
			rd_state_ = RD_OK;
		}
		mb.rd_ptr(data.len);
	}

	return data;
}

ACE_OFF_T
HTTP_Server_IO::write_file(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* filename, const char* ctype)
{
	ACE_HANDLE fh = ACE_OS::open(filename, O_BINARY | O_RDONLY);
	if ( fh == ACE_INVALID_HANDLE )
	{
		ACE_OS::printf("cannot open file: '%s'!\n", filename); //@
		return -1;
	}
	ACE_OFF_T fsize = ACE_OS::filesize(fh); 

	ssize_t n_send = -1;
	ssize_t n_recv = -1;

	mb.reset();
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "HTTP/%.1f %d %s\r\n", 1.1, 200, "OK"));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Content-Type: %s\r\n", ctype));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Content-Length: %d\r\n", fsize));
	if ( this->keep_alive() ) 
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Connection: %s\r\n", "Keep-Alive"));
	else
		mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "Connection: %s\r\n", "close"));
	mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));

	// send header
	n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, timeout);
	if ( n_send < 1 ) return -1; // write failed, close connection

	// send file
	ACE_OFF_T n_total = 0;
	if ( fsize > 0 )
	{
		while(true)
		{
			n_recv = ACE_OS::read(fh, mb.base(), mb.capacity());
			if ( n_recv < 1 ) break;
			n_send = stream.send_n(mb.base(), n_recv, flags, timeout);
			if ( n_send < 1 ) break;
			n_total += n_send;
		}
	}
	ACE_OS::close(fh);

	return n_total;
}

