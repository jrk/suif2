/* file "transforms.cc" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>

#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/group_walker.h"
#include "suifkernel/suif_object.h"
#include "cpp_transforms/cpp_symbol_walkers.h"
#include "suifkernel/real_object_factory.h"
#include "typebuilder/type_builder.h"


extern "C" void init_cpp_transforms(SuifEnv *suif_env) {
  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  init_typebuilder(suif_env);

  ms->register_module(new CombinedPassForCpp(suif_env));
}
