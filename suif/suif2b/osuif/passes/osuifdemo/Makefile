SRCS = osuif_demo_pass.cpp porky.cpp main.cpp

OBJ_FILES = osuif_demo_pass.o porky.o

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcommon \
	-lsuifpasses -lrinds -losuifnodes

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
		$(OSUIFHOME)/bin/create_osuif_hello_world 
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testporky.osuif2 
	@echo
