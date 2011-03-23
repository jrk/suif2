PASS_DRIVER =	do_dflowsolver
MAIN_OBJ =	standard_main.o

TARGET_LIB = 	dflowsolver

#HEADERS =	$(TARGET).h

HEADERS =	region_value.h region_solver.h \
		flat_value.h flat_solver.h dflowsolver_forwarders.h

SRCS =          \
                region_value.cpp region_solver.cpp \
		flat_value.cpp flat_solver.cpp \
		standard_main.cpp

OBJ_FILES =     $(SRCS:.cpp=.o)

LIBS += -lcommon -liokernel -lsuifkernel -lion \
	-lbit_vector -lsgraph -lsuper_graph -lsgraph_algs

#EXTRA_CXXFLAGS += -I../../extratypes
#EXTRA_CXXFLAGS += -I..
#EXTRA_CXXFLAGS += -Weffc++
EXTRA_CXXFLAGS += -DINLINE_ALL_TEMPLATES

include $(NCIHOME)/Makefile.std

dflowtest:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/driver -f etest.suif2 > hello2.txt
#        -diff hello2.txt hello.txt
#        rm hello2.txt hello.txt
