// $Id: instancefieldslayout.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#include "cfenodes/cfe.h"
#include "osuifnodes/osuif.h"
#include "typebuilder/type_builder.h"
#include "suifcloning/cloner.h"
#include "suifkernel/module_subsystem.h"
#include "instancefieldslayoutnodes/instancefieldslayout.h"
#include "instancefieldslayoutpass/instancefieldslayout_pass.h"
#include "instancefieldslayoutpass/offset_annote_pass.h"

#include "instancefieldslayoutpass/instancefieldslayout.h"


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
  init_instancefieldslayoutnodes(suif_env);
}

extern "C" void init_instancefieldslayoutpass( SuifEnv* suif )
{
  initialize_libraries( suif );

  ModuleSubSystem *mss = suif->get_module_subsystem();

  if ( !mss->retrieve_module("layout_single_inheritance_instance_fields") ) {
    mss->register_module(new InstanceFieldsLayoutPass( suif ) );
    mss->register_module(new InstanceFieldsLayoutOffsetAnnotePass( suif ) );
  }
}
