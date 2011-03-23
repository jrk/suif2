/* file ecr_alias_vis.cc */

/*  Copyright (c) 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */


#include "ecr_alias_vis.h"
#include "ecr_alias_state.h"
#include "ecr_annotation_manager.h"

#include "suifkernel/walking_maps.h"
#include "suifkernel/utilities.h"
#include "common/suif_list.h"

#include "suifkernel/visitor_map.h"
#include "utils/symbol_utils.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "suifnodes/suif.h"
#include "iputils/lib_fileset_util.h"

// put this into the execution utilities
static ProcedureDefinition *get_procdef_from_scoped_object(ScopedObject *s) {
  if (s == NULL) return NULL;
  if (is_kind_of<ProcedureDefinition>(s)) 
    return(to<ProcedureDefinition>(s));

  SuifObject *par = s->get_parent();
  if (is_kind_of<ScopedObject>(s)) {
    return(get_procdef_from_scoped_object(to<ScopedObject>(par)));
  }
  return(NULL);
}

extern LString k_is_allocation;
extern bool ecr_progress;  


/*
 * forward declarations of the registered functions
 */
// they look like:
// void handle_static_pre_suif_object(SuifObject *SuifObject);
//
// We build prototypes here to make sure that
// all of the static functions are ok.

#define HANDLE_DISPATCH(order, lower, name) \
static void handle_static_ ## order ## _ ## lower(WalkingMaps *map, name * ## lower);
#include "defs.macro"
#undef HANDLE_DISPATCH


/*
 * ******************************************************
 * *
 * * ecr_alias actions
 * *
 * ******************************************************
 */

static void strip_tmp_ecr_annotes(ecr_annotation_manager *mgr) {
  //  bool no_strip = true;
  bool no_strip = false;
  //suif_assert_message(obj, ("strip_tmp_ecr_annotes: obj == NULL"));
  if (no_strip) return;
  static LString annotes_set[] = {
    k_ecr_tmp_instruction_result,
      //    k_ecr_tmp_instruction_multi_result,
    k_ecr_tmp_statement_result,
    k_ecr_tmp_variable_definition_result      
      //k_ecr_tmp_statement_multi_result
  };
#define NUM_ANNOTES (sizeof(annotes_set)/sizeof(LString))

  for (unsigned i = 0; i < NUM_ANNOTES; i++)
    mgr->clear_map(annotes_set[i]);
}

/* 
 * ******************************************************
 * *
 * * class ecr_alias_visitor
 * *
 * * Walk through the program representation and
 * * calculate the ecrs.
 * *
 * ******************************************************
 */


/*
 * forward declarations of the registered functions
 */
// they look like:
//   _maps->register_pre_visit_method(
//	   (VisitMethod)handle_static_pre_procedure_definition,
//	   ProcedureDefinition::get_class_name());

void ecr_alias_pass_init_suif_maps(WalkingMaps *maps) {
  maps->init_suif_object();
#define HANDLE_DISPATCH(order, lower, name) \
  maps->register_ ## order ## _visit_method( \
	   (VisitMethod)handle_static_ ## order ## _ ## lower ##, \
	   name ## ::get_class_name());
#include "defs.macro"
#undef HANDLE_DISPATCH
}



/*
 * ************************
 * * procedure and definition
 * ************************
 */

/*
 * The plan for handling variable arguments goes as follows:
 * 1. The procedure has an annotation that will give and ecr_node
 *    that points to a lambda_type that represents the procedure's
 *    inputs, outputs, varargs, whether is has varargs, and first
 *    variable argument.
 * 2. When taking the address, just create the node
 * 3. When applying a function, extend the arguments as far as needed
 *    If they are extended beyond the first variable argument, 
 *    do an assignment from the vararg to the argeument.
 * 4. When entering a function, assign all of the formal parameters
 *     to the parameters in the procedure's annotation. Extend if
 *     needed.
 * 5. va_start, va_arg, and va_end will meet the vararg with each of
 *     the extra parameters in this procedure (and modify the minimum varargs).
 *    va_start will also meet it's first argument with the vararg.
 *     TBD: it should reset the minimum varargs if the second arg is
 *     not the last fixed parameter.
 *    va_arg will assign the first argument to the destination.
 *    
 * This scheme will work FINE with FORTRAN
 * This scheme will NOT work if a vararg function is called
 *   and it doesn't use va_start or va_begin
 *
 */

