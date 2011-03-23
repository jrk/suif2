### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

SRCS = 		class_hierarchy.cpp \
		init_utils.cpp \
		problems.cpp \
		expression_utils.cpp \
		symbol_utils.cpp \
		type_utils.cpp \
		inheritance_link_utils.cpp \
		attribute_utils.cpp \
		walker_utils.cpp \
		pass_utils.cpp \
		search_utils.cpp
HEADERS = 	class_hierarchy.h \
		init_utils.h \
		problems.h \
		expression_utils.h \
		symbol_utils.h \
		type_utils.h \
		inheritance_link_utils.h \
		attribute_utils.h \
		walker_utils.h \
		pass_utils.h \
		search_utils.h
OBJ_FILES =	class_hierarchy.o \
		init_utils.o \
		problems.o \
		expression_utils.o \
		symbol_utils.o \
		type_utils.o \
		inheritance_link_utils.o \
		attribute_utils.o \
		walker_utils.o \
		pass_utils.o \
		search_utils.o

TARGET_LIB = osuifutilities

# zero.h adlist_tos.cpp are not included in include/tos
# (used by class_hierarchy)
#INCLDIRS +=	-I$(NCIHOME)/suif/suif2b/extratypes

LIBS =	-losuiftypebuilder \
	-losuifnodes -losuifextensionnodes \
	-ltypebuilder -lutils -lsuifprinter \
	-lcfenodes -lsuifnodes -lbasicnodes -lsuifkernel \
	-liokernel -lcommon \
	-lsuifcloning

#	-ltos -lion 

include ../Makefile.osuif
