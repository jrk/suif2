PASS_DRIVER =   do_deadcode
MAIN_OBJ =      standard_main.o

TARGET_LIB =	deadcode
LIBS =		-lcommon -liokernel -lsuifkernel -lbasicnodes -lsuifnodes \
		-lbit_vector -lion\
		-lsgraph -lsgraph_algs -lsuper_graph -ldflowsolver \
		-lsuif_cfgraph -lutils -ltypebuilder\
		-lcfenodes -lsuifprinter \
		-lsuifpasses -lsuifcloning

#		-lecr_alias -lecrnodes \

MANPAGES =
HEADERS =	reaching_defs.h liveness.h
SRCS =          \
                reaching_defs.cpp deadcode_pass.cpp \
		liveness.cpp \
		var_defs_map.cpp standard_main.cpp

OBJ_FILES =     $(SRCS:.cpp=.o)

#INCLDIRS += -I..
#INCLDIRS += -I../../extratypes
#INCLDIRS += -I../../ipanalysis

include $(NCIHOME)/Makefile.std

localtest:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/suifdriver -f test.suif2 > hello.txt