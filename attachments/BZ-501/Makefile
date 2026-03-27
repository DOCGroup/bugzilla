#----------------------------------------------------------------------------
#
#	$Id: Makefile,v 1.9 1998/05/27 22:56:34 schmidt Exp $
#
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	Local macros
#----------------------------------------------------------------------------

# List all the IDL sources (IDL_SRCS) that will be used by client and
# server.
IDL_SRCS = Echo

# This generates the basenames of stub and skeleton files generated from
# the IDL sources.
IDL_FILES = \
        $(addsuffix S, $(IDL_SRCS)) \
        $(addsuffix C, $(IDL_SRCS))

# This generates the names of all the header and source files to build.
FILES = $(IDL_FILES)
DEFS  = $(addsuffix .h,$(FILES)) Echo_i.h
SRC   = $(addsuffix .cpp,$(FILES)) EchoServer.cpp DefaultEcho_i.cpp client.cpp

# List the libraries with which to link.
LDLIBS = -lTAO

# List the object files that make up the server (SVR) and client (CLT)
FILE_SVR_OBJS = $(addsuffix .o,$(IDL_FILES)) EchoServer.o DefaultEcho_i.o
FILE_CLT_OBJS = $(addsuffix .o,$(IDL_FILES)) client.o

# This defines the binary executable targets to build.
BIN = EchoServer client

#----------------------------------------------------------------------------
#       Include macros and targets
#----------------------------------------------------------------------------

include $(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include $(ACE_ROOT)/include/makeinclude/macros.GNU

include $(TAO_ROOT)/rules.tao.GNU
include $(ACE_ROOT)/include/makeinclude/rules.common.GNU
include $(ACE_ROOT)/include/makeinclude/rules.nonested.GNU
# include $(ACE_ROOT)/include/makeinclude/rules.bin.GNU
include $(ACE_ROOT)/include/makeinclude/rules.local.GNU
include $(TAO_ROOT)/taoconfig.mk

# Add this to include path so we can #include <corba.h> instead of
# #include <tao/corba.h>
INCLDIRS += -I$(TAO_ROOT)/tao

# idl_stubs: $(addsuffix .h, $(IDL_FILES))

EchoServer: $(addprefix $(VDIR),$(FILE_SVR_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

client: $(addprefix $(VDIR),$(FILE_CLT_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

realclean: clean
	-/bin/rm -rf EchoC.* EchoS.* EchoS_T.*

# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.



# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
