// $Id: main.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "suifkernel/forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module.h"
#include "suifpasses/passes.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"

#include "vtblpass/vtbl.h"
#include "vtblpass/vtbl_pass.h"


static const char* _program_name= "vtbl-prg";


/**
 * Driver module for vtbl pass.
 */
class Vtbl : public Module {
protected:
  VtblPass* _pass;

public:
  String _input_file;
  String _output_file;
  
  Vtbl (SuifEnv* suif_env, const LString& moduleName) :
    Module(suif_env, moduleName)
    {
      _pass = new VtblPass( suif_env );
    };
 
  Module* clone() const { return (Module *) this; }
  
  virtual void initialize() {
    _pass->initialize();

    OptionString* input_file_name =
      new OptionString("input-file", &_input_file);
    OptionString* output_file_name =
      new OptionString("output-file", &_output_file);

    // add additional options to OSUIF lowering pass
    _pass->get_command_line()->add(input_file_name);
    _pass->get_command_line()->add(output_file_name);
  }

  bool parse_command_line(TokenStream* stream ) {
    return _pass->parse_command_line( stream );
  }
  
  virtual void execute() {
    get_suif_env()->read( _input_file );
    _pass->execute();
    get_suif_env()->write( _output_file );
  }
};


int main(int argc, char* argv[])
{
  //initialize the environment
  SuifEnv *suif = new SuifEnv();
  suif->init();

  //initialize the necessary libraries
  initialize_libraries( suif );
  init_vtblpass( suif );

  Vtbl* vtbl_module = new Vtbl( suif, _program_name );

  ModuleSubSystem *mss = suif->get_module_subsystem();
  mss->register_module( vtbl_module );

  TokenStream token_stream(argc, argv);
  return mss->execute( _program_name, &token_stream );
}

