# Makefile
HEADERS = cpp_symbol_walkers.h cpp_transforms.h
SRCS = cpp_symbol_walkers.cpp cpp_transforms.cpp
OBJ_FILES = cpp_symbol_walkers.o cpp_transforms.o
TARGET_LIB = cpp_osuifnodes

INCLDIRS += -I.. -I$(INCDIR)

LIBS += $(DYNAMIC_LINKING_LIBRARY) -ltypebuilder -lsuifpasses -lcfenodes -lsuifnodes -lbasicnodes \
        -lsuifkernel  -liokernel -lcommon -losuifnodes -lcpp_osuifnodes -ltransforms

TARGET_LIB = cpp_transforms

include $(NCIHOME)/Makefile.std
