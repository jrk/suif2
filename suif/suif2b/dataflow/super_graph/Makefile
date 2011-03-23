PASS_DRIVER =	do_super_graph
MAIN_OBJ =	standard_main.o

TARGET_LIB = 	super_graph

# The primary interfaces here are:
# sgraph and ngraph

HEADERS =	super_graph.h super_graph_forwarders.h

SRCS =		super_graph.cpp standard_main.cpp
OBJ_FILES =	$(SRCS:.cpp=.o)

LIBS += -lsuifkernel -liokernel -lcommon
LIBS += -lion -lsgraph -lsgraph_algs -lbit_vector

EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES

include $(NCIHOME)/Makefile.std

