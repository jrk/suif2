#include "common/system_specific.h"
/* file "cpp_transforms.cpp" */


/*
       Copyright (c) 1999 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <common/suif_copyright.h>


/*
      This is the main implementation file of transforms, a library of core
      passes for converting between different dialects of SUIF.
*/


#include "typebuilder/type_builder.h"
#include "suifkernel/command_line_parsing.h"
#include "dismantle_cpp_vtables.h"

extern "C" void init_cpplowering(SuifEnv *suif_env) {
    ModuleSubSystem *ms = suif_env->get_module_subsystem();
    suif_env->require_DLL("cpp_osuifnodes");
    suif_env->require_DLL("typebuilder");
    suif_env->require_DLL("suifnodes");
    suif_env->require_DLL("cfenodes");
    suif_env->require_DLL("suifcloning");
    suif_env->require_DLL("cfeutils");
    suif_env->require_DLL("suifpasses");
    suif_env->require_DLL("suiflocation");

    ms->register_module(new DismantleCppVTablesPass(suif_env));
    ms->register_module(new BuildCppVTablesPass(suif_env));
    }

