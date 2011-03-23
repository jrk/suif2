#ifndef CONSTANT_FOLD_H
#define CONSTANT_FOLD_H

#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class ConstantFolderPass : public PipelinablePass {
public:
  ConstantFolderPass(SuifEnv *pEnv);
	Module* clone() const { return (Module*)this; }
  void do_procedure_definition(ProcedureDefinition *proc_def);
};
#endif