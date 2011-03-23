#PASS_DRIVER = do_link_suif

MAIN_OBJ = standard_main.o

PASS_DRIVER = dummy_linksuif

TARGET_LIB = linksuif

OBJ_FILES = \
	ansic.o \
	merge_fsb.o \
	decl_groups.o \
	suif_linker.o \
	link_suif_pass.o \
	replace_map.o

SRCS = \
	ansic.cpp \
	merge_fsb.cpp \
	decl_groups.cpp \
	suif_linker.cpp \
	link_suif_pass.cpp \
	replace_map.cpp \
	standard_main.cpp

MAIN_OBJ = standard_main.o

HEADERS = \
	suif_linker.h \
	replace_map.h

LIBS += $(DYNAMIC_LINKING_LIBRARY) -lcommon -liokernel -lsuifkernel
LIBS += -lbasicnodes -lsuifnodes -ltypebuilder -lsuifprinter -lutils
LIBS += -lsuifcloning -lusefulpasses -lsuifpasses -lcfenodes

PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE
INCLDIRS +=  -I$(SUIFDIR)

include $(NCIHOME)/Makefile.std
