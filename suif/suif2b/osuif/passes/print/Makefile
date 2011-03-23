### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

SRCS =		print_pass.cpp \
		printing_maps.cpp \
		osuif_visitor_info.cpp \
		print.cpp \
		main.cpp
HEADERS = 	print_pass.h \
		printing_maps.h \
		osuif_visitor_info.h \
		print.h
OBJ_FILES = 	print_pass.o \
		printing_maps.o \
		osuif_visitor_info.o \
		print.o

TARGET_LIB = osuifprint

LIBS += -lsuifnodes -lsuifkernel -liokernel -lbasicnodes -lcommon \
	-lsuifpasses -losuifnodes

PASS_DRIVER = pretty_print
PROGRAM = pretty_print
MAIN_OBJ =  main.o

include ../../Makefile.osuif



print_test:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/create_osuif_hello_world > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/pretty_print -f testporky.osuif2 > hello2.txt
	@echo
#	diff hello2.txt hello.txt
	@echo
#	rm hello2.txt hello.txt

j2s_sample:
	echo $(LD_LIBRARY_PATH)
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/j2s -f testj2s.osuif > hello.txt
	@echo
	LD_LIBRARY_PATH=$(NCI)/$(MACHINE)/solib:$(LD_LIBRARY_PATH) ; \
		$(OSUIFHOME)/bin/pretty_print -f testporky.j2s > hello2.txt
	@echo
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
