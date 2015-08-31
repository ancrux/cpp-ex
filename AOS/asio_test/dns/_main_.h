
#if defined(_WIN32)
#include "vld.h"
#endif

#include "DNS_Resolver.h"
using namespace asio;

static double foo(int x, int y, int z)
{
	double v = x * y;
	ACE_OS::printf("foo: %.3f, z=%d\n", v, z);
	return v;
}

int run_resolver_test(int argc, ACE_TCHAR* argv[])
{
	DNS_Resolver dns;
	(void) dns;

	boost::function< void (void*, int, int, unsigned char*, int) > ares_callback;
	boost::function< double (int, int) > f;

	int m = 5;
	int n = 2;
	f = boost::bind(foo, boost::arg<1>(), _2, 55);
	double d = f(8,8);
	ACE_OS::printf("ret: %.3f\n", d);

	return 0;
}

static void callback(/* void *arg, */ int status, int timeouts, unsigned char *abuf, int alen, const char* name)
{
	//char *name = (char *) arg;
	int id, qr, opcode, aa, tc, rd, ra, rcode;
	unsigned int qdcount, ancount, nscount, arcount, i;
	const unsigned char *aptr;

	(void) timeouts;

	if ( name )
		printf("Answer for query '%s':\n", name);

	/* Display an error message if there was an error, but only stop if
	* we actually didn't get an answer buffer.
	*/
	if (status != ARES_SUCCESS)
	{
		printf("%s\n", ares_strerror(status));
		if (!abuf)
			return;
	}

	/* Won't happen, but check anyway, for safety. */
	if (alen < HFIXEDSZ)
		return;

	/* Parse the answer header. */
	id = DNS_HEADER_QID(abuf);
	qr = DNS_HEADER_QR(abuf);
	opcode = DNS_HEADER_OPCODE(abuf);
	aa = DNS_HEADER_AA(abuf);
	tc = DNS_HEADER_TC(abuf);
	rd = DNS_HEADER_RD(abuf);
	ra = DNS_HEADER_RA(abuf);
	rcode = DNS_HEADER_RCODE(abuf);
	qdcount = DNS_HEADER_QDCOUNT(abuf);
	ancount = DNS_HEADER_ANCOUNT(abuf);
	nscount = DNS_HEADER_NSCOUNT(abuf);
	arcount = DNS_HEADER_ARCOUNT(abuf);

	/* Display the answer header. */
	printf("id: %d\n", id);
	printf("flags: %s%s%s%s%s\n",
		qr ? "qr " : "",
		aa ? "aa " : "",
		tc ? "tc " : "",
		rd ? "rd " : "",
		ra ? "ra " : "");
	printf("opcode: %s\n", opcodes[opcode]);
	printf("rcode: %s\n", rcodes[rcode]);

	/* Display the questions. */
	printf("Questions:\n");
	aptr = abuf + HFIXEDSZ;
	for (i = 0; i < qdcount; i++)
	{
		aptr = DNS_Resolver::display_question(aptr, abuf, alen);
		if (aptr == NULL)
			return;
	}

	/* Display the answers. */
	printf("Answers:\n");
	for (i = 0; i < ancount; i++)
	{
		aptr = DNS_Resolver::display_rr(aptr, abuf, alen);
		if (aptr == NULL)
			return;
	}

	/* Display the NS records. */
	printf("NS records:\n");
	for (i = 0; i < nscount; i++)
	{
		aptr = DNS_Resolver::display_rr(aptr, abuf, alen);
		if (aptr == NULL)
			return;
	}

	/* Display the additional records. */
	printf("Additional records:\n");
	for (i = 0; i < arcount; i++)
	{
		aptr = DNS_Resolver::display_rr(aptr, abuf, alen);
		if (aptr == NULL)
			return;
	}
}

int run_dns_test(int argc, ACE_TCHAR* argv[])
{
	DNS_Resolver dns;
	if ( !dns.is_status_ok() )
		ACE_OS::printf("resolver ctor() error: %s\n", dns.last_error());

	/*
	ares_channel channel = dns.channel();
	int status = 0;

	// set query info
	int rr_class = C_IN;
	int rr_type = T_A;
	// start async-query //ares_query(channel, str, rr_class, rr_type, callback, (char *) NULL);
	std::string name = "gmail.com";
	dns.async_query(name.c_str(), rr_class, rr_type, boost::bind(&callback, _1, _2, _3, _4, name.c_str()));
	std::string name2 = "a.com";
	dns.async_query(name2.c_str(), rr_class, rr_type, boost::bind(&callback, _1, _2, _3, _4, name2.c_str()));
	//*/

	///*
	DNS_Query q1; q1.rr_class = C_IN; q1.rr_type = T_MX; q1.name = "gmail.com";
	q1.callback = boost::bind(callback, _1, _2, _3, _4, q1.name.c_str());
	q1.ask(dns);

	DNS_Query q2; q2.rr_class = C_IN; q2.rr_type = T_A; q2.name = "microsoft.com";
	q2.callback = boost::bind(callback, _1, _2, _3, _4, q2.name.c_str());
	q2.ask(dns);
	//*/

	struct timeval tv;
	dns.wait_query(tv);

	return 0;
}
