#ifndef _SMTP_Server_IO_H_
#define _SMTP_Server_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"
#include "aos/SOCK_Stream.h"

class SMTP_Server_IO
{
public: // read state
	enum
	{
		RD_MORE = 0,
		RD_DATA,
		RD_OK
	};

public: // write state
	enum
	{
		WR_MORE = 0,
		WR_OK
	};

public: // protocol state (SMTP state)
	class State
	{
	public:
		enum
		{
			START, // HELO
			RESET, // MAIL
			AUTH,
			SSL,
			MAIL, // RCPT
			DATA
		};
	};

public: // ctor & dtor
	SMTP_Server_IO();
	~SMTP_Server_IO();

public: // open & close
	void open() { state_ = State::START; };
	void close() {};

public: // state & buffer
	int state() const { return state_; };
	std::string& buf() { return buf_; };

public: // IO read command
	int read_state() const { return rd_state_; };
	size_t bytes_to_read() const { return rd_byte_; };
	void read_reset() { rd_state_ = RD_MORE; rd_byte_ = 0; buf_.resize(0); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); };

	aos::bcstr read_line(ACE_Message_Block& mb, size_t max_size = 0); // use buf() to get line
	aos::bcstr read_data(ACE_Message_Block& mb); // use bytes_to_read() for byte read

public: // parse command
	int parse_cmd(aos::Multi_String& params, const char* buf, size_t len);

public: // IO write response
	int write_state() const { return wr_state_; };
	int write_line(ACE_Message_Block& mb, int code, const char* msg);

	int greetings(ACE_Message_Block& mb, const char* greetings = 0);
	int helo(ACE_Message_Block& mb);
	int mail(ACE_Message_Block& mb);
	int rcpt(ACE_Message_Block& mb);
	int data(ACE_Message_Block& mb);
	int data_completed(ACE_Message_Block& mb);
	int noop(ACE_Message_Block& mb);
	int rset(ACE_Message_Block& mb);
	int quit(ACE_Message_Block& mb);

public: // exec command
	int exec_cmd(ACE_Message_Block& mb);

public: // Stream command (combo)
	int greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);

protected:
	int state_; // protocol state
	std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	size_t rd_byte_; // used by read_data(), n bytes data read

	int wr_state_; // write state, see enum {}
	//size_t wr_byte_;

protected:
	aos::Multi_String params_; // command parameters e.g "tag cmd param1 param2..."
	aos::Multi_String emails_; // email addresses of "FROM:", "TO:"
	
};

#endif
