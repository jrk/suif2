### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:01 afikes Exp $

SRCS =		osuifextension.cpp \
		osuifextension_body.cpp
HEADERS =	osuifextension.h \
		osuifextension_factory.h \
		osuifextension_forwarders.h
OBJ_FILES = 	osuifextension.o \
		osuifextension_body.o

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP = 	$(NCIHOME)/suif/suif2b/basesuif/basicnodes/basic.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/suifnodes/suif.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/cfenodes/cfe.hoof \
		$(OSUIFHOME)/osuifnodes/osuif.hoof
HOOF_INCL = 	-I$(NCIHOME)/suif/suif2b/basesuif \
		-I$(OSUIFHOME)
SMNG_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac

HOOF = osuifextension
HOOF_GEN =	$(HEADERS) \
		osuifextension.cpp

TARGET_LIB = osuifextensionnodes

LIBS += -losuifnodes \
	-ldl -lcfenodes -lsuifnodes -lbasicnodes \
	-lsuifkernel -liokernel -lcommon

DOCS = $(TARGET_DOC).ps
TARGET_DOC = osuifextension
NWSRCS = doc-intro.nw osuifextension.nw doc-end.nw

.SUFFIXES: .nw .hoof
.nw.hoof:
	notangle $< >$@

include ../Makefile.osuif
