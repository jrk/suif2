BASESRC = ../../basesuif

PASS_DRIVER = do_ecrnodes


SMGN_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac
HOOF = ecr
HOOF_GEN = $(HOOF).cpp $(HOOF)_forwarders.h $(HOOF).h $(HOOF)_factory.h
HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif
HOOF_DEP = $(BASESRC)/basicnodes/basic.hoof

MAIN_OBJ = standard_main.o

SRCS = ecr.cpp ecr_constants.cpp ecr_query.cpp ecr_extras.cpp \
	standard_main.cpp

OBJ_FILES = ecr.o ecr_constants.o ecr_query.o ecr_extras.o

HEADERS = $(HOOF).h $(HOOF)_constants.h $(HOOF)_query.h \
	$(HOOF)_factory.h $(HOOF)_forwarders.h

LIBS += -liputils -lcommon -liokernel -lsuifkernel -lbasicnodes -lsuifnodes -lsuifpasses
LIBS += -ltypebuilder -lsuifcloning
#LIBS += -lcfenodes
#LIBS += -lion -lbit_vector
#LIBS += -lutils -ltypebuilder -lsuifcloning
LIBS += -iputils -lutils

TARGET_LIB = ecrnodes

include $(NCIHOME)/Makefile.std
