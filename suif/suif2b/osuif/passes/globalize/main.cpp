#include "forwarders.h"
#include "suif_env.h"
#include "module_subsystem.h"
#include "token_stream.h"
#include "suifpasses.h"
#include "porky.h"

//#define MEMORY_STATISTICS
#ifdef MEMORY_STATISTICS
#include "mem_check.cpp"
#endif

int main( int argc, char* argv[] ) {
  // initialize the environment
  SuifEnv* suif = new SuifEnv;
  suif->init();

  // import and initialize the necessary libraries
  init_suifpasses( suif );
  init_porky( suif );

  // transform the input arguments into a stream of
  // input tokens
  TokenStream token_stream( argc, argv );

  // execute the Module "execute"
  ModuleSubSystem* mSubSystem = suif->get_module_subsystem();
  mSubSystem -> execute( "execute", &token_stream );

  delete suif;

#ifdef MEMORY_STATISTICS
  print_memory_statistics();
#endif
  return 0;
}


  
