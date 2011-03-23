TARGET_LIB = viewers
LIBS =		-lsbrowser_cg -lvisual
HEADERS =	cg_viewer.h info_viewer.h output_viewer.h \
                suif_viewer.h prof_viewer.h code_tree.h src_viewer.h \
		suif_print.h suif_utils.h suif_vnode.h text_viewer.h \
		viewers.h list_viewer.h main_window.h profile.h \
		suif_event.h base_viewer.h ann_form.h suif_menu.h \
		suif_types.h

EXTRA_CXXFLAGS =  -DVIEWERSLIB $(TCL_INCLDIRS) -I$(X11_INCLUDE) \
	$(X11_SPECIAL_FLAGS)
LIBS += $(TCL_LIBDIRS)

SRCS = viewers.cpp main_window.cpp suif_vnode.cpp base_viewer.cpp \
text_viewer.cpp suif_types.cpp suif_utils.cpp prof_viewer.cpp \
profile.cpp list_viewer.cpp  info_viewer.cpp suif_menu.cpp suif_print.cpp \
ann_form.cpp output_viewer.cpp code_tree.cpp  suif_viewer.cpp \
src_viewer.cpp cg_viewer.cpp

OBJ_FILES = viewers.o main_window.o suif_vnode.o base_viewer.o text_viewer.o \
suif_types.o suif_utils.o prof_viewer.o profile.o \
list_viewer.o suif_menu.o ann_form.o suif_print.o  suif_viewer.o \
output_viewer.o code_tree.o src_viewer.o cg_viewer.o info_viewer.o

include $(NCIHOME)/Makefile.std
