#include "DNS_Resolver.h"

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf/**/x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

namespace asio {

/* const char *
* inet_ntop4(src, dst, size)
*     format an IPv4 address, more or less like inet_ntoa()
* return:
*     `dst' (as a const)
* notes:
*     (1) uses no statics
*     (2) takes a unsigned char* not an in_addr as input
* author:
*     Paul Vixie, 1996.
*/
const char*
DNS_Resolver::inet_ntop4(const unsigned char *src, char *dst, size_t size)
{
	static const char fmt[] = "%u.%u.%u.%u";
	char tmp[sizeof "255.255.255.255"];

	if (SPRINTF((tmp, fmt, src[0], src[1], src[2], src[3])) > size)
	{
		SET_ERRNO(ENOSPC);
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}

/* const char *
* inet_ntop6(src, dst, size)
*    convert IPv6 binary address into presentation (printable) format
* author:
*    Paul Vixie, 1996.
*/
const char*
DNS_Resolver::inet_ntop6(const unsigned char *src, char *dst, size_t size)
{
	/*
	* Note that int32_t and int16_t need only be "at least" large enough
	* to contain a value of the specified size.  On some systems, like
	* Crays, there is no such thing as an integer variable with 16 bits.
	* Keep this in mind if you think this function should have been coded
	* to use pointer overlays.  All the world's not a VAX.
	*/
	char tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	char *tp;
	struct {
		long base;
		long len;
	} best, cur;
	unsigned long words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;

	/*
	* Preprocess:
	*  Copy the input (bytewise) array into a wordwise array.
	*  Find the longest run of 0x00's in src[] for :: shorthanding.
	*/
	memset(words, '\0', sizeof(words));
	for (i = 0; i < NS_IN6ADDRSZ; i++)
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));

