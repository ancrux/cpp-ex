#ifndef _IMAP4_SERVER_IO_H_
#define _IMAP4_SERVER_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"

class IMAP4_Server_IO
{
public:
	enum
	{
		RD_MORE = 0,
		RD_OK

	};

	enum
	{
		WR_MORE = 0,
		WR_OK
	};

public:
	class State
	{
	public:
		enum
		{
			NOT_AUTH,
			AUTH,
			SELECT
		};
	};

public:
	IMAP4_Server_IO();
	~IMAP4_Server_IO();
	void open() {};
	void close() {};
	std::string& buf() { return buf_; };
	const char* tag() { return params_[0]; };

public: // read response
	void read_reset() { rd_state_ = RD_MORE; rd_byte_ = 0; buf_.resize(0); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); }

	aos::bcstr read_cmd(ACE_Message_Block& mb); // use buf() to get command
	int read_state() const { return rd_state_; };
	size_t bytes_to_read() const { return rd_byte_; };

public: // exec command
	int exec_cmd(ACE_Message_Block& mb);

	int greetings(ACE_Message_Block& mb);
	int logout(ACE_Message_Block& mb);
	int noop(ACE_Message_Block& mb);
	int capability(ACE_Message_Block& mb);

	int login(ACE_Message_Block& mb, const char* user, const char* pass);
	int list(ACE_Message_Block& mb, const char* param1, const char* param2);

public: // error
	int bad_cmd(ACE_Message_Block& mb);

protected:
	int parse_cmd(aos::Multi_String& params, const char* buf, size_t len);

protected:
	int state_;
	std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	size_t rd_byte_; // used by read_data(), n bytes data to read

	int wr_state_; // write state, see enum {}
	//size_t wr_byte_;

protected:
	aos::Multi_String params_; // command parameters e.g "tag cmd param1 param2..."
};

#endif
