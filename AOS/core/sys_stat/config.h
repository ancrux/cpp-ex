#ifndef CONFIG_H
#define CONFIG_H

#ifdef WIN32
#include "config_win32.h"
#endif

#ifdef linux
#include "config_linux.h"
#endif

#endif
