SUIF_INTERNAL = defined
OBJ_FILES =  cpp_osuif_type_builder.o
SRCS = cpp_osuif_type_builder.cpp
HEADERS = cpp_osuif_type_builder.h

TARGET_LIB = cpp_osuif_typebuilder

INCLDIRS += -I$(INCDIR)
LIBS += -lcpp_osuifnodes -lcfenodes -losuifnodes -lsuifkernel -liokernel -lbasicnodes \
        -lsuifnodes -lcommon -losuiftypebuilder -ltypebuilder 

include $(NCIHOME)/Makefile.std
