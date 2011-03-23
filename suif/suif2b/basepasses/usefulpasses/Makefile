SUIF_INTERNAL = defined

SRCS = time_module.cpp unowned.cpp inline_pass.cpp usefulpasses.cpp \
	type_checker.cpp unused_passes.cpp \
	suif_validater.cpp validate_suif_pass.cpp \
	suif_counter.cpp count_suif_object_pass.cpp \
	suif_gc.cpp gc_symbol_table_pass.cpp strip_annotes_pass.cpp \
	exec_pass.cpp source_lang_support.cpp standard_main.cpp

OBJ_FILES = time_module.o unowned.o inline_pass.o usefulpasses.o \
	type_checker.o unused_passes.o strip_annotes_pass.o \
	suif_validater.o validate_suif_pass.o \
	suif_counter.o count_suif_object_pass.o \
	suif_gc.o gc_symbol_table_pass.o \
	exec_pass.o source_lang_support.o

HEADERS = suif_validater.h suif_gc.h

LIBS += -lsuifkernel -liokernel -lcommon -lutils
LIBS += -lcfenodes -lsuifnodes -lbasicnodes -lsuifpasses 
LIBS += -ltypebuilder

TARGET_LIB = usefulpasses

PASS_DRIVER =   do_ecr_alias
MAIN_OBJ =      standard_main.o


#EXTRA_CXXFLAGS += -I./.. 

include $(NCIHOME)/Makefile.std