static void handle_static_pre_procedure_definition(WalkingMaps *map,
						   ProcedureDefinition *tp) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();


  ProcedureSymbol *ps = tp->get_procedure_symbol();
  eas->add_reachable_procedure(ps);
  //ProcedureSymbol *ps = tp->proc();

  // Check for its use.
  // Skip all procedures in the library fileset that
  // are not marked as needing a correspondence.
  {
    /*
      if (sym_has_no_real_sym(ps)) {
      if (ecr_progress) {
      fprintf(stderr, "Skipping aliases on library proc: %s\n", 
      ps->name().chars());
      }
      _the_walker->set_skip();
      return;
      }
    */
  }
  if (ecr_progress) {
    fprintf(stderr, "Computing aliases on proc: %s\n", ps->get_name().c_str());
  }
  
  ProcedureSymbol *real_ps = eas->get_fileset_mgr()->find_real_file_proc(ps);
  //    ProcedureType *pt = real_ps->type();

  //    unsigned num_proc_args = pt->num_args();
  ecr_node *ptr_lambda = 
    annote_mgr->find_ecr_procedure(real_ps);
  ecr_node *lambda_node = 
    comp->get_lambda_pointed_to(ptr_lambda);
  lambda_type *lt = lambda_node->get_data()->get_lambda_type();

  unsigned num_formals = tp->get_formal_parameter_count();

  //  comp->extend_lambda_inputs(lt, num_formals);
  comp->extend_lambda_inputs(lambda_node, lt, num_formals);

  // just make sure to meet formals with the posted actuals.
  for (unsigned i = 0; i < num_formals; i++) {
    ParameterSymbol *param = tp->get_formal_parameter(i);
    VariableSymbol *var = param;
    ecr_node *formal = annote_mgr->find_ecr_variable(var);
    ecr_node *actual_t = comp->get_input_tau_pointed_to(lt, i);
    ecr_node *actual_l = comp->get_input_lambda_pointed_to(lt, i);
    eas->process_cjoin_x(formal, 
			 actual_t, actual_l);

  }
} 



static void handle_static_post_procedure_definition(WalkingMaps *map,
				    ProcedureDefinition *procdef) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  strip_tmp_ecr_annotes(annote_mgr);
  //procdef);
}



/*
 * ***********************
 * * value blocks
 * ***********************
 */


static void handle_static_post_expression_value_block(WalkingMaps *map,
						      ExpressionValueBlock *valblock) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  ecr_node *ex = annote_mgr->find_ecr_value_block_result(valblock);
  ecr_node *ey = annote_mgr->find_ecr_expression_result(valblock->get_expression());
  eas->process_assign(ex, ey);
}
static void handle_static_post_multi_value_block(WalkingMaps *map,
						 MultiValueBlock *valblock) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  ecr_node *ex = annote_mgr->find_ecr_value_block_result(valblock);
  for (Iter<MultiValueBlock::sub_block_pair > iter = 
	 valblock->get_sub_block_iterator();
       iter.is_valid(); iter.next()) {
    ValueBlock *sub_block = iter.current().second;
    ecr_node *ey = annote_mgr->find_ecr_value_block_result(sub_block);
    eas->process_assign(ex, ey);
  }
}
static void handle_static_post_repeat_value_block(WalkingMaps *map,
						  RepeatValueBlock *valblock) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  ecr_node *ex =
    annote_mgr->find_ecr_value_block_result(valblock);
  ecr_node *ey = 
    annote_mgr->find_ecr_value_block_result(valblock->get_sub_block());
  eas->process_assign(ex, ey);
}
static void handle_static_post_undefined_value_block(WalkingMaps *map,
						     UndefinedValueBlock *valblock) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  //  ecr_node *ex = annote_mgr->find_ecr_value_block_result(valblock);
  (void)annote_mgr->find_ecr_value_block_result(valblock);
}

