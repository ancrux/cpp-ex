#ifndef _IMAP4_CLIENT_IO_H_
#define _IMAP4_CLIENT_IO_H_

#include "ace/OS.h"
#include "ace/Message_Block.h"

#include "aos/String.h"
#include "aos/SOCK_Stream.h"

class IMAP4_Client_IO
{
public:
	enum
	{
		RD_MORE = 0,
		RD_LINE,
		RD_DATA,
		RD_DATA_OK,
		RD_OK
	};

	enum
	{
		WR_MORE = 0,
		WR_OK
	};

	enum
	{
		OK = 0,
		NO,
		BAD,
		UNKNOWN
	};

public:
	IMAP4_Client_IO();
	~IMAP4_Client_IO();
	void open(const char* tag) { tag_.assign(tag); };
	void close() {};
	std::string& buf() { return buf_; };

public: // IO read response
	void read_reset() { rd_state_ = RD_MORE; rd_byte_ = 0; buf_.resize(0); };
	void read_reset(ACE_Message_Block& mb) { read_reset(); mb.reset(); }

	aos::bcstr read_greeting(ACE_Message_Block& mb); // use buf() to get greeting
	aos::bcstr read_line(ACE_Message_Block& mb); // use buf() to get line
	aos::bcstr read_data(ACE_Message_Block& mb);
	int read_state() const { return rd_state_; };
	size_t bytes_to_read() const { return rd_byte_; };

public: // parse command response
	int parse_cmd_response(); // return OK, NO, BAD, UNKNOWN

public: // IO send command
	int cmd_login(ACE_Message_Block& mb, const char* user, const char* pass);
	int cmd_logout(ACE_Message_Block& mb);
	int cmd_noop(ACE_Message_Block& mb);
	int cmd_capability(ACE_Message_Block& mb);
	int cmd_list(ACE_Message_Block& mb, const char* reference, const char* mbox);
	int cmd_select(ACE_Message_Block& mb, const char* mbox);
	int cmd_create(ACE_Message_Block& mb, const char* mbox);
	int cmd_append(ACE_Message_Block& mb, const char* mbox, size_t n_byte);
	int cmd_fetch(ACE_Message_Block& mb, const char* sequence, const char* item);

public: // Stream command (combo)
	int greetings(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int login(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* user, const char* pass);
	int logout(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int capability(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb);
	int list(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* reference, const char* mbox);
	int select(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox);
	int create(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox);
	int append_from_file(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox, const char* filename);
	int append_from_data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* mbox, const char* buf, size_t len);
	int fetch_to_data(aos::SOCK_Stream& stream, int flags, const ACE_Time_Value* timeout, ACE_Message_Block& mb, const char* sequence, const char* item, std::string& data);

protected:
	std::string tag_; // imap4 tag
	std::string buf_; // internal buffer

	int rd_state_; // read state, see enum {}
	size_t rd_byte_; // used by read_data(), n bytes data to read

	//int wr_state_; // not needed here!
};

#endif
