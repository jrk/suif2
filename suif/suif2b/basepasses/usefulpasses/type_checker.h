#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

// Ideally we would be able to register modules that added rules
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class TypeCheckerPass : public Pass {
public:
  TypeCheckerPass(SuifEnv *pEnv, const LString &name =
		  "check_types");
  Module* clone() const { return (Module*)this; }
  void do_file_set_block(FileSetBlock *fsb);
};
#endif