/*
 * ***********************
 * * variable definitions
 * ***********************
 */

// Variable definition assigns its value to the 
// variable Symbol
static void handle_static_post_variable_definition(WalkingMaps *map,
						   VariableDefinition *vardef) {

  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  VariableSymbol *var = vardef->get_variable_symbol();
    
  ecr_node *ex = annote_mgr->find_ecr_variable(var);
  ecr_node *ey = annote_mgr->find_ecr_variable_definition_result(vardef);

  ValueBlock *initialization = vardef->get_initialization();
  
  ecr_node *ez = NULL;
  if (initialization)
    ez = annote_mgr->find_ecr_value_block_result(initialization);

  if (ez)
    eas->process_assign(ey, ez);

  eas->process_assign(ex, ey);
  strip_tmp_ecr_annotes(annote_mgr);
}
  
static void handle_static_pre_variable_symbol(WalkingMaps *map,
					      VariableSymbol *var) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  //  ecr_node *ex = annote_mgr->find_ecr_variable(var);
  (void)annote_mgr->find_ecr_variable(var);

  //  map->set_skip(true); // Don't look for owned objects

  if (!is_external_symbol_table(var->get_symbol_table()) ||
      !is_statically_allocated(var)) {
    return;
  }
    //  if (bst->is_global() &&
    //      !var->has_var_def()) {
    // If this is in the REAL fileset, make sure that
    // There is a library variable for this.
    // And that the library variable has a vardef.
  VariableSymbol *real_var = 
    eas->get_fileset_mgr()->find_real_var_sym(var);
  if (var != real_var) return;

  if (var->get_definition() != NULL) return;

  //  if (var == real_var) {
  VariableSymbol *lib_var = 
    eas->get_fileset_mgr()->find_library_var_sym(var);
  //if (lib_var != NULL)  return;
  if (lib_var == NULL) {
      if(!eas->is_silent()){
        fprintf(stderr, "WARNING: no library var for external var: %s\n",
	        var->get_name().c_str());
      }
    //    var->print(stderr);
  } else {
    //    if (!lib_var->has_var_def()) {
    if (lib_var->get_definition() == 0) {
        if(!eas->is_silent()){
            fprintf(stderr, "WARNING: no library var def for external var: %s\n",
	            var->get_name().c_str());
        //	          var->print(stderr);
        }
    }
  }
}

static void handle_static_pre_annote(WalkingMaps *map,
				     Annote *var) {
  map->set_skip(true);
}
  

// This may also be called by some other statements.
static void handle_static_post_statement(WalkingMaps *map,
					 Statement *st) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  strip_tmp_ecr_annotes(annote_mgr);
}

static void handle_static_post_for_statement(WalkingMaps *map,
					     ForStatement *tf) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  // need to meet the lb and step op with the index
  // variable
  Expression *lb = tf->get_lower_bound();
  //  Expression *ub = tf->get_upper_bound();  // ignore this
  Expression *step = tf->get_step();
  VariableSymbol *idx = tf->get_index();
    
  ecr_node *ex = annote_mgr->find_ecr_variable(idx);
  ecr_node *ey = annote_mgr->find_ecr_expression_result(lb);
  eas->process_assign(ex, ey);

  ecr_node *ez = annote_mgr->find_ecr_expression_result(step);
  eas->process_assign(ex, ez);

  strip_tmp_ecr_annotes(annote_mgr);
}

/*
static void handle_static_post_expression(Expression *i) {
  eas->do_instruction(i);
  
  // @@@ print this stuff
//  if (alias_verbose) {
//    i->print(stdout);
//    comp->print(stdout_ion);
//  }

}
*/


