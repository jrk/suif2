#ifndef _MARK_LIB_PASS_H_
#define _MARK_LIB_PASS_H_

/*
 * This pass adds information about the known but undefined procedures.
 * These include system functions, runtime library functions, and
 *  intrisic functions.
 * The information is added in the form of annotation on ProcedureSymbol.
 *
 * Currently, the following annotations will be installed:
 *
 *  Name              Annote type        Desc
 * -------            ------------       ------
 * mark_lib.is_known  GeneralAnnote      The presence of this annotation
 *                                       means the proc is a known function.
 */


#include "utils/simple_module.h"
#include "suifkernel/module_subsystem.h"
#include "proc_table.h"


class MarkLibPass : public SimpleModule {
 private:
  StaticProcTable _proc_table;

  void set_known_proc(ProcedureSymbol*);

 public:
  MarkLibPass(SuifEnv* senv) :
    SimpleModule(senv, "mark_lib"),
    _proc_table() {
  };

  virtual void execute(suif_vector<LString>*);
  virtual String get_description(void) const;

  bool is_known_proc(ProcedureSymbol*) const;

};



#endif // _MARK_LIB_PASS_H_
