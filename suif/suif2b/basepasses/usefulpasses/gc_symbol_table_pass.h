#ifndef _GCSYMBOLTABLEPASS_H_
#define _GCSYMBOLTABLEPASS_H_

/**
  * @file 
  * This pass removes unreferenced types & symbols from external symbol table.
  *
  */

#include "suifkernel/module.h"

class GCSymbolTablePass : public Module {
public:
  GCSymbolTablePass( SuifEnv* suif_env );

  virtual void initialize();

  virtual Module* clone() const;

  virtual void execute();

  virtual bool delete_me() const;

  static const LString get_class_name();

};

// @class GCSymbolTablePass gc_symbol_table_pass.h usefulpasses/gc_symbol_table_pass.h

#endif /* _GCSYMBOLTBLEPASS_H_ */

