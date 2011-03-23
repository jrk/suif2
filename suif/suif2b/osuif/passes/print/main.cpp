// $Id: main.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "suifkernel/forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"
#include "suifpasses/suifpasses.h"
#include "osuifprint/print.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "osuifnodes/osuif.h"

int main(int argc, char *argv[]){
  //initialize the environment
  SuifEnv *suif = new SuifEnv;
  suif->init();

  //import and initialize the necessary libraries
  init_basicnodes(suif);
  init_suifnodes(suif);
  init_osuifnodes(suif);

  init_suifpasses(suif);
  init_osuifprint(suif);

  //transform the input into a stream of tokens
  TokenStream token_stream(argc, argv);

  //execute the module driver, it is registered with a module name of execute
  ModuleSubSystem *mSubSystem = suif->get_module_subsystem();
  mSubSystem->execute("execute", &token_stream);

  delete suif;
}

