#ifndef _SMTP_CLIENT_IO_H_
#define _SMTP_CLIENT_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"
#include "aos/SOCK_Stream.h"

class SMTP_Client_IO
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
	SMTP_Client_IO();
	~SMTP_Client_IO();

public: // open & close
	void open() {};
	void close() {};

public: // state & buffer
	int state() const { return state_; };
	std::string& buf() { return buf_; };

public: // IO read command
	int read_state() const { return rd_state_; };
	void read_reset() { rd_state_ = RD_MORE; buf_.resize(0); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); };

	aos::bcstr read_line(ACE_Message_Block& mb, size_t max_size = 0); // use buf() to get line

public: // parse command response
	int parse_cmd_response();

public: // IO write response
	//int write_state() const { return wr_state_; };

	// SMTP
	int cmd_helo(ACE_Message_Block& mb, const char* host);
	int cmd_mail_from(ACE_Message_Block& mb, const char* email, ACE_INT64 size = -1);
	int cmd_rcpt_to(ACE_Message_Block& mb, const char* email);
	int cmd_data(ACE_Message_Block& mb);
	int cmd_data_completed(ACE_Message_Block& mb);
	int cmd_noop(ACE_Message_Block& mb);
	int cmd_rset(ACE_Message_Block& mb);
	int cmd_quit(ACE_Message_Block& mb);

	// Extended SMTP
	int cmd_ehlo(ACE_Message_Block& mb, const char* host);
	int cmd_starttls(ACE_Message_Block& mb);
	int cmd_auth(ACE_Message_Block& mb, const char* scheme, const char* arg = 0);
	//?int fmt_base64()
	int cmd_bdat(ACE_Message_Block& mb, size_t size, bool last = true);
	
public: // Stream command (combo)
	// SMTP
	int greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int helo(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* host);
	int mail_from(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* email, ACE_INT64 size = -1);
	int rcpt_to(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* email);
	int data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int write_data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, const void* buf, size_t len) { return stream.send_n(buf, len, flags, timeout); };	
	int data_completed(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int rset(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int noop(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int quit(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);

	// Extended SMTP
	int ehlo(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* host);
	int starttls(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int auth(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* scheme, const char* arg = 0);
	int auth_login(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* user, const char* pass);
	int bdat(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const void* buf, size_t len, bool last = true);

protected:
	int state_; // protocol state
	std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	//size_t rd_byte_; // used by read_data(), n bytes data read

	//int wr_state_; // write state, see enum {}
	//size_t wr_byte_;
};

#endif
