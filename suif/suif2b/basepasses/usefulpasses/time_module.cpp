#include "common/system_specific.h"
#include "time_module.h"

#include <stdio.h>
#include <time.h>

TimeModule::TimeModule( SuifEnv* suif_env ) :
  Module( suif_env ) {
  _module_name = "time";
}


TimeModule::~TimeModule() {
}


Module* TimeModule::clone() const {
  return (Module*)this;
}


bool TimeModule::delete_me() const {
  return false;
}


void TimeModule::execute() {
  fprintf( stderr, "TIME %f\n", ((float)clock())/CLOCKS_PER_SEC );
}