	best.base = -1;
	cur.base = -1;
	best.len = 0;
	cur.len = 0;

	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
	{
		if (words[i] == 0)
		{
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		}
		else
		{
			if (cur.base != -1)
			{
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1)
	{
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	/*
	* Format the result.
	*/
	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++)
	{
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base &&
			i < (best.base + best.len))
		{
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		/* Are we following an initial run of 0x00s or any real hex? */
		if (i != 0)
			*tp++ = ':';
		/* Is this address an encapsulated IPv4? */
		if (i == 6 && best.base == 0 &&
			(best.len == 6 || (best.len == 5 && words[5] == 0xffff)))
		{
			if (!inet_ntop4(src+12, tp, sizeof(tmp) - (tp - tmp)))
				return (NULL);
			tp += strlen(tp);
			break;
		}
		tp += SPRINTF((tp, "%lx", words[i]));
	}

	/* Was it a trailing run of 0x00's? */
	if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	/*
	* Check for overflow, copy, and we're done.
	*/
	if ((size_t)(tp - tmp) > size)
	{
		SET_ERRNO(ENOSPC);
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}

/* char *
* inet_ntop(af, src, dst, size)
*     convert a network format address to presentation format.
* return:
*     pointer to presentation format address (`dst'), or NULL (see errno).
* note:
*      On Windows we store the error in the thread errno, not
*      in the winsock error code. This is to avoid loosing the
*      actual last winsock error. So use macro ERRNO to fetch the
*      errno this funtion sets when returning NULL, not SOCKERRNO.
* author:
*     Paul Vixie, 1996.
*/
const char*
DNS_Resolver::ares_inet_ntop(int af, const void *src, char *dst, size_t size)
{
	switch (af)
	{
	case AF_INET:
		return (inet_ntop4((const unsigned char*) src, dst, size));
	case AF_INET6:
		return (inet_ntop6((const unsigned char*) src, dst, size));
	default:
		SET_ERRNO(EAFNOSUPPORT);
		return (NULL);
	}
	/* NOTREACHED */
}

const char*
DNS_Resolver::type_name(int type)
{
	int i;

	for (i = 0; i < ntypes; i++)
	{
		if (types[i].value == type)
			return types[i].name;
	}
	return "(unknown)";
}

const char*
DNS_Resolver::class_name(int dnsclass)
{
	int i;

	for (i = 0; i < nclasses; i++)
	{
		if (classes[i].value == dnsclass)
			return classes[i].name;
	}
	return "(unknown)";
}

const unsigned char*
DNS_Resolver::display_question(const unsigned char *aptr, const unsigned char *abuf, int alen)
{
	char *name;
	int type, dnsclass, status;
	long len;

	/* Parse the question name. */
	status = ares_expand_name(aptr, abuf, alen, &name, &len);
	if (status != ARES_SUCCESS)
		return NULL;
	aptr += len;

	/* Make sure there's enough data after the name for the fixed part
	* of the question.
	*/
	if (aptr + QFIXEDSZ > abuf + alen)
	{
		ares_free_string(name);
		return NULL;
	}

	/* Parse the question type and class. */
	type = DNS_QUESTION_TYPE(aptr);
	dnsclass = DNS_QUESTION_CLASS(aptr);
	aptr += QFIXEDSZ;

	/* Display the question, in a format sort of similar to how we will
	* display RRs.
	*/
	printf("\t%-15s.\t", name);
	if (dnsclass != C_IN)
		printf("\t%s", class_name(dnsclass));
	printf("\t%s\n", type_name(type));
	ares_free_string(name);
	return aptr;
}

const unsigned char*
DNS_Resolver::display_rr(const unsigned char *aptr, const unsigned char *abuf, int alen)
{
	const unsigned char *p;
	int type, dns_class, ttl, dlen, status;
	long len;
	char addr[46];
	union {
		unsigned char * as_uchar;
		char * as_char;
	} name;

	/* Parse the RR name. */
	status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
	if (status != ARES_SUCCESS)
		return NULL;
	aptr += len;

	/* Make sure there is enough data after the RR name for the fixed
	* part of the RR.
	*/
	if (aptr + RRFIXEDSZ > abuf + alen)
	{
		ares_free_string(name.as_char);
		return NULL;
	}

	/* Parse the fixed part of the RR, and advance to the RR data
	* field. */
	type = DNS_RR_TYPE(aptr);
	dns_class = DNS_RR_CLASS(aptr);
	ttl = DNS_RR_TTL(aptr);
	dlen = DNS_RR_LEN(aptr);
	aptr += RRFIXEDSZ;
	if (aptr + dlen > abuf + alen)
	{
		ares_free_string(name.as_char);
		return NULL;
	}

	/* Display the RR name, class, and type. */
	printf("\t%-15s.\t%d", name.as_char, ttl);
	if (dns_class != C_IN)
		printf("\t%s", class_name(dns_class));
	printf("\t%s", type_name(type));
	ares_free_string(name.as_char);

	/* Display the RR data.  Don't touch aptr. */
	switch (type)
	{
	case T_CNAME:
	case T_MB:
	case T_MD:
	case T_MF:
	case T_MG:
	case T_MR:
	case T_NS:
	case T_PTR:
		/* For these types, the RR data is just a domain name. */
		status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.", name.as_char);
		ares_free_string(name.as_char);
		break;

	case T_HINFO:
		/* The RR data is two length-counted character strings. */
		p = aptr;
		len = *p;
		if (p + len + 1 > aptr + dlen)
			return NULL;
		status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s", name.as_char);
		ares_free_string(name.as_char);
		p += len;
		len = *p;
		if (p + len + 1 > aptr + dlen)
			return NULL;
		status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s", name.as_char);
		ares_free_string(name.as_char);
		break;

	case T_MINFO:
		/* The RR data is two domain names. */
		p = aptr;
		status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.", name.as_char);
		ares_free_string(name.as_char);
		p += len;
		status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.", name.as_char);
		ares_free_string(name.as_char);
		break;

	case T_MX:
		/* The RR data is two bytes giving a preference ordering, and
		* then a domain name.
		*/
		if (dlen < 2)
			return NULL;
		printf("\t%d", DNS__16BIT(aptr));
		status = ares_expand_name(aptr + 2, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.", name.as_char);
		ares_free_string(name.as_char);
		break;

	case T_SOA:
		/* The RR data is two domain names and then five four-byte
		* numbers giving the serial number and some timeouts.
		*/
		p = aptr;
		status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.\n", name.as_char);
		ares_free_string(name.as_char);
		p += len;
		status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t\t\t\t\t\t%s.\n", name.as_char);
		ares_free_string(name.as_char);
		p += len;
		if (p + 20 > aptr + dlen)
			return NULL;
		printf("\t\t\t\t\t\t( %lu %lu %lu %lu %lu )",
			(unsigned long)DNS__32BIT(p), (unsigned long)DNS__32BIT(p+4),
			(unsigned long)DNS__32BIT(p+8), (unsigned long)DNS__32BIT(p+12),
			(unsigned long)DNS__32BIT(p+16));
		break;

	case T_TXT:
		/* The RR data is one or more length-counted character
		* strings. */
		p = aptr;
		while (p < aptr + dlen)
		{
			len = *p;
			if (p + len + 1 > aptr + dlen)
				return NULL;
			status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
			if (status != ARES_SUCCESS)
				return NULL;
			printf("\t%s", name.as_char);
			ares_free_string(name.as_char);
			p += len;
		}
		break;

	case T_A:
		/* The RR data is a four-byte Internet address. */
		if (dlen != 4)
			return NULL;
		printf("\t%s", DNS_Resolver::ares_inet_ntop(AF_INET,aptr,addr,sizeof(addr)));
		break;

	case T_AAAA:
		/* The RR data is a 16-byte IPv6 address. */
		if (dlen != 16)
			return NULL;
		printf("\t%s", DNS_Resolver::ares_inet_ntop(AF_INET6,aptr,addr,sizeof(addr)));
		break;

	case T_WKS:
		/* Not implemented yet */
		break;

	case T_SRV:
		/* The RR data is three two-byte numbers representing the
		* priority, weight, and port, followed by a domain name.
		*/

		printf("\t%d", DNS__16BIT(aptr));
		printf(" %d", DNS__16BIT(aptr + 2));
		printf(" %d", DNS__16BIT(aptr + 4));

		status = ares_expand_name(aptr + 6, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t%s.", name.as_char);
		ares_free_string(name.as_char);
		break;

	case T_NAPTR:

		printf("\t%d", DNS__16BIT(aptr)); /* order */
		printf(" %d\n", DNS__16BIT(aptr + 2)); /* preference */

		p = aptr + 4;
		status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t\t\t\t\t\t%s\n", name.as_char);
		ares_free_string(name.as_char);
		p += len;

		status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t\t\t\t\t\t%s\n", name.as_char);
		ares_free_string(name.as_char);
		p += len;

		status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t\t\t\t\t\t%s\n", name.as_char);
		ares_free_string(name.as_char);
		p += len;

		status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
		if (status != ARES_SUCCESS)
			return NULL;
		printf("\t\t\t\t\t\t%s", name.as_char);
		ares_free_string(name.as_char);
		break;


	default:
		printf("\t[Unknown RR; cannot parse]");
		break;
	}
	printf("\n");

	return aptr + dlen;
}

void
DNS_Resolver::ares_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen)
{
	DNS_Resolver* instance = (DNS_Resolver*) arg;
	instance->func_(/* arg, */ status, timeouts, abuf, alen); // may take arg out if not needed!
}

DNS_Resolver::DNS_Resolver(struct ares_options* ares_options, int optmask)
:
status_(ARES_SUCCESS)
{
	// initialize c-ares library
	status_ = The_DNS_Resolver_Environment::instance()->status();
	if ( !is_status_ok() )
		return;

	// open channel
	struct ares_options options;
	options.flags = ARES_FLAG_NOCHECKRESP;
	options.servers = NULL;
	options.nservers = 0;
	if ( ares_options )
		options = *ares_options;

	status_ = ares_init_options(&channel_, &options, optmask);
	if ( !is_status_ok() )
		return;
}

DNS_Resolver::~DNS_Resolver()
{
	// close channel
	ares_destroy(channel_);
}

void
DNS_Resolver::async_query(const char* name, int rr_class, int rr_type, query_callback func)
{
	func_ = func;
	ares_query(channel_, name, rr_class, rr_type, ares_callback, this);
}

int
DNS_Resolver::wait_query(struct timeval& tv)
{
	// wait for all queries to complete
	int nfds, count;
	fd_set read_fds, write_fds;
	struct timeval *tvp;
	
	for (;;)
	{
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		nfds = ares_fds(channel_, &read_fds, &write_fds);
		if ( nfds == 0 )
			break;
		tvp = ares_timeout(channel_, NULL, &tv);
		count = select(nfds, &read_fds, &write_fds, NULL, tvp);
		if ( count < 0 && SOCKERRNO != EINVAL )
		{
			perror("select");
			return -1;
		}
		ares_process(channel_, &read_fds, &write_fds);
	}

	return 0;
}

void
DNS_Query::ares_callback(void* arg, int status, int timeouts, unsigned char* abuf, int alen)
{
	DNS_Query* query = (DNS_Query*) arg;
	query->callback(/* arg, */ status, timeouts, abuf, alen); // may take arg out if not needed!
}

void
DNS_Query::ask(DNS_Resolver& resolver)
{
	ares_query(resolver.channel(), name.c_str(), rr_class, rr_type, ares_callback, this);
}

} // namespace asio

