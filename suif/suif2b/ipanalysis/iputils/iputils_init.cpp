#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"

extern "C" void init_iputilsnodes(SuifEnv *s);
extern "C" void init_iputils(SuifEnv *s)
{
  s->require_module("utils");
  init_iputilsnodes(s);
}
