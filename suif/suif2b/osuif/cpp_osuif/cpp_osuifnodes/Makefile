# Makefile
HEADERS = cpp_osuif_forwarders.h cpp_osuif.h cpp_osuif_factory.h
SRCS = cpp_osuif.cpp cpp_osuif_body.cpp
OBJ_FILES = cpp_osuif.o cpp_osuif_body.o
HOOF_GEN = cpp_osuif.cpp $(HEADERS)
HOOF_MAC = $(SMGNDIR)/suif.mac
BASESUIF = $(NCIHOME)/suif/suif2b/basesuif
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif -I$(NCIHOME)/suif/suif2b/osuif
HOOF_DEP = $(BASESUIF)/basicnodes/basic.hoof $(BASESUIF)/suifnodes/suif.hoof \
           $(BASESUIF)/cfenodes/cfe.hoof $(OSUIFHOME)/osuifnodes/osuif.hoof 
TARGET_DOC = cpp_osuif


SMEG_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac
HOOF = cpp_osuif
TARGET_LIB = cpp_osuifnodes

INCLDIRS += -I.. -I$(INCDIR)

LIBS += $(DYNAMIC_LINKING_LIBRARY) -losuifextensionnodes -losuifnodes -lcfenodes -lsuifnodes \
        -lbasicnodes -lsuifkernel -liokernel -lcommon -losuifnodes

include $(NCIHOME)/Makefile.std
