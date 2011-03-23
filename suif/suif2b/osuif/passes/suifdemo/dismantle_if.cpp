/* file "dismantle_if.cc" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


/*
      This is the implementation of dismantling passes of the porky
      library.
*/


#define _MODULE_ "libporky"

#pragma implementation "dismantle_if.h"

#include <cast.h>
#include <utilities.h>
#include <error_macros.h>
#include <i_integer.h>
#include <basic_factory.h>
#include <suif.h>
#include <suif_factory.h>
#include <basic_constants.h>
#include <suifkernel_messages.h>

#include "dismantle_if.h"
#ifdef HACK
#include <symbol_utils.h>
#include <execution_utils.h>
#endif

dismantle_if_statements_pass::
dismantle_if_statements_pass(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {};


list<IfStatement*>* collect_if_objects( SuifObject* start_object ) {
  list<IfStatement*>* l = new list<IfStatement*>;
  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( IfStatement::get_class_name() );


    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      l->push_back( (IfStatement*)it->current() );
      it->next();
    }
    delete it;
  }
  return l;
}


void dismantle_if_statements_pass::do_procedure_definition(
			   ProcedureDefinition *proc_def)
{

  //  list<IfStatement *> *l = collect_objects<IfStatement>(proc_def);
  list<IfStatement *> *l = collect_if_objects(proc_def);
  for (list<IfStatement *>::iterator iter(l->begin());
       iter != l->end(); iter++) {
    IfStatement *the_if = *iter;
    StatementList *replacement = dismantle_if_statement(the_if);
    if (replacement == NULL) continue;
    // ignore the return value here.
    replace_statement_with_list(get_suif_env(), the_if, replacement);
  }
}  


StatementList *dismantle_if_statements_pass::
dismantle_if_statement(IfStatement *the_if)
  {
    suif_assert(the_if != NULL);

    SuifEnv *s = get_suif_env();
    StatementList *replacement = create_statement_list(s);

    // 
    // convert
    // Case 1: has 'else' and 'then' part:
    // if (expr) then Then_Statement else Else_Statement
    // into
    //   if (!expr) goto __else
    //   Then_Statement
    //   goto __done
    // __else:
    //   Else_Statement
    // __done:
    /*
     * we ignore the rest of these cases 
     * and the case where expression is a constant.
     */
    // Case 2: has 'then' part. no 'else' part:
    // if (expr) then Then_Statement else Else_Statement
    // into
    //   if (!expr) goto __done
    //   Then_Statement
    // __done:
    // Case 3: has 'else' part. no 'then' part:
    // if (expr) then Then_Statement else Else_Statement
    // into
    //   if (expr) goto __done
    //   Else_Statement
    // __done:
    // Case 4: has no 'else' part. no 'then' part:
    // if (expr) then Then_Statement else Else_Statement
    // into
    //   eval(expr)

    SourceOp condition = the_if->get_condition();
    SourceOp new_condition = deep_clone_source_op(condition);
    
    //    the_if->set_condition(SourceOp());

    CodeLabelSymbol *else_label = new_unique_label(s, the_if, "__else");
    CodeLabelSymbol *done_label = new_unique_label(s, the_if, "__done");

    Statement *then_statement = the_if->get_then_part();
    Statement *else_statement = the_if->get_else_part();


    if (then_statement && else_statement) {
      //   if (!expr) goto __else
      replacement->
	append_statement(create_branch_statement(s, k_branch_if_false,
						 new_condition, else_label));
      
      // Then_Statement
      //      if (then_statement != NULL) {
	Statement *new_then_statement = deep_suif_clone(then_statement);
	replacement->append_statement(new_then_statement);
	//      }
    
      // goto __done
      replacement->
	append_statement(create_jump_statement(s, done_label));
      // __else:
      replacement->
	append_statement(create_label_location_statement(s, else_label));
      
      // Else_Statement
      //      if (else_statement != NULL) {
	Statement *new_else_statement = deep_suif_clone(else_statement);
	replacement->append_statement(new_else_statement);
	//      }
      
      // __done:
      replacement->
	append_statement(create_label_location_statement(s, done_label));
    }
    if (then_statement && !else_statement) {
      //   if (!expr) goto __done
      replacement->
	append_statement(create_branch_statement(s, k_branch_if_false,
						 new_condition, done_label));
      
      // Then_Statement
      if (then_statement != NULL) {
	Statement *new_then_statement = deep_suif_clone(then_statement);
	replacement->append_statement(new_then_statement);
      }
    
      // __done:
      replacement->
	append_statement(create_label_location_statement(s, done_label));
      
    }
    if (!then_statement && else_statement) {
      //   if (expr) goto __done
      replacement->
	append_statement(create_branch_statement(s, k_branch_if_true,
						 new_condition, done_label));
      
      // Else_Statement
      Statement *new_else_statement = deep_suif_clone(else_statement);
      replacement->append_statement(new_else_statement);
      
      // __done:
      replacement->
	append_statement(create_label_location_statement(s, done_label));
    }
    if (!then_statement && !else_statement) {
      //   eval (expr)
      EvalStatement *ev = create_eval_statement(s);
      // @@@ This is illegal!!
      ev->append_expression(create_copy_expression(s, 
          to<DataType>(new_condition.get_type()), new_condition));
      replacement->append_statement(ev);
    }
    
    
    return(replacement);
  }


