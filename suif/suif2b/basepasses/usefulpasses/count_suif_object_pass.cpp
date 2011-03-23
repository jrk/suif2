
#include "count_suif_object_pass.h"

#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/command_line_parsing.h"
#include "basicnodes/basic.h"
#include <iostream.h>

#include "suif_counter.h"



const LString CountSuifObjectPass::get_class_name()
{
  static LString name( "count_suif_object" );
  return name;
}


CountSuifObjectPass::CountSuifObjectPass( SuifEnv* suif_env ) :
  Module( suif_env )
{
  // override an inherited instance variable
  _module_name = get_class_name();
}


void CountSuifObjectPass::initialize(void)
{
  Module::initialize();
  _command_line -> set_description("Count number of suif IR objects");
}


Module* CountSuifObjectPass::clone() const
{
  return (Module*)this;
}

bool CountSuifObjectPass::delete_me() const
{
  return false;
}


void CountSuifObjectPass::execute(void)
{
  SuifCounter cnt(_suif_env);
  cnt.count(_suif_env->get_file_set_block());
  cnt.print_result(cout);
}
