#ifndef __USEFULPASSES__STRIP_ANNOTES_H__
#define __USEFULPASSES__STRIP_ANNOTES_H__ 1

#include "suifpasses/suifpasses.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/command_line_parsing.h"
#include "common/suif_vector.h"

class StripAnnotesPass : public Pass {
public:
  StripAnnotesPass(SuifEnv *suif_env, const LString &name = "strip_annotes");
  Module * clone() const { return (Module *)this; }
  void do_file_set_block(FileSetBlock *fsb);
  virtual void initialize();

private:
  suif_vector< String > annote_name_args;
};
  

#endif
