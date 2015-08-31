#ifndef _MAILBOX_API_H_
#define _MAILBOX_API_H_

#include "aos/Config.h"
#include "aos/String.h"

namespace aos {

class AOS_API Mailbox_API
{
// ctor & dtor
public:
	Mailbox_API() {};
	virtual ~Mailbox_API() {};

// ID generator
public:
	virtual std::string generate_mailbox_id() = 0;
	virtual std::string generate_mail_id(const char* eml_path = 0) = 0;

// Mailbox
public:
	virtual std::string create_mailbox() = 0; // return mailbox_id
	virtual int destroy_mailbox(const std::string& mailbox_id) = 0; // return status

// Mail address
public:
	virtual int link_address_to_mailbox(const std::string& address, const std::string& mailbox_id) = 0;
	virtual std::string unlink_address_to_mailbox(const std::string& address) = 0;
	virtual std::string get_mailbox_from_address(const std::string& address) = 0;

// Mail
public:
	virtual std::string insert_mail(const char* eml_path, const std::string& mailbox_id_list = "") = 0; // return mail_id for eml_path
	virtual int remove_mail(const std::string& mailbox_id, const std::string& mail_id_list) = 0; // return status

// get Mail Meta

// get Mail Data

};

} // namespace aos

#endif // _MAILBOX_API_H_