static void handle_static_post_array_reference_expression(WalkingMaps *map,
					  ArrayReferenceExpression *the_array) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  //void EcrAliasState::handle_array_reference_instruction(ArrayReferenceStatement *the_array) {
  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_array);
  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_array->get_base_array_address());
  eas->process_assign(ex, ey);
  
  eas->process_expression_dests(the_array);
  //assert(0);
}


static void handle_static_post_store_variable_statement(WalkingMaps *map,
				      StoreVariableStatement *the_store) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  ecr_node *ex = annote_mgr->find_ecr_statement_result(the_store);
  //  ecr_node *ey = annote_mgr->find_ecr_variable(the_store->get_destination());
  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_store->get_value());
  //eas->process_assign(ey, ez);
  eas->process_assign(ex, ey);
  eas->process_statement_dests(the_store);
}


/*
 * ************************
 * * instructions
 * ************************
 */

// The default case
//static void handle_static_post_expression(Expression *i) {
//  eas->do_instruction(i);
  
  // @@@ print this stuff
//  if (alias_verbose) {
//    i->print(stdout);
//    comp->print(stdout_ion);
//  }

//}


static void handle_static_post_expression(WalkingMaps *map,
					  Expression *the_instr) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_instr);
  if (ex == NULL) return;
  {
  for (Iter<Expression *> iter = the_instr->get_source_op_iterator();
       iter.is_valid(); iter.next()) {
    //  for (i = 0; i< the_instr->num_srcs(); i++) {
    Expression *op = iter.current();
    //    if (!op.is_null()) {
    // @@@ WHAT ABOUT NULL source ops?
    ecr_node *ey = annote_mgr->find_ecr_expression_result(op);
    eas->process_assign(ex, ey);
      //    }
  }
  }
  for (Iter<VariableSymbol *> iter = the_instr->get_source_var_iterator();
       iter.is_valid(); iter.next()) {
    //  for (i = 0; i< the_instr->num_srcs(); i++) {
    VariableSymbol *var = iter.current();
    //    if (!op.is_null()) {
    // @@@ WHAT ABOUT NULL source ops?
    ecr_node *ey = annote_mgr->find_ecr_variable(var);
    eas->process_assign(ex, ey);
      //    }
  }
  eas->process_expression_dests(the_instr);
}



static void handle_static_post_load_expression(WalkingMaps *map,
					       LoadExpression *the_load) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_load->get_source_address());
  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_load);
  ecr_node *load_from = annote_mgr->find_ecr_load_from(the_load);

  
  //  comp->cjoin(ey, load_from);
  comp->cjoin(load_from, ey);

  eas->process_load(ex, ey);

  eas->process_expression_dests(the_load);
}

static void handle_static_post_load_variable_expression(WalkingMaps *map,
							LoadVariableExpression *the_load) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  //  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  ecr_node *ey = annote_mgr->find_ecr_variable(the_load->get_source());
  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_load);
  eas->process_assign(ex, ey);
  //  comp->cjoin(ey, ex);

  eas->process_expression_dests(the_load);
}

