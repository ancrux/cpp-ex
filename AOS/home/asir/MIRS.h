#ifndef _MIRS_H_
#define _MIRS_H_

#include "ace/OS.h"

#include "stx/btree_map" // namespace stx
typedef stx::btree_map< ACE_UINT32, unsigned char > MIRS_MAP;
#include <vector>
typedef std::vector< MIRS_MAP* > MIRS_MAPS;
#include <fstream> // used with btree dump() and restore()

class IPv4
{
public:
	static ACE_UINT32 to_uint32(const std::string& ipv4);
	static std::string to_string(ACE_UINT32 ipv4);

};

#endif // _MIRS_H_
