### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

SRCS =		vtbl.cpp \
		vtbl_body.cpp \
		vtbl_utils.cpp
HEADERS =	vtbl.h \
		vtbl_factory.h \
		vtbl_forwarders.h \
		vtbl_utils.h
OBJ_FILES = 	vtbl.o \
		vtbl_body.o \
		vtbl_utils.o

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP = 	$(NCIHOME)/suif/suif2b/basesuif/basicnodes/basic.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/suifnodes/suif.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/cfenodes/cfe.hoof \
		$(OSUIFHOME)/osuifnodes/osuif.hoof \
		$(OSUIFHOME)/osuifextensionnodes/osuifextension.hoof
HOOF_INCL = 	-I$(NCIHOME)/suif/suif2b/basesuif \
		-I$(OSUIFHOME)
SMNG_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac

HOOF = vtbl
HOOF_GEN =	vtbl.h \
		vtbl_factory.h \
		vtbl_forwarders.h \
		vtbl.cpp

TARGET_LIB = vtblnodes

LIBS =	-losuifutilities \
	-losuifnodes -losuifextensionnodes \
	-ltypebuilder -lutils \
	-lcfenodes -lsuifnodes -lbasicnodes -lsuifkernel \
	-liokernel -lcommon \
	-lsuifcloning \
	-ldl


include $(OSUIFHOME)/Makefile.osuif
