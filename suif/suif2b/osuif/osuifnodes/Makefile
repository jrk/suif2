### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:01 afikes Exp $

SRCS =		osuif.cpp \
		osuif_body.cpp
HEADERS =	osuif.h \
		osuif_factory.h \
		osuif_forwarders.h
OBJ_FILES = 	osuif.o \
		osuif_body.o

HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_DEP = 	$(NCIHOME)/suif/suif2b/basesuif/basicnodes/basic.hoof \
		$(NCIHOME)/suif/suif2b/basesuif/suifnodes/suif.hoof
HOOF_INCL = 	-I$(NCIHOME)/suif/suif2b/basesuif
SMNG_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac

HOOF = osuif
HOOF_GEN =	$(HEADERS) \
		osuif.cpp

TARGET_LIB = osuifnodes

LIBS += -ldl -lsuifnodes -lbasicnodes -lsuifkernel -liokernel -lcommon

DOCS = $(TARGET_DOC).ps
TARGET_DOC = osuif
NWSRCS = doc-intro.nw osuif.nw doc-end.nw

.SUFFIXES: .nw .hoof
.nw.hoof:
	notangle $< >$@

include ../Makefile.osuif
