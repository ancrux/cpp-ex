#ifndef _CONFIG_H_
#define _CONFIG_H_

#define AOS // AOS_LIB
#define AOS_VERSION "1.0"

#if defined ( AOS_HAS_DLL ) // WIN32
#	ifdef AOS_BUILD_DLL
#		define AOS_API	__declspec(dllexport) // export DLL information
#	else
#		define AOS_API	__declspec(dllimport) // import DLL information
#	endif
#else
#	define AOS_API // no DLL available
#endif

#if defined ( _MSC_VER )
#pragma warning(disable: 4251) // disable Visual Studio warning for STL template class/container
#endif

// for more ACE Macros, check ace/config-lite.h, ace/config-macro.h, ace/config.h
#if defined ( ACE_VERSION )
#	define __ACE__
#endif

#include "aos/Macro.h"

namespace aos {

} // namespace aos

#endif // _CONFIG_H_
