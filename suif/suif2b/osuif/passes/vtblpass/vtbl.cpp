// $Id: vtbl.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "cfenodes/cfe.h"
#include "osuifnodes/osuif.h"
#include "typebuilder/type_builder.h"
#include "suifcloning/cloner.h"
#include "suifkernel/module_subsystem.h"
#include "vtblnodes/vtbl.h"
#include "vtblpass/vtbl_pass.h"
#include "vtblpass/vtbl_annote_pass.h"
#include "vtblpass/slot_annote_pass.h"
#include "vtblpass/slot_count_annote_pass.h"
#include "vtblpass/instance_method_call_pass.h"

#include "vtblpass/vtbl.h"


// Initialize necessary libraries
void initialize_libraries(SuifEnv *suif_env) {
  init_typebuilder(suif_env);
  init_basicnodes(suif_env);
  init_suifnodes(suif_env);
  init_cfenodes(suif_env);
  init_suifcloning(suif_env);
  init_suifpasses(suif_env);
  init_osuifnodes(suif_env);
  init_osuifextensionnodes(suif_env);
  init_vtblnodes(suif_env);
}

extern "C" void init_vtblpass( SuifEnv* suif )
{
  initialize_libraries( suif );

  ModuleSubSystem *mss = suif->get_module_subsystem();

  if ( !mss->retrieve_module("build_single_inheritance_vtbl") ) {
    mss->register_module(new VtblPass( suif ) );
    mss->register_module(new VtblAnnotePass( suif ) );
    mss->register_module(new VtblSlotAnnotePass( suif ) );
    mss->register_module(new VtblSlotCountAnnotePass( suif ) );
    mss->register_module(new InstanceMethodCallStatementLoweringPass( suif ) );
    mss->register_module(new InstanceMethodCallExpressionLoweringPass( suif ) );
  }
}
