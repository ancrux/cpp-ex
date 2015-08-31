#ifndef _MIME_ANALYZER_H_
#define _MIME_ANALYZER_H_

#include "aos/Config.h"

#include "aos/mime/MIME_Util.h"
#include "aos/hash/FNV.h"
//#include "hash/fnv.h"

#include <set>

namespace aos {

class MIME_Analyzer
{
public: //? moved to MIME_Analyzer
	static int has_url(MIME_Entity& e);
	static void get_url(MIME_Entity& e, std::set< std::string >& url_set);
	static int get_phone(MIME_Entity& e);
	static void dump_body(MIME_Entity& e);

public: //? moved to MIME_Analyzer
	static ACE_UINT32 get_hash(MIME_Entity& e);
	static ACE_UINT32 hash(MIME_Entity& e, ACE_UINT32 hash_val);
	//static Fnv32_t get_hash2(MIME_Entity& e);
	//static Fnv32_t hash2(MIME_Entity& e, Fnv32_t hash_val);
};

} // namespace aos

#endif // _MIME_ANALYZER_H_

