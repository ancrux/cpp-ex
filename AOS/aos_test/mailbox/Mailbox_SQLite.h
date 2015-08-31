#ifndef _MAILBOX_SQLITE_H_
#define _MAILBOX_SQLITE_H_

#include "Mailbox_API.h"

#include "ace/Atomic_Op.h"

namespace aos {

class AOS_API Mailbox_SQLite : public Mailbox_API
{
// ctor & dtor
public:
	Mailbox_SQLite(const char* base_dir = ".");
	virtual ~Mailbox_SQLite();

// ID generator
public:
	virtual std::string generate_mailbox_id();
	virtual std::string generate_mail_id(const char* eml_path = 0);

// Mailbox
public:
	virtual std::string create_mailbox(); // return mailbox_id
	virtual int destroy_mailbox(const std::string& mailbox_id); // return status

// Mail address
public:
	virtual int link_address_to_mailbox(const std::string& address, const std::string& mailbox_id);
	virtual std::string unlink_address_to_mailbox(const std::string& address);
	virtual std::string get_mailbox_from_address(const std::string& address);

// Mail
public:
	virtual std::string insert_mail(const char* eml_path, const std::string& mailbox_id_list = ""); // return mail_id for eml_path
	virtual int remove_mail(const std::string& mailbox_id, const std::string& mail_id_list); // return status

// get Mail Meta

// get Mail Data

protected:
	std::string base_dir_;

protected:
	static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> counter_;
};

} // namespace aos

#endif // _MAILBOX_API_H_
