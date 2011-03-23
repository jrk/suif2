### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

SRCS =		main.cpp \
		instancefieldslayout.cpp \
		instancefieldslayout_pass.cpp \
		offset_annote_pass.cpp
HEADERS =	instancefieldslayout.h \
		instancefieldslayout_pass.h \
		offset_annote_pass.h
OBJ_FILES = 	instancefieldslayout.o \
		instancefieldslayout_pass.o \
		offset_annote_pass.o

TARGET_LIB = instancefieldslayoutpass

PASS_DRIVER = instancefieldslayout

MAIN_OBJ = main.o

LIBS =	-linstancefieldslayoutnodes \
	-losuifutilities -losuifnodes -losuifextensionnodes \
	-lsuifpasses -ltypebuilder -lutils -lsuifprinter \
	-lcfenodes -lsuifnodes -lbasicnodes \
	-lsuifcloning -lsuifkernel -liokernel -lcommon


include ../../Makefile.osuif
