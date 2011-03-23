/* file "globalize_class_variables.h" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


#ifndef PORKY_GLOB_VAR_H
#define PORKY_GLOB_VAR_H

#ifndef SUPPRESS_PRAGMA_INTERFACE
#pragma interface
#endif

#include <suifpasses.h>
#include <module_subsystem.h>
#include <suif_env.h>

#include <osuif.h>
/*
      This is the interface to globalizing passes of the porky
      library.
*/

/*
 *  PREREQUISITES:
 *
 *  GENERATES: "Class variables in global symbol table"
 */

class globalize_class_variable_symbols_pass : public PipelinablePass
{
public:
  globalize_class_variable_symbols_pass(SuifEnv *env, const LString &name);
  virtual ~globalize_class_variable_symbols_pass(void)  { }
  Module *clone() const { return(Module*)this;};

  virtual void do_file_block( FileBlock* file_block );
  virtual void do_class_type(FileSetBlock* f, ClassType *the_class);

  virtual void move_variable_symbols(SuifEnv *s, const LString &mname, BasicSymbolTable *the_table, BasicSymbolTable *st);
  // Name mangler
  virtual LString mangle(BasicSymbolTable* st, const char* classname, const char* membername);
};

#endif /* PORKY_GLOB_VAR_H */
