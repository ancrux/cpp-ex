#ifndef _ASIO_SMTP_MAILER_JOB_H_
#define _ASIO_SMTP_MAILER_JOB_H_

#include "ace/OS.h"

#include "aos/String.h"

namespace asio {

class SMTP_Mailer_Job
{
// old
public:
	std::string from;
	std::string to;
	std::string eml;

// new
public:
	const char* evl_path() { return mstr_[0]; };
	const char* eml_path() { return mstr_[1]; };
	const char* sndr() { return mstr_[2]; };
	const char* rcpt() { return mstr_[3]; };

protected:
	aos::Multi_String mstr_;
	ACE_HANDLE fhv_;
	ACE_HANDLE fhm_;
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_JOB_H_
