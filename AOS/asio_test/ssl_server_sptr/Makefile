# Makefile for the ssl_server_sptr project
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
LIBS = -lACE -lssl -lcrypto
#IPATH = -I$(ACE_ROOT) -I$(AIO_ROOT) -I$(AOS_ROOT) 
IPATH = -I$(ACE_ROOT) -I$(AOS_ROOT) -I/home/angus/openssl-0.9.8i/include -I/home/angus/boost_1_41_0/ -I/home/angus/AOS/asio/ 
LPATH = -L$(ACE_ROOT)/lib -L/home/angus/openssl-0.9.8i
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
                SSL_Server.$(ObjSuf) \
                SSL_Server_Connection.$(ObjSuf) \
                HTTP_Server.$(ObjSuf) \
                HTTP_Server_Connection.$(ObjSuf)

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all:	ssl_server_sptr

ssl_server_sptr:   $(OBJFILES)
	@echo "Linking..."
	$(LD) $(LDFLAGS) $(notdir $(OBJFILES)) $(LIBS) \
	    -o ssl_server_sptr
	@echo "Done"

clean:
	@echo "Cleaning project area"
	@rm -f $(OBJFILES) core

.SUFFIXES: .$(SrcSuf)

#------------------------------------------------------------------------------

.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $<

