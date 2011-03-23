PASS_DRIVER =   do_copyprop
MAIN_OBJ =      standard_main.o

TARGET_LIB =	copyprop
LIBS =		-lcommon -liokernel -lsuifkernel -lbasicnodes -lsuifnodes \
		-lbit_vector -lion\
		-lsgraph -lsgraph_algs -lsuper_graph -ldflowsolver \
		-lsuif_cfgraph -lutils -ltypebuilder\
		-lsuifprinter -lcfenodes \
		-lsuifpasses -lsuifcloning
MANPAGES =
HEADERS =	var_prop.h
SRCS =		var_prop.cpp copyprop_pass.cpp standard_main.cpp

OBJ_FILES =     $(SRCS:.cpp=.o)

#INCLDIRS += -I..
#INCLDIRS += -I../../extratypes
#INCLDIRS += -I../../ipanalysis

include $(NCIHOME)/Makefile.std

localtest:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/suifdriver -f test.suif2 > hello.txt