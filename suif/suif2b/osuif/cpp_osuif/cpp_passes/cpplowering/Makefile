
SRCS =		dismantle_cpp_vtables.cpp \
		cpp_transforms.cpp
HEADERS = 	dismantle_cpp_vtables.h \
		cpp_transforms.h
OBJ_FILES =	dismantle_cpp_vtables.o \
		cpp_transforms.o

LIBS += -lcpp_osuifnodes -losuifnodes -lbasicnodes -lsuifnodes -lsuifkernel -lcommon \
	-liokernel -lsuifpasses -ltos -lion -losuifutilities -ltypebuilder \
	-lsuifcloning

TARGET_LIB = cpplowering

include $(NCIHOME)/Makefile.std
