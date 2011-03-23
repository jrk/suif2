SRCS = build_single_vtable.cpp globalize_class_methods.cpp globalize_class_variables.cpp porky.cpp main.cpp

OBJ_FILES = build_single_vtable.o globalize_class_methods.o globalize_class_variables.o porky.o

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcommon \
	-lsuifpasses -losuifnodes

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
#
test:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
#	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
#		$(NCI)/$(MACHINE)/bin/driver -f testporky.osuif2 
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testporky.osuif2 > hello2.txt
	@echo
	-diff -u hello2.txt hello.txt | tee d
	@echo
#	rm hello2.txt hello.txt

#
# 'make test1' to test vtable pass
#
test1:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testvtable.osuif2 > hello2.txt
	@echo
	-diff -u hello2.txt hello.txt | tee d
	@echo

#
# 'make test2' to test variables pass
#
test2:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testvar.osuif2 > hello2.txt
	@echo
	-diff -u hello2.txt hello.txt | tee d
	@echo

#
# 'make test3' to test methods pass
#
test3:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(NCI)/$(MACHINE)/bin/driver -f testmethods.osuif2 > hello2.txt
	@echo
	-diff -u hello2.txt hello.txt | tee d
	@echo

#
# 'make debug' to actually try this out under the debugger --
#    the preliminary run (of hello world) is done normally, but the
#    globalizer is done under gdb.  You'll have to manually supply
#    the command line args, of course.
debug:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		gdb $(OSUIFHOME)/bin/create_osuif_hello_world
	@echo
	@echo
	@echo Use command line arguments: -f testporky.osuif2
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		gdb $(NCI)/$(MACHINE)/bin/driver
