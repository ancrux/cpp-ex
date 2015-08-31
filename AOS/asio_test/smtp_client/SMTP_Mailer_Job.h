#ifndef _ASIO_SMTP_MAILER_JOB_H_
#define _ASIO_SMTP_MAILER_JOB_H_

#include "aos/String.h"

namespace asio {

class SMTP_Mailer_Job
{
public:
	std::string from;
	std::string to;
	std::string eml;
};

} // namespace asio

#endif // _ASIO_SMTP_MAILER_JOB_H_
