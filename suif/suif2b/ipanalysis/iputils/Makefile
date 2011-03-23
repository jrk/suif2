PASS_DRIVER = do_iputils

HOOF = iputils
HOOF_HEADERS = 	$(HOOF).h $(HOOF)_forwarders.h $(HOOF)_factory.h

SRCS = lib_fileset_util.cpp \
	iputils.cpp iputils_extras.cpp iputils_query.cpp \
	iputils_init.cpp standard_main.cpp

HEADERS = lib_fileset_util.h \
	iputils.h iputils_forwarders.h iputils_factory.h iputils_query.h


OBJ_FILES = lib_fileset_util.o \
	iputils.o iputils_extras.o iputils_query.o \
	iputils_init.o

MAIN_OBJ = standard_main.o

# Hoof section

BASESRC = ../../basesuif

SMGN_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac

HOOF_GEN = $(HOOF_HEADERS) $(HOOF).cpp
HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif
HOOF_DEP = $(BASESRC)/basicnodes/basic.hoof

LIBS += -lcommon -liokernel -lsuifkernel -lsuifpasses \
	-lbasicnodes -lsuifnodes -lutils -ltypebuilder -lsuifcloning

TARGET_LIB = iputils

include $(NCIHOME)/Makefile.std
