/* visual.h */

#ifndef VISUAL_H
#define VISUAL_H

/* functions */

void enter_visual(int *argc, char *argv[]);
void exit_visual(void);

void visual_mainloop(void);
void visual_do_one_event(void);
void visual_prompt(void);
void visual_yield(int yield_count = 20);

/* globals */

class vtoplevel;
extern vtoplevel *main_toplevel;

/* include files */

//#include "sty.h" // for boolean definition

#ifdef DEVELOPMENT
#define VISUAL_INCLFILE(F) #F
#else
#define VISUAL_INCLFILE(F) <visual/ ## F ## >
#endif

#include VISUAL_INCLFILE(vtcl.h)
#include VISUAL_INCLFILE(vnode.h)
#include VISUAL_INCLFILE(window.h)
#include VISUAL_INCLFILE(vman.h)
#include VISUAL_INCLFILE(vcommands.h)
#include VISUAL_INCLFILE(vprop.h)
#include VISUAL_INCLFILE(vtoplevel.h)
#include VISUAL_INCLFILE(vpipe.h)
#include VISUAL_INCLFILE(vtext.h)
#include VISUAL_INCLFILE(vtagman.h)
#include VISUAL_INCLFILE(vframe.h)
#include VISUAL_INCLFILE(vmenu.h)
#include VISUAL_INCLFILE(vform.h)
#include VISUAL_INCLFILE(vgraph.h)
#include VISUAL_INCLFILE(vmessage.h)
#include VISUAL_INCLFILE(vlistbox.h)
#include VISUAL_INCLFILE(vbuttonbar.h)
#include VISUAL_INCLFILE(vmisc.h)
#include VISUAL_INCLFILE(event.h)
#include VISUAL_INCLFILE(binding.h)
//#include VISUAL_INCLFILE(heap.h)
#include VISUAL_INCLFILE(vmodule.h)
#include VISUAL_INCLFILE(dynatype.h)

#endif
