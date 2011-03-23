PASS_DRIVER =	do_ecr_alias
MAIN_OBJ =	standard_main.o


BASESRC = ../../basesuif


SRCS = ecr_alias_pass.cpp ecr_alias_state.cpp \
	ecr_alias_vis.cpp \
	ecr_annotation_manager.cpp \
	ecr_computation.cpp ecr_type.cpp \
	standard_main.cpp

OBJ_FILES = ecr_alias_pass.o ecr_alias_state.o ecr_alias_vis.o \
	ecr_annotation_manager.o ecr_computation.o ecr_type.o

HEADERS = ecr_alias_forwarders.h 

LIBS += -lcommon -lsuifkernel -liokernel -lbasicnodes -lsuifnodes -lsuifpasses
LIBS += -lcfenodes -lecrnodes
LIBS += -lion -lbit_vector
LIBS += -lutils -ltypebuilder -lsuifcloning

LIBS += -liputils

TARGET_LIB = ecr_alias

PASS_DRIVER = do_ecr_alias

include $(NCIHOME)/Makefile.std

ecr_test:
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/driver -f build.suif2 > hello.txt
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ; \
		export LD_LIBRARY_PATH ; \
		$(BINDIR)/driver -f etest.suif2 > hello2.txt
	-diff hello2.txt hello.txt
	rm hello2.txt hello.txt