// call Expression
/*
static void handle_static_post_call_expression(WalkingMaps *map,
						CallExpression *cal) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  eas->add_call_site(cal);

  // make SURE that the proc argument number
  // is the same as the number of operands

  // Get the address to call.  it will point to a lambda type.
  //  ecr_node *e = eas->get_ecr_annotation->find_ecr_expression_result(cal->callee_address());
  ecr_node *e = annote_mgr->find_ecr_expression_result(cal->get_callee_address());
  //ecr_node *t = comp->get_tau_pointed_to(e);
  ecr_node *l = comp->get_lambda_pointed_to(e);
  lambda_type *lt = l->get_data()->get_lambda_type();

  // Join this directly.
  ecr_node *ecall = annote_mgr->find_ecr_call_target(cal);
  comp->join(e, ecall);
	  

  // Special case for allocators
  if (lt->may_be_alloc()) {
    if (lt->is_alloc()) {
      eas->process_alloc(cal);
      return;
    } else {
      // get proc_name_from_suif_object
      // @@@ need to do this eventually
      //fprintf(stderr, "WARNING: IGNORING call site may_be_alloc: %s:i%d\n",
//	      cal->owner()->proc()->name(),
//	      cal->number());
      //
    }
  }
  unsigned num_call_args = cal->get_argument_count();

  unsigned num_call_dests = 1;
  if (is_kind_of<VoidType>(cal->get_result_type())) {
    num_call_dests = 0;
  }

  //  comp->extend_lambda_inputs(lt, num_call_args);
  //  comp->extend_lambda_outputs(lt, num_call_dests);
  comp->extend_lambda_inputs(l, lt, num_call_args);
  comp->extend_lambda_outputs(l, lt, num_call_dests);
  
  {

    // New plan.
    // walk through the pointed to args, 
    // If we run out of call arguments, extend them.
    //this->extend_lambda_inputs(lt, num_call_args);
    {
      unsigned i;
      for (i = 0; i< num_call_args; i++) {
	
	ecr_node *eyi = annote_mgr->find_ecr_expression_result(cal->get_argument(i));
	
	ecr_node *ez_tau = comp->get_input_tau_pointed_to(lt, i);
	ecr_node *ez_lambda = comp->get_input_lambda_pointed_to(lt, i);

	eas->process_cjoin_y(ez_tau, ez_lambda,
			eyi);
      }
    }
    {
      //      assert(num_call_dests <= lt->num_outputs(),
      //	     "Number of destination ops must be <= type arguments");
      //      this->extend_lambda_outputs(lt, num_call_dests);
      assert(num_call_dests <= 1);
      if (num_call_dests == 1) {
	//      for (unsigned i = 0; i < num_call_dests; i++) {
	ecr_node *exi = annote_mgr->find_ecr_expression_result(cal);
	ecr_node *ez_tau = comp->get_output_tau_pointed_to(lt, 0);
	ecr_node *ez_lambda = comp->get_output_lambda_pointed_to(lt, 0);

	eas->process_cjoin_x(exi,
			     ez_tau, ez_lambda);
      }
    }
  }
  eas->process_expression_dests(cal);
}
*/
static void handle_static_post_call_statement(WalkingMaps *map,
						CallStatement *cal) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  eas->add_call_site(cal);

  // make SURE that the proc argument number
  // is the same as the number of operands

  // Get the address to call.  it will point to a lambda type.
  ecr_node *e = annote_mgr->find_ecr_expression_result(cal->get_callee_address());
  ecr_node *l = comp->get_lambda_pointed_to(e);
  lambda_type *lt = l->get_data()->get_lambda_type();

  // Join this directly.
  ecr_node *ecall = annote_mgr->find_ecr_call_target(cal);
  comp->join(e, ecall);
	  

  // Special case for allocators
  if (lt->may_be_alloc()) {
    if (lt->is_alloc()) {
      eas->process_alloc(cal);
      return;
    } else {
      // get proc_name_from_suif_object
      /* @@@ need to do this eventually
      fprintf(stderr, "WARNING: IGNORING call site may_be_alloc: %s:i%d\n",
	      cal->owner()->proc()->name(),
	      cal->number());
      */
    }
  }
  unsigned num_call_args = cal->get_argument_count();

  unsigned num_call_dests = 1;
  if (cal->get_destination() == NULL)
    num_call_dests = 0;
  //  if (is_kind_of<VoidType>(cal->get_result_type())) {
  //    num_call_dests = 0;
  //  }

  comp->extend_lambda_inputs(l, lt, num_call_args);
  comp->extend_lambda_outputs(l, lt, num_call_dests);
  
  {

    // New plan.
    // walk through the pointed to args, 
    // If we run out of call arguments, extend them.
    {
      unsigned i;
      for (i = 0; i< num_call_args; i++) {
	
	ecr_node *eyi = annote_mgr->find_ecr_expression_result(cal->get_argument(i));
	
	ecr_node *ez_tau = comp->get_input_tau_pointed_to(lt, i);
	ecr_node *ez_lambda = comp->get_input_lambda_pointed_to(lt, i);

	eas->process_cjoin_y(ez_tau, ez_lambda,
			eyi);
      }
    }
    {
      //      assert(num_call_dests <= lt->num_outputs(),
      //	     "Number of destination ops must be <= type arguments");
      //      this->extend_lambda_outputs(lt, num_call_dests);
      assert(num_call_dests <= 1);
      if (num_call_dests == 1) {
	//      for (unsigned i = 0; i < num_call_dests; i++) {
	//	ecr_node *exi = annote_mgr->find_ecr_expression_result(cal);
	int i = 0;
	ecr_node *exi = annote_mgr->find_ecr_statement_result(cal);
	ecr_node *ez_tau = comp->get_output_tau_pointed_to(lt, i);
	ecr_node *ez_lambda = comp->get_output_lambda_pointed_to(lt, i);

	eas->process_cjoin_x(exi,
			     ez_tau, ez_lambda);
      }
    }
  }
  eas->process_statement_dests(cal);
}


