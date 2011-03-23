// $Id: init_utils.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "typebuilder/type_builder.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"
#include "suifcloning/cloner.h"
#include "suifpasses/suifpasses.h"
#include "suifprinter/suifprinter.h"
#include "osuiftypebuilder/type_builder.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"

#include "osuifutilities/init_utils.h"


void initialize_suif_libraries( SuifEnv *suif_env ) {
  init_typebuilder(suif_env);
  init_basicnodes(suif_env);
  init_suifnodes(suif_env);
  init_cfenodes(suif_env);
  init_suifcloning(suif_env);
  init_suifpasses(suif_env);
  init_suifprinter(suif_env);
}


void initialize_osuif_libraries( SuifEnv *suif_env ) {
  initialize_suif_libraries(suif_env);

  init_osuiftypebuilder(suif_env);
  init_osuifnodes(suif_env);
  init_osuifextensionnodes(suif_env);
}
