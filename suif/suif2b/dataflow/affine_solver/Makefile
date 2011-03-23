PASS_DRIVER =   do_affine_solver
MAIN_OBJ =      standard_main.o

TARGET_LIB =	affine_solver
LIBS =		-lsuif_cfgraph -ldflowsolver -lsgraph -lsgraph_algs \
		-lecr_alias

MANPAGES =

BASESRC = ../../basesuif
SMGN_INCLUDES = $(SMGNDIR)/searchable_list_builders.mac \
                $(SMGNDIR)/indexed_list_builders.mac \
                $(SMGNDIR)/list_builders.mac \
                $(SMGNDIR)/vector_builders.mac
HOOF = affine
HOOF_GEN = affine.cpp affine_forwarders.h affine.h affine_factory.h
HOOF_MAC = $(SMGNDIR)/suif.mac
HOOF_INCL = -I$(NCIHOME)/suif/suif2b/basesuif
HOOF_DEP = $(BASESRC)/basicnodes/basic.hoof


HEADERS =
SRCS =          \
                affine.cpp \
		affine_value.cpp \
		affine_solver.cpp \
		affine_solver_pass.cpp \
		standard_main.cpp

OBJ_FILES =     $(SRCS:.cpp=.o)

INCLDIRS += -I..
INCLDIRS += -I../../extratypes
INCLDIRS += -I../../ipanalysis
INCLDIRS += -I../..

include $(NCIHOME)/Makefile.std

localtest:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/suifdriver -f test.suif2 > hello.txt