static void 
handle_static_post_symbol_address_expression(WalkingMaps *map,
					   SymbolAddressExpression *lda) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  //ecr_computation *comp = eas->get_ecr_comp();
  // ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  //  if (is_post_node()) {
  {
    Symbol *sym = lda->get_addressed_symbol();
    assert(sym != NULL);
    if (is_kind_of<ProcedureSymbol>(sym)) {
      ProcedureSymbol *ps = to<ProcedureSymbol>(sym);
      eas->process_load_procedure_address_expression(lda, ps);
    } else {
      eas->process_load_symbol_address_expression(lda, sym);
    }
  }
}

  

static void handle_static_post_va_arg_expression(WalkingMaps *map,
						  VaArgExpression *the_va) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  //  assert(the_va->name() == k_va_arg);
  // If we assign it here to a Symbol, then FORCE
  // the join of all of the variable arguments
  
  ProcedureSymbol *ps = get_procdef_from_scoped_object(the_va)->get_procedure_symbol();
  lambda_type *lt = eas->meet_procedure_varargs(ps);
  //ecr_node *varargs_ez = meet_procedure_varargs(ps);
  

  //  if (the_va->dst_op().is_null()) {  return;  }

  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_va);
  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_va->get_ap_address());
  
  //    ProcedureDefinition *pd = get_procdef_from_zot(the_va);
  //  ProcedureSymbol *ps = the_va->owner()->proc();

  {
    ecr_node *ez_tau = comp->get_varargs_tau_pointed_to(lt);
    ecr_node *ez_lambda = comp->get_varargs_lambda_pointed_to(lt);
    
    eas->process_cjoin_y(ez_tau,
			 ez_lambda,
			 ey);
  }

  {
    ecr_node *ez_tau = comp->get_varargs_tau_pointed_to(lt);
    ecr_node *ez_lambda = comp->get_varargs_lambda_pointed_to(lt);
    
    eas->process_cjoin_x(ex, 
			 ez_tau,
			 ez_lambda);
  }
  eas->process_expression_dests(the_va);
}

static void handle_static_post_va_start_statement(WalkingMaps *map,
						  VaStartStatement *the_va) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  //  assert(the_va->name() == k_va_start);

  ProcedureSymbol *ps = get_procdef_from_scoped_object(the_va)->get_procedure_symbol();
  lambda_type *lt = eas->meet_procedure_varargs(ps);

  // This is screwed up.  Really it assigning arg1 to arg0.
  //  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_va);
  //  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_va->src_op(1));
  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_va->get_ap_address());
  
  //    ProcedureDefinition *pd = get_procdef_from_zot(the_va);
  //  ProcedureSymbol *ps = the_va->owner()->proc();
  //  ecr_node *ez = annote_mgr->find_ecr_tmp_procedure_varargs(ps);
  
  //  process_assign(ex, ey);
  ecr_node *ez_tau = comp->get_varargs_tau_pointed_to(lt);
  ecr_node *ez_lambda = comp->get_varargs_lambda_pointed_to(lt);

  eas->process_cjoin_x(ex, 
		       ez_tau, ez_lambda);
  //  process_assign(ex, varargs_ez);
  //  process_assign(ex, ez);
  //  process_expression_dests(the_va);
  //
}

