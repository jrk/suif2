
TARGET_LIB = mark_lib

PASS_DRIVER = dummy_mark_lib

MAIN_OBJ = standard_main.o

OBJ_FILES = \
	proc_table.o \
	mark_lib_pass.o \
	mark_lib_query.o \
	standard_main.o

SRCS = \
	proc_table.cpp \
	mark_lib_pass.cpp \
	mark_lib_query.cpp \
	standard_main.cpp

HEADERS = mark_lib_query.h

#LIBS += -L$(GC_LIB_DIR) -lgc $(DYNAMIC_LINKING_LIBRARY)
LIBS += -lcommon -liokernel -lsuifkernel \
	-lbasicnodes -lsuifnodes -lutils -lsuifprinter -lutils -liokernel  

#PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE

include $(NCIHOME)/Makefile.std
