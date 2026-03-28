#----------------------------------------------------------------------------
#	Local macros
#----------------------------------------------------------------------------
BIN   	= server
#LIBS	+=-lmputil -lmpalarm_i

#----------------------------------------------------------------------------
#	Should not have to change anything below
#----------------------------------------------------------------------------


# include ${MP_SRC}/Makefile.iace

include ${ACE_ROOT}/include/makeinclude/wrapper_macros.GNU
include ${ACE_ROOT}/include/makeinclude/macros.GNU
include ${TAO_ROOT}/rules.tao.GNU

SRC	= $(wildcard *.cpp)
CLEANUP_OBJS= $(SRC)

include ${ACE_ROOT}/include/makeinclude/rules.common.GNU
include ${ACE_ROOT}/include/makeinclude/rules.nonested.GNU
ifdef BIN
include ${ACE_ROOT}/include/makeinclude/rules.bin.GNU
BUILD = $(VBIN)
endif
ifdef SHLIB
include ${ACE_ROOT}/include/makeinclude/rules.lib.GNU
BUILD = $(VLIB)
endif
LIB_INSTALL=
include ${ACE_ROOT}/include/makeinclude/rules.local.GNU

.PRECIOUS: $(foreach ext, $(IDL_EXT), $(IDL_FILES)$(ext))

#----------------------------------------------------------------------------
#	Dependencies
#----------------------------------------------------------------------------

# DO NOT DELETE THIS LINE -- g++dep uses it.
# DO NOT PUT ANYTHING AFTER THIS LINE, IT WILL GO AWAY.

# IF YOU PUT ANYTHING HERE IT WILL GO AWAY
