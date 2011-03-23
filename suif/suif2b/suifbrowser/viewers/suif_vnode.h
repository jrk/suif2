#ifndef SUIF_VNODE_H
#define SUIF_VNODE_H

//#include <zot.h>

class vnode;

void init_vnode_tags();

/*
 * Tags of suif vnodes.
 */

extern const char* tag_suif_object;
extern const char* tag_code_fragment;

/*
 * Functions to help create vnodes of suif objects.
 */

vnode* create_vnode( SuifObject *suifobj );


#endif
