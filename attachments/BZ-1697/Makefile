#----------------------------------------------------------------------------
#
# $Id: Makefile,v 1.10 2003/03/14 20:30:42 ossama Exp $
#
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	Local macros
#----------------------------------------------------------------------------

ifndef TAO_ROOT
  TAO_ROOT = $(ACE_ROOT)/TAO
endif # ! TAO_ROOT

LDLIBS = -lTAO_SmartProxies -lTAO
LDFLAGS += -L$(ACE_ROOT)/bin

IDL_FILES = test
IDL_SRC = testC.cpp testS.cpp
BIN = client server
FILES = Smart_Proxy_Impl

SRC = $(addsuffix .cpp, $(BIN) $(FILES)) $(IDL_SRC)

CLIENT_OBJS = testC.o client.o Smart_Proxy_Impl.o
SERVER_OBJS = $(IDL_SRC:.cpp=.o) server.o  Smart_Proxy_Impl.o

TAO_IDLFLAGS += -Ge 1 -Gsp

#----------------------------------------------------------------------------
#	Include macros and targets
#----------------------------------------------------------------------------
include $(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include $(ACE_ROOT)/include/makeinclude/macros.GNU
include $(TAO_ROOT)/rules.tao.GNU
include $(ACE_ROOT)/include/makeinclude/rules.common.GNU
include $(ACE_ROOT)/include/makeinclude/rules.nested.GNU
include $(ACE_ROOT)/include/makeinclude/rules.local.GNU
include $(TAO_ROOT)/taoconfig.mk

#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

LDFLAGS += -L$(TAO_ROOT)/tao

.PRECIOUS: testC.h testC.i testC.cpp
.PRECIOUS: testS.h testS.i testS.cpp

server: $(addprefix $(VDIR),$(SERVER_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(TAO_SRVR_LIBS) $(POSTLINK)

client: $(addprefix $(VDIR),$(CLIENT_OBJS))
	$(LINK.cc) $(LDFLAGS) -o $@ $^ $(VLDLIBS) $(POSTLINK)

realclean: clean
	-$(RM) $(foreach ext, $(IDL_EXT), test$(ext))

