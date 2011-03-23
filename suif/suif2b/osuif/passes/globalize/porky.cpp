/* file "porky.cc" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


/*
      This is the main implementation file of porky, a library of core
      passes for converting between different dialects of SUIF.
*/


#define _MODULE_ "libporky"

#pragma implementation "porky.h"


/*
#include <suif.h>
#include <suifpasses.h>
*/

#include "porky.h"
#include "build_single_vtable.h"
#include "globalize_class_methods.h"
#include "globalize_class_variables.h"
/*
#include "make_empty_file_set_symbol_table.h"
*/

extern "C" void init_porky(SuifEnv *suif_env) {

  ModuleSubSystem *ms = suif_env->get_module_subsystem();

  ms->register_module(new build_single_vtable_pass(suif_env, 
						      "build_single_vtable"));
  ms->register_module(new globalize_class_method_symbols_pass(suif_env, 
						      "globalize_class_methods"));
  ms->register_module(new globalize_class_variable_symbols_pass(suif_env, 
						      "globalize_class_variables"));
}

  //extern "C" void exit_porky(void)
  //  {
    /* empty */
  //  }