static void handle_static_post_va_end_statement(WalkingMaps *map,
						  VaEndStatement *the_va) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  //ecr_computation *comp = eas->get_ecr_comp();
  //ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

//  assert(the_va->name() == k_va_end);

  ProcedureSymbol *ps = get_procdef_from_scoped_object(the_va)->get_procedure_symbol();
  eas->meet_procedure_varargs(ps);

  return;
}

/*
 * ************************
 * * special processing
 * ************************
 */


static void handle_static_post_store_statement(WalkingMaps *map,
					       StoreStatement *the_store) {
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();
  
  ecr_node *ex = annote_mgr->find_ecr_expression_result(the_store->get_destination_address());
  ecr_node *ey = annote_mgr->find_ecr_expression_result(the_store->get_value()); 
  ecr_node *store_to = annote_mgr->find_ecr_store_to(the_store);

  // This makes sense..
  //  comp->cjoin(store_to, ey);
 
  //comp->cjoin(ex, store_to);
  comp->cjoin(store_to, ex);

  eas->process_store(ex, ey);

  strip_tmp_ecr_annotes(annote_mgr);
  //the_store);
}




static void handle_static_post_return_statement(WalkingMaps *map,
						ReturnStatement *ret) { 
  EcrAliasState *eas = (EcrAliasState *)map->get_user_state();
  assert(eas->get_maps() == map);
  ecr_computation *comp = eas->get_ecr_comp();
  ecr_annotation_manager *annote_mgr = eas->get_ecr_annotation();

  // This is just an assignment from the source
  // operand to the function return value;
  // (kept on the ProcedureSymbol)
    
  // x = y becomes
  // x is procval
  ProcedureSymbol *ps =
    get_procdef_from_scoped_object(ret)->get_procedure_symbol();
  //const char *procname = ps->get_name().c_str();
  //ProcedureType *pt = unqualify_procedure_type(ps->get_type());
  //  assert(pt->have_result_info(), ("No result info for procedure: %s",
  //  				  procname));

  Expression *retval = ret->get_return_value();
  unsigned num_return_values = (retval != NULL);

  ProcedureSymbol *real_ps = eas->get_fileset_mgr()->find_real_file_proc(ps);

  ecr_node *ptr_lambda = 
    annote_mgr->find_ecr_procedure(real_ps);
  ecr_node *lambda_node = 
    comp->get_lambda_pointed_to(ptr_lambda);
  lambda_type *lt = lambda_node->get_data()->get_lambda_type();

  //  suif_assert_message(num_type_results == num_return_values, 
  //	     ("Procedure %s returning wrong number of values: %d != %d",
  //	      procname,
  //	      num_type_results, num_return_values));
  
  comp->extend_lambda_outputs(lambda_node, lt, num_return_values);
  //  comp->extend_lambda_outputs(lt, num_return_values);
  if (num_return_values) {
    //  for (size_t i = 0; i < num_return_values; i++) {
    ecr_node *ex_tau = comp->get_output_tau_pointed_to(lt, 0);
    ecr_node *ex_lambda = comp->get_output_lambda_pointed_to(lt, 0);
    ecr_node *ey = 
      annote_mgr->find_ecr_expression_result(ret->get_return_value());
    eas->process_cjoin_y(ex_tau, ex_lambda,
			 ey);
  }
  //  strip_tmp_ecr_annotes(ret);
  strip_tmp_ecr_annotes(annote_mgr);
}

