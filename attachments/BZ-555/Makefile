# creates test for 3 platforms: HP-UX, AIX and Linux

#  item		    | HP-UX            | Linux		  | AIX
#
#  CXX              | /opt/aCC/bin/aCC | gcc		  | xlC_r
#  Pthreads	    | Draft4	       | Final		  | Final
#  lib. pthreads    | dce or cma       | pthread	  | pthreads
#  yield	    | pthread_yield    | sched_yield	  | pthread_yield

OS=$(shell uname -s)

ifeq ($(OS),HP-UX)

#CXX = gcc
CXX = /opt/aCC/bin/aCC
CXXFLAGS = -DPTHREAD_DRAFT4
LIBS = -lcma

else
ifeq ($(OS),Linux)

CXX = gcc
CXXFLAGS = -DPTHREAD_FINAL -DHAVE_SCHED_YIELD
LIBS = -lpthread

else

ifeq ($(OS),AIX)

CXX = xlC_r
CXXFLAGS = -DPTHREAD_FINAL
ifeq ($(CXX),xlC_r)
# xlC_r lacks 'bool' type
CXXFLAGS += -Dbool=int
else
endif
LIBS = 

else

#unknown OS

endif
endif
endif

TEST=thread
EXE=$(TEST:=-$(OS).test)
OBJ=$(EXE:.test=.o)
SRC=$(TEST:=.cpp)

.PHONY: all clean check

all: $(EXE)

clean:
	$(RM) $(EXE) $(OBJ) core

check : $(EXE)
	./$(EXE)

%-$(OS).o : %.cpp
	$(CXX) ${CXXFLAGS}  -c $< -o $@

%.test : %.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
