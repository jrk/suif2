### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

SRCS =		lowering_pass.cpp \
		lowering.cpp \
		static_method_call_utils.cpp \
		static_method_call_pass.cpp \
		definition_block_pass.cpp \
		main.cpp
HEADERS =	lowering_pass.h \
		lowering.h \
		static_method_call_utils.h \
		static_method_call_pass.h \
		definition_block_pass.h
OBJ_FILES =	lowering_pass.o \
		lowering.o \
		static_method_call_utils.o \
		static_method_call_pass.o \
		definition_block_pass.o

TARGET_LIB = osuiflowering

PASS_DRIVER = osuiflowering

MAIN_OBJ = main.o

LIBS =	-losuifutilities -losuiftypebuilder \
	-losuifnodes -losuifextensionnodes \
	-lutils -ltypebuilder -lsuifprinter \
	-lbasicnodes -lsuifnodes -lcfenodes \
	-lsuifkernel -lcommon \
	-liokernel -lsuifpasses \
	-lsuifcloning \
	-ldl


include ../../Makefile.osuif
