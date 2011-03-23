TARGET_LIB = sbrowser
LIBS =		$(TCL_LIBDIRS) \
		-lviewers -lvisual -lsbrowser_cg -lX11 -ltk8.0 -ltcl8.0 \
		$(DYNAMIC_LINKING_LIBRARY) -liokernel -lsuifkernel \
		-lbasicnodes -lsuifnodes \
		-lsuifprinter -ltypebuilder -lcommon -lutils \
		-lcfeutils -lcfenodes \
		-lsuifcloning $(EXTRA_DL_LIBS)

HEADERS =	sbrowser.h includes.h 
SRCS =		sbrowser.cpp sbrowser_pass.cpp help.cpp
OBJ_FILES =	sbrowser_pass.o help.o
MANPAGES =	sbrowser.1

EXTRA_CXXFLAGS =   $(TCL_INCLDIRS) -I$(X11_INCLUDE) $(X11_SPECIAL_FLAGS)

# also build a program with this.
PROGRAM = sbrowser
MAIN_OBJ = sbrowser.o

include $(NCIHOME)/Makefile.std
