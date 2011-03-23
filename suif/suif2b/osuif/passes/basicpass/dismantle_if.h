/* file "dismantle_if.h" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


#ifndef PORKY_DISMANTLE_IF_H
#define PORKY_DISMANTLE_IF_H

#ifndef SUPPRESS_PRAGMA_INTERFACE
#pragma interface
#endif

#include <suifpasses.h>
#include <module_subsystem.h>
#include <suif_env.h>

#include <suif.h>
/*
      This is the interface to dismantling passes of the porky
      library.
*/

/*
 *  PREREQUISITES: "Statement List Form"
 *                 "Statements Only In Statement Lists"
 *                 "Statements Only In Procedure Definitions"
 *                 "All Szots Scoped"
 *                 "Standard If Statement Semantics"
 *                 "Standard Statement List Semantics"
 *                 "Standard Branch Statement Semantics"
 *                 "Standard Jump Statement Semantics"
 *                 "Standard Label Location Statement Semantics"
 *                 "Standard If Statement Completeness"
 *  GENERATES: "No If Statements"
 */

class dismantle_if_statements_pass : public PipelinablePass
{
public:
  dismantle_if_statements_pass(SuifEnv *env, const LString &name);
  virtual ~dismantle_if_statements_pass(void)  { }

  virtual void do_procedure_definition(ProcedureDefinition *proc_def);

  // The dismantling "Action"
  // This will clone anything it needs.
  virtual StatementList *dismantle_if_statement(IfStatement *the_if);
  Module *clone() const { return(Module*)this;};
};

#endif /* PORKY_DISMANTLE_IF_H */
