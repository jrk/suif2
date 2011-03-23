/* file "build_single_vtable.h" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


#ifndef PORKY_SINGLE_VTABLE_H
#define PORKY_SINGLE_VTABLE_H

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
 *  PREREQUISITES: "Single inheritance model"
 *                 "Dynamic methods are marked as such"
 *                 "Methods in ClassType symbol tables"
 *                 "Classes inherit from a global class"
 *
 *  GENERATES: "ClassTypes annoted with vtable"
 */

class build_single_vtable_pass : public PipelinablePass
{
public:
  build_single_vtable_pass(SuifEnv *env, const LString &name);
  virtual ~build_single_vtable_pass(void)  { }
  Module *clone() const { return(Module*)this;};

  virtual void do_file_block( FileBlock* file_block );
  virtual void do_class_type(FileSetBlock *f, ClassType *the_class,
                                list<MethodSymbol*> *super_list);
  PointerType* pointer_to(Type *type);

  static CProcedureType *vtable_entry_type;
};

#endif /* PORKY_SINGLE_VTABLE_H */
