// $Id: print.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

//#include <common/suif_copyright.h>

#include "osuifprint/print.h"
#include "osuifprint/print_pass.h"

extern "C" void init_osuifprint(SuifEnv *suif_env)
{
  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  ms->register_module(new print_pass(suif_env, "osuifprint"));
}
