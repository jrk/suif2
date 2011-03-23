/*----------------------------------------------------------------------
 * suif_types.cc
 *
 */



#include "suif_types.h"
//#include <sty.h>
#include "suifkernel/suifkernel_forwarders.h"
#include <stdlib.h>
#include <stdio.h>


char *type_BRICK_NULL = (char*) LString("BRICK_NULL").c_str(), 
     *type_BRICK_OWNED_ZOT = (char*) LString("BRICK_OWNDED_ZOT").c_str(), 
     *type_BRICK_ZOT_REF = (char*) LString("BRICK_ZOT_REF").c_str(), 
     *type_BRICK_INTEGER = (char*) LString("BRICK_INTEGER").c_str(), 
     *type_BRICK_STRING = (char*) LString("BRICK_STRING").c_str(),
     *type_BRICK_BIT_BLOCK = (char*) LString("BRICK_BIT_BLOCK").c_str();



