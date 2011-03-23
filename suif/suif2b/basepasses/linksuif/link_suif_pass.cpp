
#include "link_suif_pass.h"

#include "suifkernel/command_line_parsing.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/error_subsystem.h"
#include "suifkernel/module_subsystem.h"
#include "utils/trash_utils.h"

#include "common/suif_vector.h"
#include <iostream.h>

#include "suif_linker.h"

extern "C" void EXPORT init_linksuif( SuifEnv* suif_env ) {
  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("typebuilder");
  suif_env->require_module("suifprinter");
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();
  module_subsystem->test_and_register_module(new LinkSuifPass(suif_env), true);
}




const LString LinkSuifPass::get_command_name()
{
  static LString name( "link_suif" );
  return name;
}


LinkSuifPass::LinkSuifPass( SuifEnv* suif_env ) :
  SimpleModule( suif_env, get_command_name() )
{}


LinkSuifPass::~LinkSuifPass(void)
{
}


void LinkSuifPass::execute(suif_vector<LString>* args)
{
  int fcnt = args->size();
  int i=0;

  // this should really be replaced with the command-line parsing stuff.
  if (args->at(0) == LString("-v")) {
    _link_verbose = true;
    i++;
  }
  else {
    _link_verbose = false;
  }

  LString filename("existing file set block");

  if (fcnt == 0) return;
  FileSetBlock *master = _suif_env->get_file_set_block();
  if (master == NULL) {
    filename = args->at(i);

    if (_link_verbose) {
      cerr << "linksuif: Reading file " << filename.c_str() << " as master file set block.\n";
    }

    _suif_env->read(filename);    
    i++;
  };


  master = _suif_env->get_file_set_block();
  SuifLinker slinker(_suif_env, master, filename);
  slinker.set_verbose(_link_verbose);
  for (; i<fcnt; i++) {
    filename = args->at(i);

    if (_link_verbose) {
      cerr << "linksuif: Reading file " << filename.c_str() << " as slave file set block.\n";
    }

    FileSetBlock *slave = _suif_env->read_more(filename);
    /*
    if (validate_file_set_block_ownership(slave)!= 0) {
      suif_warning("invalid suif slave ownership");
    }
    if (validate_file_set_block_ownership(_suif_env->get_file_set_block())) {
      suif_warning("invalid suif master ownership");
    }
    */
    slinker.add(slave, filename);
    delete slave;
  }
  slinker.link();
  if (slinker.get_error_count() > 0)
    SUIF_THROW(SuifException(String("Link command failed with ") +
			     String(slinker.get_error_count()) + " errors\n" +
			     slinker.get_error_message()));
}



String LinkSuifPass::get_help_string(void) const
{
  return String("Usage:\n  link_suif [-v] <suif_file_1> <suif_file_2> ...\n"
                "  -v: be verbose\n");
}
