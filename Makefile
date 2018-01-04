DEBUG = 1
CC  = gcc
CXX = g++
GCCVER := $(shell $(CC) -dumpversion | awk -F. '{ print $$1"."$$2}' )
GOOGELBUFERDIR := /usr/local/protobuf
GAMEBASEDIR=/usr/local/gamebase
APILIBDIR=/usr/local/apilib
BOOSTDIR=/usr/local/boost
HIREDISDIR=/usr/local/hiredis

CORE_INC= -I./process  -I./pb  -I./ -I./src/
COMPILE_LIB_HOME ?=./
GAMEBASE_INC= $(GAMEBASEDIR)/include
GOOGLEBUFER_INC = $(GOOGELBUFERDIR)/include
APILIB_INC=$(APILIBDIR)/include
BOOST_INC=$(BOOSTDIR)/include 
HIREDIS_INC=$(HIREDISDIR)/include

INC     =  $(CORE_INC) -I$(GAMEBASE_INC) -I$(GOOGLEBUFER_INC) -I$(APILIB_INC) -I$(BOOST_INC) -I$(HIREDIS_INC)
OPT     = -pipe -fno-ident -fPIC -shared -z defs
LINK_APILIB = -L$(GOOGLEBUFER_INC)/lib -L$(GAMEBASEDIR)/lib -lgamebase -L$(GOOGELBUFERDIR)/lib -lprotobuf-lite -lprotobuf -L$(APILIBDIR)/lib -lapilib -L$(BOOSTDIR)/lib -lboost_system


LINK    = -lpthread $(LINK_APILIB)

CFLAGS +=  -m64  -pg -Wall -D_GNU_SOURCE -funroll-loops -MMD -D_REENTRANT -std=c++11 -fPIC -Wno-unused
# $(MYSQL_FLAAGS)

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG -g
endif

CXXFLAGS := $(CFLAGS)
CORE_SRC=$(wildcard src/*.cpp ) 
SRC_PB = $(wildcard pb/*.cc)
SRC_CPP =  $(wildcard ./*.cpp )  $(wildcard process/*.cpp)  $(CORE_SRC)
SRC_C = $(wildcard  ./*.c)

DYNAMIC_NAME = libKartingServer.so
STATIC_NAME  = libKartingServer.a

DYNAMIC_LIB = $(COMPILE_LIB_HOME)/$(DYNAMIC_NAME)
STATIC_LIB = $(COMPILE_LIB_HOME)/$(STATIC_NAME)

all: $(DYNAMIC_LIB) 

$(DYNAMIC_LIB): $(SRC_CPP:.cpp=.o) $(SRC_PB:.cc=.o) $(SRC_C:.c=.o)
	$(CXX)  -o $@ $^ $(CXXFLAGS) $(LINK) -shared  

$(STATIC_LIB): $(SRC_CPP:.cpp=.o) $(SRC_C:.c=.o) $(SRC_PB:.cc=.o)
	@ar cr $@ $^
#	cp $(STATIC_LIB) .	

%.o: %.c Makefile 
	$(CC) $(CFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@
%.o: %.cc Makefile  
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@
%.o: %.cpp Makefile 
	$(CXX) $(CXXFLAGS) $(INC) -c -pg -o $@ $<
#	@-mv -f $*.d .dep.$@

install:
	cp -rf $(DYNAMIC_NAME) ./bin/$(DYNAMIC_NAME)

clean:
	rm -f *.o .po *.so *.d .dep.* $(SRC_C:.c=.o) $(SRC_C:.c=.d)  $(SRC_CPP:.cpp=.o) $(SRC_CPP:.cpp=.d)  $(DYNAMIC_NAME) $(STATIC_NAME) $(SRC_PB:.cc=.o) $(SRC_PB:.cc=.d)
-include /dev/null $(wildcard .dep.*) 
