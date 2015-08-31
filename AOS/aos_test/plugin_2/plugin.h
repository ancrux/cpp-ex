#ifndef _USER_PLUGIN_H_
#define _USER_PLUGIN_H_

#if defined ( WIN32 )
#	ifdef PLUGIN_2_EXPORTS
#		define PLUGIN_API	__declspec(dllexport) // export DLL information
#	else
#		define PLUGIN_API	__declspec(dllimport) // import DLL information
#	endif
#else
#	define PLUGIN_API
#endif

#include "../plugin_loader/My_Interface.h"

#include <string>

class My_Plugin_2 : My_Plugin_Interface
{
public:
	// base class
	virtual const char* plugin_name();
	virtual const char* plugin_version();

	// derived class
	virtual void process(std::string& str);
	//virtual My_Plugin_Interface* clone();

public:
	virtual ~My_Plugin_2()
	{
		::printf("DTOR: ~My_Plugin_2\n");
	};
};

#ifdef __cplusplus
extern "C" {
#endif

//PLUGIN_API void register_plugin(My_Plugin_Manager& pm);
//PLUGIN_API const char* engine_version();
PLUGIN_API My_Plugin_Interface* create_instance();

std::string& ltrim(std::string& str, const char* trim);
std::string& rtrim(std::string& str, const char* trim);
inline std::string& trim(std::string& str, const char* trim) { return ltrim(rtrim(str, trim), trim); };

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // _USER_PLUGIN_H_

