/*
 * file : lib_marker.cpp
 */

#include "mark_lib_query.h"

#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "utils/dcast.h"
#include "mark_lib_pass.h"



bool MarkLibQuery::is_known_proc(ProcedureSymbol* psym)
{
  ModuleSubSystem* msub = psym->get_suif_env()->get_module_subsystem();
  Module* mlpass = msub->retrieve_module("mark_lib");
  suif_assert_message(mlpass != 0, ("Need to run \"mark_lib\" pass."));
  return (DCAST(MarkLibPass*, mlpass)->is_known_proc(psym));
}
