
TARGET_LIB = tclsuif

OBJ_FILES = \
	suifInit.o \
	suif_utils.o \
	tcl_token_stream.o \
	suifCmd.o

SRCS = \
	suifInit.cpp \
	suif_utils.cpp \
	tcl_token_stream.cpp \
	suifCmd.cpp

#HEADERS = \

LIBS += $(DYNAMIC_LINKING_LIBRARY)
LIBS += $(TCL_LIBDIRS) 
LIBS += -lcommon -lutils -lsuifkernel -lsuifpasses



PREPROCESSORFLAGS += -DUNIX_DLL_INTERFACE
INCLDIRS += $(TCL_INCLDIRS)

include $(NCIHOME)/Makefile.std
