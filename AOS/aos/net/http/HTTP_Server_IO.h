#ifndef _HTTP_SERVER_IO_H_
#define _HTTP_SERVER_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"
#include "aos/SOCK_Stream.h"

class HTTP_Server_IO
{
public:
	static const size_t HEADER_MAX = 8192;
	static const size_t NO_CONTENT_LENGTH = (size_t) -1;

public:
	enum
	{
		KEEP_ALIVE = 0x01
	};

public:
	enum
	{
		RD_MORE = 0,
		RD_HEADER,
		RD_DATA,
		RD_OK,
		RD_ERR_HEADER_OVERSIZE
	};

	enum
	{
		WR_MORE = 0,
		WR_OK
	};

public:
	HTTP_Server_IO();
	~HTTP_Server_IO();
	void open() {};
	void close() {};
	//std::string& buf() { return buf_; };

public: // IO read command
	void read_reset() { rd_state_ = RD_MORE; rd_byte_ = 0; flags_ = 0; headers_.clear(); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); }

	aos::bcstr read_header(ACE_Message_Block& mb); // stored in headers_
	aos::bcstr read_data(ACE_Message_Block& mb);
	int read_state() const { return rd_state_; };
	size_t bytes_to_read() const { return rd_byte_; };

public: // IO write command
	//int write_header(ACE_Message_Block& mb, const char* header, int append = 1);
	//int write_header(ACE_Message_Block& mb, const char* key, const char* value, int append = 1);

public:
	void get_request(std::string* cmd, std::string* url = 0, std::string* ver = 0) const;
	int keep_alive() const
	{
		return (flags_ & KEEP_ALIVE);
	}
	void keep_alive(int keep_alive)
	{
		if ( keep_alive )
		{
			flags_ |= KEEP_ALIVE;
		}
		else
		{
			if ( flags_ & KEEP_ALIVE ) flags_ ^= KEEP_ALIVE; // erase KEEP_ALIVE bit
		}
	}
	void dump_headers() const
	{
		for(size_t i=0; i < headers_.count(); ++i)
		{
			ACE_OS::printf("[%d]%s\n", i, headers_[i]);
		}
	};

public: // Stream command (combo)
	ACE_OFF_T write_file(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* filename, const char* ctype = "application/octet-stream");

protected:
	void parse_header(); // parse header when header is completed

protected:
	//std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	size_t rd_byte_; // used by read_data(), n bytes data to read

	int wr_state_; // write state, see enum {}
	//size_t wr_byte_;

protected:
	int flags_; // flags for Keep-Alive or other header attributes
	aos::Multi_String headers_; // command parameters e.g "tag cmd param1 param2..."
};

#endif
