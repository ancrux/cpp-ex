#ifndef _MBOX_CONFIGURATION_H_
#define _MBOX_CONFIGURATION_H_

#include <string>
#include <vector>

struct MBox_Configuration
{
public:
	int load(const char* path);

public:
	std::string path_prefix;
	std::string path_mbox_base;
	std::vector< std::string > path_spools;
};

#endif // _MBOX_CONFIGURATION_H_

