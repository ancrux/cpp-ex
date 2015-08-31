#ifndef _ENVELOPE_H_
#define _ENVELOPE_H_

#include "aos/mime/MIME_Util.h"

#include "ace/OS.h"
#include "ace/Synch_T.h"

#include <string>
#include <list>

namespace aos {
	
class AOS_API Envelope // or Envelope_Util
{
public:
	//static void to_mime(MIME_Entity& e); 
	//static ACE_HANDLE open_log(const char* evl_file);
	//static int write_log(ACE_HANDLE fh, const void* buf, size_t len);
	//static int close_log(ACE_HANDLE fh);
	//static void get_value(MIME_Entity& e, const char* key);
	//static void get_mail(MIME_Entity& e);
	//static void get_from(MIME_Entity& e);
	//static void get_to(MIME_Entity& e, const char* domain);
	//static void to_job();

public:
	Envelope(const char* evl_file = 0);
	~Envelope();

public:
	int open(const char *evl_file); // thread-safe & return ref_count?
	int log(const void* buf, size_t len); // thread-safe log
	int close(); // thread-safe & return ref_count?

public:
	//+ get_value(key);
	//+ get_mail();
	//+ get_from();
	//+ get_to(domain);
	//+ add support for split/merge one envelope to many envelopes by recipient domain?

protected: // envelope
	int open_evl(const char *evl_file);
	int is_evl_open() { return (ACE_INVALID_HANDLE != fhv_); };
	int log_evl(const void* buf, size_t len) { return ACE_OS::write(fhv_, buf, len); };
	int close_evl();
	
protected:
	ACE_HANDLE fhv_; // file handle to evl (envelope)
	MIME_Entity e_; // evl header entity
	//int n_ref_ // reference count 

	ACE_Thread_Mutex lock_; // mutex
};
	
} // namespace aos

#endif // _MIME_ENTITY_H_
