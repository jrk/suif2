// $Id: vtbl.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBLPASS__VTBL_H
#define VTBLPASS__VTBL_H

#include "suifkernel/suifkernel_forwarders.h"


void initialize_libraries(SuifEnv *suif_env);

extern "C" void init_vtblpass(SuifEnv* suif);


#endif /* VTBLPASS__VTBL_H */
