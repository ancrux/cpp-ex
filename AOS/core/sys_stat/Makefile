# Makefile for the sys_stat project
#
#---------------------------------------------------------
.SILENT:

ARCH          = linux

CXX           = 
ObjSuf        = o
SrcSuf        = cpp
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o 
#@
CSuf = c
#@

### @ my variables

#LIBS = -lTProactor_SSL -lACE_SSL -lTProactor -lACE -lssl -lcrypto
LIBS = -lACE
#IPATH = -I$(ACE_ROOT) -I$(AIO_ROOT) -I$(AOS_ROOT)
IPATH = -I$(ACE_ROOT) -I$(AOS_ROOT)
LPATH = -L$(ACE_ROOT)/lib -L$(AIO_ROOT)/lib
SOPATH = -Wl,-rpath,./ -Wl,-rpath,./lib -Wl,-rpath,../lib -Wl,-rpath,/usr/local/lib
OUTPATH = ./ 

ifeq ($(ARCH),hpux)
# HP-UX with CC
CXX           = CC
CXXFLAGS      = -O +Z
LD            = CC
LDFLAGS       = -O +a1 -z
SOFLAGS       = -b
DllSuf        = sl
endif

ifeq ($(ARCH),solaris)
# Solaris with CC
CXX           = CC
CXXFLAGS      = -O +Z
LD            = CC
LDFLAGS       = -O -z
SOFLAGS       = -b
DllSuf        = sl
endif

ifeq ($(ARCH),linux)
# Linux with gcc 2.7.2.x
CXX           = g++
CXXFLAGS      = -O -Wall -fPIC $(IPATH)
LD            = g++
LDFLAGS       = -O $(SOPATH) $(LPATH)
SOFLAGS       = -shared
#@
GCC = gcc
GCCFLAGS = -g -O2 -DHAVE_CONFIG_H
GCCFLAGS += $(DEBUG_OPTIONS)
#@
endif

ifeq ($(ARCH),mingw)
# MinGW g++ 3.0.6
CXX           = g++
CXXFLAGS      = -Wall
LD            = g++
LDFLAGS       =
SOFLAGS       = -shared
endif

ifeq ($(ARCH),freebsd)
# FreeBSD with libc5
CXX           = g++
CXXFLAGS      = -O -pipe -W -Wall -fPIC
LD            = g++
LDFLAGS       = -O
SOFLAGS       = -shared -Wl,-x
endif

ifeq ($(CXX),)
$(error $(ARCH) invalid architecture)
endif

CXXFLAGS     += $(DEBUG_OPTIONS)

#------------------------------------------------------------------------------


OBJFILES      = _main_.$(ObjSuf) \
                cpu_stats.$(ObjSuf) \
                disk_stats.$(ObjSuf) \
                error.$(ObjSuf) \
                load_stats.$(ObjSuf) \
                memory_stats.$(ObjSuf) \
                network_stats.$(ObjSuf) \
                os_info.$(ObjSuf) \
                page_stats.$(ObjSuf) \
                process_stats.$(ObjSuf) \
                statgrab_deprecated.$(ObjSuf) \
                swap_stats.$(ObjSuf) \
                tools.$(ObjSuf) \
                user_stats.$(ObjSuf) \
                vector.$(ObjSuf) \
                win32.$(ObjSuf) \
                $(AOS_ROOT)/aos/String.$(ObjSuf) \
                $(AOS_ROOT)/aos/IPC.$(ObjSuf) \
                System_Stat.$(ObjSuf) \
                Cello_Directory_Updater.$(ObjSuf)


#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:	sys_stat

sys_stat:   $(OBJFILES)
	@echo "Linking..."
	$(LD) $(LDFLAGS) $(notdir $(OBJFILES)) $(LIBS) \
	    -o sys_stat
	@echo "Done"

clean:
	@echo "Cleaning project area"
	@rm -f $(OBJFILES) core

.SUFFIXES: .$(SrcSuf)

#------------------------------------------------------------------------------

.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $<

#@
.$(CSuf).$(ObjSuf):
	@echo "C Compiling $<"
	$(GCC) $(GCCFLAGS) -c $<
#@


