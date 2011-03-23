### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

SRCS =		instancefieldslayout.cpp \
		instancefieldslayout_body.cpp \
		instancefieldslayout_utils.cpp
HEADERS =	instancefieldslayout.h \
		instancefieldslayout_factory.h \
		instancefieldslayout_forwarders.h \
		instancefieldslayout_utils.h
OBJ_FILES = 	instancefieldslayout.o \
		instancefieldslayout_body.o \
		instancefieldslayout_utils.o

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP = 	$(NCIHOME)/suif/suif2b/basesuif/basicnodes/basic.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/suifnodes/suif.hoof \
		$(OSUIFHOME)/osuifnodes/osuif.hoof \
		$(OSUIFHOME)/osuifextensionnodes/osuifextension.hoof
HOOF_INCL = 	-I$(NCIHOME)/suif/suif2b/basesuif \
		-I$(OSUIFHOME)
SMNG_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac

HOOF = instancefieldslayout
HOOF_GEN =	instancefieldslayout.h \
		instancefieldslayout_factory.h \
		instancefieldslayout_forwarders.h \
		instancefieldslayout.cpp

TARGET_LIB = instancefieldslayoutnodes

LIBS =	-losuiftypebuilder -losuifutilities \
	-losuifnodes -losuifextensionnodes \
	-ltypebuilder -lutils -lsuifprinter \
	-lcfenodes -lsuifnodes -lbasicnodes -lsuifkernel \
	-liokernel -lcommon \
	-lsuifcloning


include ../../Makefile.osuif
