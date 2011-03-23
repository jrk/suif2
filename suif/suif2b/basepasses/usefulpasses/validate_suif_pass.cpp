
#include "validate_suif_pass.h"

#include "suifkernel/command_line_parsing.h"
#include "suifkernel/suif_env.h"

#include <iostream.h>

#include "suif_validater.h"


const LString ValidateSuifPass::get_class_name()
{
  static LString name( "validate_suif" );
  return name;
}


ValidateSuifPass::ValidateSuifPass( SuifEnv* suif_env ) :
  Module( suif_env )
{
  // override an inherited instance variable
  _module_name = get_class_name();
}


ValidateSuifPass::~ValidateSuifPass(void)
{
}

void ValidateSuifPass::initialize(void)
{
  Module::initialize();
  _command_line -> set_description("validates the FileSetBlock");
}


Module* ValidateSuifPass::clone() const
{
  return (Module*)this;
}

bool ValidateSuifPass::delete_me() const
{
  return false;
}

void ValidateSuifPass::execute(void)
{
  FileSetBlock *master = _suif_env->get_file_set_block();
  SuifValidater val;
  if (!val.is_valid(master))
    cout << val.get_error() << endl;
}
