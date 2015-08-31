# Makefile for the plugin_2 project
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

### @ my variables

#LIBS = -lTProactor_SSL -lACE_SSL -lTProactor -lACE -lssl -lcrypto
LIBS = -licuuc -licuio -lboost_regex-gcc-1_35 -lACE_SSL -lACE -lssl -lcrypto
#IPATH = -I$(ACE_ROOT) -I$(AIO_ROOT) -I$(AOS_ROOT)
IPATH = -I$(ACE_ROOT) -I$(AOS_ROOT) -I/home/angus/boost_1_35_0
LPATH = -L$(ACE_ROOT)/lib -L/home/angus/boost_1_35_0/libs/regex/build/gcc
SOPATH = -Wl,-rpath,./ -Wl,-rpath,./lib -Wl,-rpath,/usr/local/lib
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
SOFLAGS       = -shared -Wl,-soname,plugin_2.so
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


OBJFILES      = plugin.$(ObjSuf) \
				../plugin_loader/My_Interface.$(ObjSuf)


#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:	plugin_2

plugin_2:   $(OBJFILES)
	@echo "Linking..."
	$(LD) $(SOFLAGS) $(LDFLAGS) $(notdir $(OBJFILES)) $(LIBS) \
	    -o ../plugin_dir/plugin_2.so 
	@echo "Done"

clean:
	@echo "Cleaning project area"
	@rm -f $(OBJFILES) core

.SUFFIXES: .$(SrcSuf)

#------------------------------------------------------------------------------

.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $<

