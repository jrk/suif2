### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

SRCS =		vtbl.cpp \
		vtbl_pass.cpp \
		vtbl_annote_pass.cpp \
		slot_annote_pass.cpp \
		slot_count_annote_pass.cpp \
		instance_method_call_pass.cpp \
		instance_method_call_utils.cpp
#main.cpp
HEADERS =	vtbl.h \
		vtbl_pass.h \
		vtbl_annote_pass.h \
		slot_annote_pass.h \
		slot_count_annote_pass.h \
		instance_method_call_pass.h \
		instance_method_call_utils.h
OBJ_FILES = 	vtbl.o \
		vtbl_pass.o \
		vtbl_annote_pass.o \
		slot_annote_pass.o \
		slot_count_annote_pass.o \
		instance_method_call_pass.o \
		instance_method_call_utils.o
TARGET_LIB = vtblpass

#PASS_DRIVER = vtbl
#MAIN_OBJ = main.o

LIBS =	-lvtblnodes \
	-losuifutilities -losuifnodes -losuifextensionnodes \
	-lsuifpasses -ltypebuilder \
	-lcfenodes -lsuifnodes -lbasicnodes \
	-lsuifcloning -lsuifkernel -liokernel -lcommon \
	-ltos -lion \
	-ldl


include ../../Makefile.osuif
