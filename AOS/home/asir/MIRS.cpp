#include "MIRS.h"

ACE_UINT32
IPv4::to_uint32(const std::string& ipv4)
{
	ACE_UINT32 ip = 0;
	do
	{
		size_t pos = 0;
		int ia = ACE_OS::atoi(&ipv4[pos]);
		if ( ia == 0 && ipv4[pos] != '0' )
			break;

		pos = ipv4.find('.', pos);
		if ( pos == std::string::npos )
			break;
		++pos;
		int ib = ACE_OS::atoi(&ipv4[pos]);
		if ( ib == 0 && ipv4[pos] != '0' )
			break;

		pos = ipv4.find('.', pos);
		if ( pos == std::string::npos )
			break;
		++pos;
		int ic = ACE_OS::atoi(&ipv4[pos]);
		if ( ic == 0 && ipv4[pos] != '0' )
			break;

		pos = ipv4.find('.', pos);
		if ( pos == std::string::npos )
			break;
		++pos;
		int id = ACE_OS::atoi(&ipv4[pos]);
		if ( id == 0 && ipv4[pos] != '0' )
			break;

		ip = ia * 16777216 + ib * 65536 + ic * 256 + id;
	}
	while(0);

	return ip;
}

std::string
IPv4::to_string(ACE_UINT32 ipv4)
{
	char buf[32];

	int ia = ipv4 >> 24;
	int ib = (ipv4 >> 16) & 0x000000FF;
	int ic = (ipv4 >> 8) & 0x000000FF;
	int id = ipv4 & 0x000000FF;

	int n_buf = ACE_OS::snprintf(buf, 31, "%d.%d.%d.%d", ia, ib, ic, id);

	std::string ip;
	if ( n_buf > 0 ) ip.assign(buf);

	return ip;
}
