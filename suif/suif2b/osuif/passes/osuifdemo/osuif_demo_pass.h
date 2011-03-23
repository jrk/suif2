/* file "osuif_demo_pass.h" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


#ifndef PORKY_DEMO_H
#define PORKY_DEMO_H

#ifndef SUPPRESS_PRAGMA_INTERFACE
#pragma interface
#endif

#include <suifpasses.h>
#include <module_subsystem.h>
#include <suif_env.h>

#include <suif.h>
/*
      This is the interface to globalizing passes of the porky
      library.
*/

/*
 *  PREREQUISITES: 
 *  GENERATES: 
 */

class globalize_class_method_symbols_pass : public PipelinablePass
{
public:
  globalize_class_method_symbols_pass(SuifEnv *env, const LString &name);
  virtual ~globalize_class_method_symbols_pass(void)  { }

  // By overriding these methods, we get access to the tree
  virtual void do_file_set_block( FileSetBlock* file_set_block );
  virtual void do_file_block    ( FileBlock*    file_block );
  virtual void do_procedure_definition(ProcedureDefinition *proc_def);
  virtual void do_variable_definition(VariableDefinition *proc_def);

  Module *clone() const { return(Module*)this;};
};

#endif /* PORKY_DEMO_H */
