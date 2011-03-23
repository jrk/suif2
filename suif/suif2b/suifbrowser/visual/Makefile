TARGET_LIB = visual
LIBS =		-lX11 -ltk8.0 -ltcl8.0 $(DYNAMIC_LINKING_LIBRARY) \
		$(EXTRA_DL_LIBS) -lsuifkernel
HEADERS =	event.h graph_layout.h  binding.h \
		vcommands.h vdefs.h vform.h vframe.h vgraph.h \
		visual.h vman.h vmenu.h vmessage.h vmisc.h vnode.h \
		vpipe.h vprop.h vtagman.h vtcl.h vtext.h vtoplevel.h \
		vwidget.h window.h vlistbox.h vmodule.h dynatype.h \
		vtty.h vbuttonbar.h tcltk_calling_convention.h
SRCS = visual.cpp vnode.cpp vprop.cpp vman.cpp vcommands.cpp vtagman.cpp \
	vtoplevel.cpp vframe.cpp vmenu.cpp vpipe.cpp vtext.cpp \
	vform.cpp vgraph.cpp binding.cpp vmessage.cpp vmisc.cpp \
	graph_layout.cpp event.cpp vlistbox.cpp window.cpp \
	vwidget.cpp vmodule.cpp dynatype.cpp vtty.cpp vbuttonbar.cpp
OBJ_FILES = visual.o vnode.o vprop.o vman.o vcommands.o vtagman.o \
	vtoplevel.o vframe.o vmenu.o vpipe.o vtext.o \
	vform.o vgraph.o binding.o vmessage.o vmisc.o \
	graph_layout.o event.o vlistbox.o window.o \
	vwidget.o vmodule.o dynatype.o vtty.o vbuttonbar.o

EXTRA_CXXFLAGS = $(TCL_INCLDIRS) -I$(X11_INCLUDE) -I.  \
	$(X11_SPECIAL_FLAGS)
LIBS += $(TCL_LIBDIRS)

include $(NCIHOME)/Makefile.std
