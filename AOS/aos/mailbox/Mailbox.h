#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include "aos/OS.h"
#include "aos/String.h"
#include "aos/mime/MIME_Entity.h"

namespace aos {
namespace mailbox {

struct Mail_Block //? or class
{
	//ACE_OFF_T size;
	//std::string data;
};

struct Mail_Entry //? or class
{
	//ACE_OFF_T size; //? 4:mail size
	time_t atime; // 8:arrival/received time

	//ACE_UINT32 flag; //? 4:deleted (replace with Trash tag?), attach_count, priority
	ACE_UINT32 mbox; // 4:mbox/where/tag/group/folder id e.g. Deleted(id=0), INBOX(id=1), Sent, Draft, Trash
	
	// MIME info //? std::vector< std::string >
	aos::Multi_String info; // 44:[0]=uid/file/filesize, [1]=subject, [2]=sender, [3]=recipient... (MIME) //std::string needs 28 bytes
	//int n_to;
	//int n_cc;
	//int n_bcc;
	
	//time_t ctime; //? 8:created/sent time (MIME)
	//int attach_count; (MIME)
	//int priority; (MIME)

	Mail_Block* data; // complete mail data (eml)
};

class Mail_Archive
{
public:

protected:
	ACE_HANDLE fh_;
};

class Mailbox
{
public:
	Mailbox();
	virtual ~Mailbox();

public:
	// e_addr for mailbox's email address
	virtual int open(const char* e_addr);
	virtual int close();
	
public:
	//// maintain one std::list for mail entries (structure only),
	//// one std::map for sorted mail entries (pair<sorted field, pair<pointer to mail entries, mail id>)
	// void list(); // list all mail entries
	// void sort(); // sort mail entries

	virtual int select(const char* mbox = "");
	virtual int count(); // # of mails in mbox, 0 for all mboxes
	virtual int list(); // list all mail entries, 0 for all mboxes

protected: // variables for based class
	std::string addr_; // for file-based mbox, use email_addr to store directory path
	std::string mbox_; // selected mbox name

protected: // variables for derived class
	//ACE_Dirent dir_;
	//ACE_DIRENT* d;
};

} // namespace mailbox
} // namespace aos

#endif // _MAILBOX_H_

