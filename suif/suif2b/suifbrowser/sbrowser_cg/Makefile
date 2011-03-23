TARGET_LIB =	sbrowser_cg
LIBS =	        -lsuifnodes -lbasicnodes -lutils -lcfenodes -lcfeutils
HEADERS =	sbrowser_cg.h cnode.h cgraph.h 
SRCS =		cgraph.cpp cnode.cpp 
OBJ_FILES =		cgraph.o cnode.o

EXTRA_CXXFLAGS =  $(TCL_INCLDIRS)
LIBS += $(TCL_LIBDIRS)
#EXTRA_CXXFLAGS += -I.
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/suifbrowser 
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/suifbrowser/viewers
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/basetypes 
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/basesuif/suifkernel
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/basesuif/iokernel
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/basesuif/suifnodes
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/basesuif/basicnodes
#EXTRA_CXXFLAGS += -I$(NCIHOME)/suif/suif2b/utils
EXTRA_CXXFLAGS += -I/usr/local/ultra/lib 
#EXTRA_CXXFLAGS += -L$(NCIHOME)/solib

include $(NCIHOME)/Makefile.std
