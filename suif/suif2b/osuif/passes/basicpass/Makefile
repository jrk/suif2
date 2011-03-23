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

#
# 'make test' to actually try this out
test:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testporky.osuif2 > hello2.txt
	@echo
	-diff hello2.txt hello.txt
	@echo
#	rm hello2.txt hello.txt

#
# 'make sample' is expected to fail because it does not load the
# osuif library.
#
sample:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	- LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testporky.suif2 > hello2.txt
	@echo
	@echo
	@echo THIS IS EXPECTED TO FAIL
	@echo     TRY make test INSTEAD
	@echo
#	-diff hello2.txt hello.txt
#	@echo
#	rm hello2.txt hello.txt
