// $Id: lowering.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#include "cfenodes/cfe.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"
#include "typebuilder/type_builder.h"
#include "suifcloning/cloner.h"
#include "suifkernel/module_subsystem.h"

#include "osuiflowering/lowering_pass.h"
#include "osuiflowering/static_method_call_pass.h"
#include "osuiflowering/definition_block_pass.h"
#include "osuiflowering/lowering.h"


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
}

extern "C" void init_osuiflowering( SuifEnv* suif )
{
  initialize_libraries( suif );

  ModuleSubSystem *mss = suif->get_module_subsystem();

  mss->register_module(new LoweringPass( suif ) );
  mss->register_module(new StaticMethodCallStatementLoweringPass( suif ) );
  mss->register_module(new StaticMethodCallExpressionLoweringPass( suif ) );
  mss->register_module(new DefinitionBlockPass( suif ) );
}
