/*  Top-level Call Graph Include File */

/*  Copyright (c) 1995, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef CG_H
#define CG_H



/*
 *  Use a macro to include files so that they can be treated differently
 *  when compiling the library than when compiling an application.
 */

#ifdef CGLIB
#define CGINCLFILE(F) #F
#else
#define CGINCLFILE(F) <sbrowser_cg/ ## F ## >
#endif

#include CGINCLFILE(cgraph.h)
#include CGINCLFILE(cnode.h)

#endif /* CG_H */
