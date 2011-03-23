// $Id: print_pass.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFPRINT__PRINT_PASS_H
#define OSUIFPRINT__PRINT_PASS_H

#include "suifpasses/suifpasses.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifnodes/suif.h"

#include "osuifprint/printing_maps.h"
#include "osuifprint/osuif_visitor_info.h"

class print_pass : public PipelinablePass
{
public:
  print_pass(SuifEnv *env, const LString &name);
  virtual ~print_pass(void){}

  virtual void do_file_set_block(FileSetBlock* file_set_block);

  Module *clone() const{ return (Module*) this;}

private:
  PrintingMaps *printingMaps;
  OsuifVisitorInfo *osuifVisitorInfo;
};

#endif /* OSUIFPRINT__PRINT_PASS_H */
