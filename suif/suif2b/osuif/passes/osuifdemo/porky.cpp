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
#include "osuif_demo_pass.h"
/*
#include "make_empty_file_set_symbol_table.h"
*/

extern "C" void init_porky(SuifEnv *suif_env) {
  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  /*
    static boolean init_done = FALSE;

    if (init_done)
    return;
    init_done = TRUE;
    enter_suif(argc, argv);
    enter_suifpasses(argc, argv);
  */

  /*
  ms->register_module(new make_empty_file_set_symbol_table_pass());
  */
  ms->register_module(new globalize_class_method_symbols_pass(suif_env, 
						      "osuif_demo_pass"));
  /*
  ms->register_module(new dismantle_while_statements_pass());
  ms->register_module(new dismantle_do_while_statements_pass());
  ms->register_module(new dismantle_for_statements_pass());
  ms->register_module(new dismantle_scope_statements_pass());
  ms->register_module(new dismantle_multi_way_branch_statements_pass());
  ms->register_module(new dismantle_select_instructions_pass());
  ms->register_module(new dismantle_array_reference_instructions_pass());
  ms->register_module(new dismantle_field_access_instructions_pass());
  ms->register_module(new dismantle_extract_fields_instructions_pass());
  ms->register_module(new dismantle_set_fields_instructions_pass());
  ms->register_module(new dismantle_extract_elements_instructions_pass());
  ms->register_module(new dismantle_set_elements_instructions_pass());
  ms->register_module(new dismantle_bit_size_of_instructions_pass());
  ms->register_module(new dismantle_byte_size_of_instructions_pass());
  ms->register_module(new dismantle_bit_alignment_of_instructions_pass());
  ms->register_module(new dismantle_byte_alignment_of_instructions_pass());
  ms->register_module(new dismantle_bit_offset_of_instructions_pass());
  ms->register_module(new dismantle_byte_offset_of_instructions_pass());
  ms->register_module(new dismantle_sc_and_instructions_pass());
  ms->register_module(new dismantle_sc_or_instructions_pass());
  ms->register_module(new dismantle_sc_select_instructions_pass());
  ms->register_module(new dismantle_load_value_block_instructions_pass());
  ms->register_module(new dismantle_multi_way_branch_instructions_pass());
  */
}

  //extern "C" void exit_porky(void)
  //  {
    /* empty */
  //  }
