SRCS = dismantle_if.cpp porky.cpp main.cpp

OBJ_FILES = dismantle_if.o porky.o

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcommon \
	-lsuifpasses

TARGET_LIB = porky

EXTRA_CXXFLAGS += -I$(SUIF2B)/basesuif/suifpasses
EXTRA_CXXFLAGS += -I$(SUIF2B)/basesuif/suifnodes
EXTRA_CXXFLAGS += -I$(SUIF2B)/basetypes
EXTRA_CXXFLAGS += -I$(SUIF2B)/rinds
#EXTRA_CXXFLAGS += -I/sow/1/1/suifhome/i586-linux/include

PASS_DRIVER = do_porky
MAIN_OBJ = main.o

include $(NCIHOME)/Makefile.std

CC = gcc
CXX = g++
AWK = nawk

test:
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/create_suif_hello_world > hello.txt
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testporky.suif2 > hello2.txt
	-diff hello2.txt hello.txt
#	rm hello2.txt hello.txt
