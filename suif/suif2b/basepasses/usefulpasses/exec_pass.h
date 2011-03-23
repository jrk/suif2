#ifndef _EXEC_PASS_H_
#define _EXEC_PASS_H_

/** 
  * Defines ExecPass, a pass that executes a shell command.
  *
  * Usage:
  *   exec <command> <arg1> <arg2> ...
  *
  * If a system error occurrs, an error message will be printed to cerr
  * and an exception will be thrown.
  */

#include "utils/simple_module.h"

class ExecPass : public SimpleModule {

 public:
  ExecPass(SuifEnv* senv);
  virtual void execute(suif_vector<LString>* args);
  virtual String get_help_string(void) const;
};


#endif // _EXEC_PASS_H_
