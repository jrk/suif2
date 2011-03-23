// $Id: print_pass.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

/***
#include <suif_copyright.h>
#include <cast.h>
**/
#include "suifkernel/utilities.h"
#include "common/i_integer.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifkernel/suifkernel_messages.h"
#include "osuifnodes/osuif.h"
#include "osuifprint/print_pass.h"

print_pass::print_pass(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name)
{
  printingMaps = new PrintingMaps(env, "PrettyPrinting");
  printingMaps->init_suif_object();
  osuifVisitorInfo = new OsuifVisitorInfo(printingMaps);
  osuifVisitorInfo->initOsuifObjects();
}

void print_pass::do_file_set_block(FileSetBlock* file_set_block)
{
  //apply the print pass
  printingMaps->get_process_map()->apply(file_set_block);
}

