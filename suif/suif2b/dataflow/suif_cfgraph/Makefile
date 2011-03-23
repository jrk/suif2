PASS_DRIVER =	do_build_cfgraph
MAIN_OBJ =	standard_main.o

TARGET_LIB = 	suif_cfgraph

# The primary interfaces here are:

HEADERS =	cfgraph_state.h suif_cfgraph.h cfgraph_vis.h \
		suif_cfgraph_constants.h \
		suif_cfgraph_pass.h suif_cfgraph_query.h \
		suif_cfgraph_forwarders.h cfgraph_module.h \
		cfgraph_forwarders.h


HOOF_MAC = 	$(SMGNDIR)/suif.mac
SMGN_INCLUDES =	$(SMGNDIR)/searchable_list_builders.mac \
		$(SMGNDIR)/indexed_list_builders.mac \
		$(SMGNDIR)/list_builders.mac \
		$(SMGNDIR)/vector_builders.mac
HOOF =		suif_cfgraph
HOOF_GEN = 	suif_cfgraph.cpp suif_cfgraph_forwarders.h suif_cfgraph.h \
		suif_cfgraph_factory.h

#EXPORTS =	$(TARGET).h

SRCS =          \
                cfgraph_state.cpp suif_cfgraph.cpp cfgraph_vis.cpp \
		suif_cfgraph_constants.cpp \
		suif_cfgraph_pass.cpp suif_cfgraph_query.cpp \
		cfgraph_module.cpp \
		standard_main.cpp

OBJ_FILES =     $(SRCS:.cpp=.o)

LIBS += -lsuifkernel -liokernel -lcommon -lsgraph
LIBS += -lion -lbasicnodes -lsuifnodes -lsuifpasses
LIBS += -lutils -lsuper_graph -lbit_vector -lsgraph_algs
LIBS += -ltypebuilder -lcfenodes -lsuifcloning
# -ldflowtypes -lrinds -ltos

EXTRA_CXXFLAGS += -I../../extratypes
EXTRA_CXXFLAGS += -I..
EXTRA_CXXFLAGS += -I../..
#EXTRA_CXXFLAGS += -Weffc++
EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES
EXTRA_CXXFLAGS += -DBUILDING_LIBRARY

#EXTRA_CXXFLAGS += -Weffc++

include $(NCIHOME)/Makefile.std
