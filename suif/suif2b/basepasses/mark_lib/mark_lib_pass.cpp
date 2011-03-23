/*
 * file : mark_lib_pass.h
 */

#include "mark_lib_pass.h"
#include "iokernel/object_factory.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/field_description.h"
#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"
#include "suifprinter/suifprinter.h"
#include "utils/print_utils.h"
#include "suifkernel/utilities.h"
#include "basicnodes/basic_factory.h"


#include "iostream.h"


extern "C" void EXPORT init_mark_lib(SuifEnv* suif_env) {
  ModuleSubSystem* msub = suif_env->get_module_subsystem();
  msub->test_and_register_module(new MarkLibPass(suif_env), true);
}

  



static const String k_known_proc_annote("mark_lib.is_known_proc");


void MarkLibPass::set_known_proc(ProcedureSymbol* psym)
{
  SuifEnv* senv = psym->get_suif_env();
  suif_assert_message(psym->peek_annote(k_known_proc_annote) == 0,
		      ("already ran mark_lib."));
  psym->append_annote(create_general_annote(senv, k_known_proc_annote));
}


String MarkLibPass::get_description(void) const
{
  return String("Usage:\n  mark_lib\n");
}


void MarkLibPass::execute(suif_vector<LString>* args)
{
  check_arg_count(args, 0);
  check_file_set_block();
  for (Iter<ProcedureSymbol> iter = 
	 object_iterator<ProcedureSymbol>(get_file_set_block());
       iter.is_valid();
       iter.next()) {
    ProcedureSymbol* psym = &iter.current();
    if (_proc_table.is_in_table(psym))
      set_known_proc(psym);
  }
}


bool MarkLibPass::is_known_proc(ProcedureSymbol* psym) const
{
  return (psym->peek_annote(k_known_proc_annote) != 0);
}
