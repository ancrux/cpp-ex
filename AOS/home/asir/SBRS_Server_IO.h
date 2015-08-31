#ifndef _SBRS_Server_IO_H_
#define _SBRS_Server_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"
#include "aos/SOCK_Stream.h"

class SBRS_Server_IO
{
public: // read state
	enum
	{
		RD_MORE = 0,
		RD_OK
	};

public: // write state
	enum
	{
		WR_MORE = 0,
		WR_OK
	};

public: // ctor & dtor
	SBRS_Server_IO();
	~SBRS_Server_IO();

public: // open & close
	void open() {};
	void close() {};

public: // state & buffer
	int state() const { return state_; };
	std::string& buf() { return buf_; };

public: // IO read command
	int read_state() const { return rd_state_; };
	size_t bytes_to_read() const { return rd_byte_; };
	void read_reset() { rd_state_ = RD_MORE; rd_byte_ = 0; buf_.resize(0); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); };

	aos::bcstr read_request(ACE_Message_Block& mb);

public: // parse command
	int parse_cmd(aos::Multi_String& params, const char* buf, size_t len);

public: // IO write response
	int write_state() const { return wr_state_; };
	int write_response(ACE_Message_Block& mb, const char* msg);
	int write_score(ACE_Message_Block& mb, double score);

protected:
	int state_; // protocol state
	std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	size_t rd_byte_; // used by read_data(), n bytes data read

	int wr_state_; // write state, see enum {}
	//size_t wr_byte_;

protected:
	aos::Multi_String params_; // command parameters e.g "tag cmd param1 param2..."
};

#endif // _SBRS_Server_IO_H_

