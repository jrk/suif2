// This file was automatically build by
// /home/dlheine/nci_clean/nci/build_main.pl '-lcommon -lsuifkernel -liokernel -lbasicnodes -lsuifnodes -lsuifpasses -lcfenodes -lecrnodes -lion -lbit_vector -lutils -ltypebuilder -lsuifcloning -liputils'
// from the $(LIBS} variable in the Makefile
// 
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"
#include "suifpasses/suifpasses.h"


int main( int argc, char* argv[] ) {
  // initialize the environment
  SuifEnv* suif = new SuifEnv;
  suif->init();

  // import and initialize the necessary libraries
  suif->require_module("basicnodes");
  suif->require_module("suifnodes");
  suif->require_module("suifpasses");
  suif->require_module("cfenodes");
  suif->require_module("ecrnodes");
  suif->require_module("ion");
  suif->require_module("bit_vector");
  suif->require_module("utils");
  suif->require_module("typebuilder");
  suif->require_module("suifcloning");
  suif->require_module("iputils");

  // transform the input arguments into a stream of
  // input tokens
  TokenStream token_stream( argc, argv );

  // execute the Module "execute"
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  mSubSystem -> execute( "execute", &token_stream );

  delete suif;

  return 0;
}
