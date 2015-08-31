#ifndef _ASIO_DNS_RESOLVER_H_
#define _ASIO_DNS_RESOLVER_H_

#include "ace/Singleton.h"
#include "ace/Synch_T.h"

#define HAVE_BOOL_T
#include "ares_setup.h"
#include "ares.h"
#include "ares_dns.h"
#include "nameser.h"
//#include "inet_ntop.h"
//#include "inet_net_pton.h"

#include <boost/bind.hpp> // use boost::ref() & boost::cref() to pass arguments
#include <boost/function.hpp>

namespace asio {

class DNS_Resolver_Environment
{
public:
	DNS_Resolver_Environment() { status_ = ares_library_init(ARES_LIB_INIT_ALL); };
	~DNS_Resolver_Environment() { ares_library_cleanup(); };

public:
	int status() const { return status_; };

protected:
	int status_;
};
typedef ACE_Singleton< DNS_Resolver_Environment, ACE_Thread_Mutex > The_DNS_Resolver_Environment;

struct nv {
  const char *name;
  int value;
};

static const struct nv classes[] = {
  { "IN",       C_IN },
  { "CHAOS",    C_CHAOS },
  { "HS",       C_HS },
  { "ANY",      C_ANY }
};
static const int nclasses = sizeof(classes) / sizeof(classes[0]);

static const struct nv types[] = {
  { "A",        T_A },
  { "NS",       T_NS },
  { "MD",       T_MD },
  { "MF",       T_MF },
  { "CNAME",    T_CNAME },
  { "SOA",      T_SOA },
  { "MB",       T_MB },
  { "MG",       T_MG },
  { "MR",       T_MR },
  { "NULL",     T_NULL },
  { "WKS",      T_WKS },
  { "PTR",      T_PTR },
  { "HINFO",    T_HINFO },
  { "MINFO",    T_MINFO },
  { "MX",       T_MX },
  { "TXT",      T_TXT },
  { "RP",       T_RP },
  { "AFSDB",    T_AFSDB },
  { "X25",      T_X25 },
  { "ISDN",     T_ISDN },
  { "RT",       T_RT },
  { "NSAP",     T_NSAP },
  { "NSAP_PTR", T_NSAP_PTR },
  { "SIG",      T_SIG },
  { "KEY",      T_KEY },
  { "PX",       T_PX },
  { "GPOS",     T_GPOS },
  { "AAAA",     T_AAAA },
  { "LOC",      T_LOC },
  { "SRV",      T_SRV },
  { "AXFR",     T_AXFR },
  { "MAILB",    T_MAILB },
  { "MAILA",    T_MAILA },
  { "NAPTR",    T_NAPTR },
  { "ANY",      T_ANY }
};
static const int ntypes = sizeof(types) / sizeof(types[0]);

static const char *opcodes[] = {
  "QUERY", "IQUERY", "STATUS", "(reserved)", "NOTIFY",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "UPDATEA", "UPDATED", "UPDATEDA", "UPDATEM", "UPDATEMA",
  "ZONEINIT", "ZONEREF"
};

static const char *rcodes[] = {
  "NOERROR", "FORMERR", "SERVFAIL", "NXDOMAIN", "NOTIMP", "REFUSED",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "NOCHANGE"
};

class DNS_Resolver
{
public:
	static const char* inet_ntop4(const unsigned char *src, char *dst, size_t size);
	static const char* inet_ntop6(const unsigned char *src, char *dst, size_t size);
	static const char* ares_inet_ntop(int af, const void *src, char *dst, size_t size);

public:
	static const char* type_name(int type);
	static const char* class_name(int dnsclass);
	static const unsigned char* display_question(const unsigned char *aptr, const unsigned char *abuf, int alen);
	static const unsigned char* display_rr(const unsigned char *aptr, const unsigned char *abuf, int alen);

public:
	typedef boost::function< void (/* void*, */ int, int, unsigned char*, int) > query_callback;
	static void ares_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen);

public:
	DNS_Resolver(struct ares_options* ares_options = 0, int optmask = ARES_OPT_FLAGS);
	virtual ~DNS_Resolver();

public:
	int status() const { return status_; };
	const char* last_error() const { return ares_strerror(status_); };
	bool is_status_ok() const { return status() == ARES_SUCCESS; };
	ares_channel channel() const { return channel_; };
	void async_query(const char* name, int rr_class, int rr_type, query_callback func);
	int wait_query(struct timeval& tv);

protected:
	int status_;
	ares_channel channel_;
	query_callback func_;
};

struct DNS_Query
{
	typedef boost::function< void (/* void*, */ int, int, unsigned char*, int) > query_callback;
	static void ares_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen);	

	std::string name;
	int rr_class;
	int rr_type;
	query_callback callback;

	void ask(DNS_Resolver& reslover);
};

} // namespace asio

#endif // _ASIO_DNS_RESOLVER_H_