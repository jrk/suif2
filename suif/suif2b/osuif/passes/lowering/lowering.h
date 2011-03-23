// $Id: lowering.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef OSUIF_LOWERING__LOWERING_H
#define OSUIF_LOWERING__LOWERING_H

#include "suifkernel/suifkernel_forwarders.h"


void initialize_libraries(SuifEnv *suif_env);

extern "C" void init_osuiflowering(SuifEnv* suif);


#endif /* OSUIF_LOWERING__LOWERING_H */
