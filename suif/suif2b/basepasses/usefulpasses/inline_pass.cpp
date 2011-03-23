#include "common/system_specific.h"

/*  Copyright (c) 1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"
#include "suifpasses/passes.h"
#include "suifnodes/suif.h"
#include "suifkernel/utilities.h"
#include "common/suif_list.h"
#include "utils/expression_utils.h"

class InlinePass : public PipelinablePass {
public:
  InlinePass( SuifEnv* suif_env, const LString &name = "inline_pass" );
  virtual ~InlinePass();

  virtual Module* clone() const { return(Module*)this; }
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
};

InlinePass::InlinePass( SuifEnv* suif_env, const LString &name ) :
  PipelinablePass( suif_env, name ) {
}


InlinePass::~InlinePass() {
}

void InlinePass::do_procedure_definition(ProcedureDefinition *proc) {
  // create a list of the CallExpressions
#ifdef CALL_EXP
  typedef CallExpression CallType;
#else
  typedef CallStatement CallType;
#endif
  list<CallType*> *l = collect_objects<CallType>(proc);
  // Let's go nuts... Inline EVERYTHING!!!
  // It's just a test..
  for (
       list<CallType*>::iterator iter = l->begin();
       iter != l->end(); iter++) {
    CallType *the_call = *iter;
    ProcedureSymbol *proc = get_procedure_target_from_call(the_call);
    if (proc == NULL) continue;
    if (proc->get_definition() == NULL) continue;

    inline_call(the_call, proc);
  }
  delete l;
}


