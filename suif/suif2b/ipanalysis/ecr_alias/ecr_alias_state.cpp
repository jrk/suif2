/* file ecr_alias_state.cc */

/*  Copyright (c) 1997-1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */


//#pragma implementation


#include "ecr_alias_state.h"
#include "ecr_computation.h"
#include "ecr_annotation_manager.h"

#include "common/suif_list.h"
#include "suifkernel/visitor_map.h"
#include "suifkernel/walking_maps.h"
#include "suifkernel/utilities.h"
#include "suifnodes/suif.h"
#include "utils/symbol_utils.h"
#include "iputils/lib_fileset_util.h"

#include <string.h>

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
 * ******************************************************
 * *
 * * ecr_alias actions
 * *
 * ******************************************************
 */


/* 
 * ******************************************************
 * *
 * * class EcrAliasState
 * *
 * * This is the state and public methods
 * * that can be used to calculate the ecrs.
 * *
 * ******************************************************
 */


EcrAliasState::EcrAliasState(SuifEnv *s,
			     ecr_computation *ecr_comp,
			     ecr_owner *ecr_owner,
			     ecr_annotation_manager *ecr_annotation,
			     FileSetMgr *fileset_mgr, bool silent) 
  : _suif_env(s),
    _ecr_comp(ecr_comp), 
    _ecr_owner(ecr_owner),
    _ecr_annotation(ecr_annotation),
    _maps(0),
    _reachable_procedures(),
    _fileset_mgr(fileset_mgr),
    _silent(silent)
{
  //  _maps = new WalkingMaps(s, this);
  //  _maps->init_suif_object();
  //  ecr_alias_dispatch::init_all(_map, this);
  // Here we need to add all the stuff
}

EcrAliasState::~EcrAliasState() {
  delete _maps;
}


SuifEnv *EcrAliasState::get_suif_env() const { return _suif_env; }
ecr_computation *EcrAliasState::get_ecr_comp() const { return _ecr_comp; }
ecr_owner *EcrAliasState::get_ecr_owner() const { return _ecr_owner; }
ecr_annotation_manager *EcrAliasState::get_ecr_annotation() const { return _ecr_annotation; }
WalkingMaps *EcrAliasState::get_maps() const { return(_maps); }
void EcrAliasState::set_maps(WalkingMaps *maps) { _maps = maps; }
FileSetMgr *EcrAliasState::get_fileset_mgr() const { return(_fileset_mgr); }

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


/*
void EcrAliasState::set_real_fileset(FileSetBlock *fileset) 
{
  extern FileSetBlock * real_fileset;
  real_fileset = fileset;
}

void EcrAliasState::add_library_fileset(FileSetBlock *fileset) 
{
  extern FileSetBlock * library_fileset;
  suif_assert_message(library_fileset == NULL, 
		      ("Currently only 1 library fileset allowed"));
  library_fileset = fileset;
}
*/

void EcrAliasState::process_the_fileset(FileSetBlock *fileset) 
{
  _maps->process_a_suif_object(fileset);
}

void EcrAliasState::process_a_symbol_table(SymbolTable *symtab) 
{
  _maps->process_a_suif_object(symtab);
}

void EcrAliasState::process_a_procedure_definition(ProcedureDefinition *procdef) 
{
  _maps->process_a_suif_object(procdef);
}
void EcrAliasState::process_a_variable_definition(VariableDefinition *vardef) 
{
  _maps->process_a_suif_object(vardef);
}

list<ProcedureSymbol *> *EcrAliasState::
get_reachable_procedures() const {
  list<ProcedureSymbol *> *the_list = new list<ProcedureSymbol*>();
  for (list<ProcedureSymbol*>::iterator iter = _reachable_procedures.begin();
       iter != _reachable_procedures.end(); iter++) {
    the_list->push_back(*iter);
  }
  return(the_list);
}
void EcrAliasState::add_reachable_procedure(ProcedureSymbol* ps) {
  for (list<ProcedureSymbol*>::iterator iter = 
	 _reachable_procedures.begin();
       iter != _reachable_procedures.end(); iter++) {
    if ((*iter) == ps) return;
  }
  _reachable_procedures.push_back(ps);
}

list<CallStatement *> *EcrAliasState::
get_call_sites() const {
  list<CallStatement *> *the_list = new list<CallStatement*>();
  for (list<CallStatement*>::iterator iter = _call_sites.begin();
       iter != _call_sites.end(); iter++) {
    the_list->push_back(*iter);
  }
  return(the_list);
}
void EcrAliasState::add_call_site(CallStatement* ps) {
  for (list<CallStatement*>::iterator iter = 
	 _call_sites.begin();
       iter != _call_sites.end(); iter++) {
    if ((*iter) == ps) return;
  }
  _call_sites.push_back(ps);
}



void EcrAliasState::process_expression_dests(Expression *the_expr) {
  return;
}

void EcrAliasState::process_statement_dests(Statement *the_statement) {
  ecr_annotation_manager *annote_mgr = get_ecr_annotation();
  Iter<VariableSymbol*> iter = the_statement->get_destination_var_iterator();
  //  unsigned num_dests = iter.length();//get_destination_var_count(the_statement);
  for (unsigned i = 0; iter.is_valid(); iter.next(), i++) {
    //    suif_assert(i < num_dests);
    ecr_node *ey = annote_mgr->find_ecr_statement_result(the_statement);
    VariableSymbol *var = iter.current();
    if (var == NULL) continue;
    suif_assert(is_kind_of<VariableSymbol>(var));
    ecr_node *ex = annote_mgr->find_ecr_variable(var);
    
    process_assign(ex, ey);
  }
}



void EcrAliasState::process_load_procedure_address_expression(SymbolAddressExpression *the_load, ProcedureSymbol *ps) {
  
  ecr_node *ex = get_ecr_annotation()->find_ecr_expression_result(the_load);
  process_procedure_address_taken(ex, ps);
  process_expression_dests(the_load);
}


void EcrAliasState::process_load_symbol_address_expression(SymbolAddressExpression *the_load, Symbol *sym) {
  suif_assert_message(is_kind_of<VariableSymbol>(sym), 
		      ("Only variable symbols and procedures symbols may have addresses taken"));
  VariableSymbol *var = to<VariableSymbol>(sym);

  ecr_node *ex = get_ecr_annotation()->find_ecr_expression_result(the_load);
  process_variable_address_taken(ex, var);

  process_expression_dests(the_load);
}


/* This is complex.
 * we need to take a look at the number of procedure
 * parameters in the procedure. (The procedure must be in memory)
 * and then take all the ones above this number and
 * cjoin them with the varargs.
 * 
 * The number of params must come from the procedure under exam
 * The rest of the data comes from the real procedure
 */

lambda_type *EcrAliasState::meet_procedure_varargs(ProcedureSymbol *ps) {

  ProcedureSymbol *real_ps = _fileset_mgr->find_real_file_proc(ps);

  unsigned num_fixed_args = ps->get_definition()->get_formal_parameter_count();

  ecr_node *ptr_lambda = 
    get_ecr_annotation()->find_ecr_procedure(real_ps);
  ecr_node *lambda_node = 
    get_ecr_comp()->get_lambda_pointed_to(ptr_lambda);
  lambda_type *lt = lambda_node->get_data()->get_lambda_type();

  get_ecr_comp()->reset_lambda_min_vararg(lambda_node, lt, num_fixed_args);
  //  get_ecr_comp()->reset_lambda_min_vararg(lt, num_fixed_args);

  return (lt);
}
  


/*
 * ************************
 * * special processing
 * ************************
 */

void EcrAliasState::process_load(ecr_node *ex, ecr_node *ey) { // x = *y
  ecr_node *t1 = get_ecr_comp()->get_tau_pointed_to(ex);
  ecr_node *l1 = get_ecr_comp()->get_lambda_pointed_to(ex);

  ecr_node *t2 = get_ecr_comp()->get_tau_pointed_to(ey);
  //  ecr_node *l2 = get_ecr_comp()->get_lambda_pointed_to(ey);

  if (t2->get_data()->is_bottom()) {
    get_ecr_comp()->set_type_to_new_tau(t2,
					t1, l1);
  } else {
    process_cjoin_y(t1, l1,
		    t2);
  }
}

void EcrAliasState::process_alloc(CallStatement *cal) {
  // make SURE that the proc argument number
  // is the same as the number of operands
  ecr_node *e = get_ecr_annotation()->find_ecr_statement_result(cal);
  ecr_node *a = get_ecr_annotation()->find_ecr_allocation(cal);
  process_assign(a, e);
  ecr_node *t = get_ecr_comp()->get_tau_pointed_to(e);
  //  ecr_node *l = get_ecr_comp()->get_lambda_pointed_to(e);

  assert(t->get_data()->is_tau());
  tau_type *tau = STATIC_CAST(tau_type *,t->get_data());
  // Throw away the lambda.
  if (tau->is_bottom()) {
    // @@@ we should also add the allocation site to the owners.
    // in addition, we should fix the join of an allocator and
    // a non-allocator.
    //    get_ecr_comp()->set_type(t, get_ecr_comp()->new_tau_type());
    get_ecr_comp()->set_type_to_new_tau(t);

  }
  process_statement_dests(cal);
}



void EcrAliasState::process_store(ecr_node *ex, ecr_node *ey) { // *x = y
  ecr_node *t1 = get_ecr_comp()->get_tau_pointed_to(ex);
  //  ecr_node *l1 = get_ecr_comp()->get_lambda_pointed_to(ex);

  ecr_node *t2 = get_ecr_comp()->get_tau_pointed_to(ey);
  ecr_node *l2 = get_ecr_comp()->get_lambda_pointed_to(ey);

  if (t1->get_data()->is_bottom()) {
    //@@@ warning this will OVERWRITE the old stuff..
    get_ecr_comp()->set_type_to_new_tau(t1,
					t2, l2);
    //    get_ecr_comp()->set_type(t1, get_ecr_comp()->new_tau_type(t2, l2));
  } else {
    process_cjoin_x(t1,
		    t2, l2);
  }
}



// This processes an assignment:  ex = ey
void EcrAliasState::process_assign(ecr_node *ex, ecr_node *ey) {
  ecr_node *ex_tau = get_ecr_comp()->get_tau_pointed_to(ex);
  ecr_node *ex_lambda = get_ecr_comp()->get_lambda_pointed_to(ex);

  ecr_node *ey_tau = get_ecr_comp()->get_tau_pointed_to(ey);
  ecr_node *ey_lambda = get_ecr_comp()->get_lambda_pointed_to(ey);
  process_cjoin(ex_tau, ex_lambda, ey_tau, ey_lambda);
}


// This processes an assignment:  ex = ey
void EcrAliasState::process_cjoin_x(ecr_node *ex,
					ecr_node *ey_tau, ecr_node *ey_lambda)
{
  ecr_node *ex_tau = get_ecr_comp()->get_tau_pointed_to(ex);
  ecr_node *ex_lambda = get_ecr_comp()->get_lambda_pointed_to(ex);
  process_cjoin(ex_tau, ex_lambda, ey_tau, ey_lambda);
}

// This processes an assignment:  ex = ey
void EcrAliasState::process_cjoin_y(ecr_node *ex_tau, ecr_node *ex_lambda, 
				       ecr_node *ey) {
  ecr_node *ey_tau = get_ecr_comp()->get_tau_pointed_to(ey);
  ecr_node *ey_lambda = get_ecr_comp()->get_lambda_pointed_to(ey);
  process_cjoin(ex_tau, ex_lambda, ey_tau, ey_lambda);
}

void EcrAliasState::process_cjoin(ecr_node *ex_tau, ecr_node *ex_lambda, 
				      ecr_node *ey_tau, ecr_node *ey_lambda) {
  assert(ex_tau->get_data()->is_tau() && ex_lambda->get_data()->is_lambda() &&
	 ey_tau->get_data()->is_tau() && ey_lambda->get_data()->is_lambda());
  get_ecr_comp()->cjoin(ex_tau, ey_tau);
  get_ecr_comp()->cjoin(ex_lambda, ey_lambda);
}

void EcrAliasState::process_variable_address_taken(ecr_node *ex, VariableSymbol *var) {
  if (ex == NULL) return;
  // ignore offsets for this analysis
  ecr_node *ey = get_ecr_annotation()->find_ecr_variable(var);
      
  ecr_node *t1 = get_ecr_comp()->get_tau_pointed_to(ex);
  //  ecr_node *l1 = get_ecr_comp()->get_lambda_pointed_to(ex);
      
  ecr_node *t2 = ey;

  get_ecr_comp()->join(t1, t2);
}

static char *allocator_names[] = {
  "malloc", "calloc", "alloca", "sbrk", 
  "operator new[]",
  "operator new"
};

#define NUM_ALLOCATORS (sizeof(allocator_names)/sizeof(char *))

// I wouldn't mind being able to pass in names from the 
// command line.  like stralloc...

bool EcrAliasState::proc_is_allocator(ProcedureSymbol *ps) const {
  unsigned i;

  for (i = 0; i < NUM_ALLOCATORS; i++) {
    if (!strcmp(allocator_names[i], ps->get_name().c_str())) {
      return(true);
    }
  }
  return(false);
}


/*
 * ************************
 * * special processing
 * ************************
 */


// Get the inputs outputs, and varargs from the procedure declaration.
// create a lambda type for this address taken value.
// assign the procedure decls to the lambda type.
// Anything that doesn't match becomes a vararg.
void EcrAliasState::process_procedure_address_taken(ecr_node *ex, 
						    ProcedureSymbol *ps) {
  // need the REAL ProcedureSymbol and the REAL or LIBRARY block
  ProcedureSymbol *real_ps = _fileset_mgr->find_real_file_proc(ps);

  // If this procedure has no definition, then
  // check that the library version exists. If not,
  // give a warning
  // 
  //  if (!real_ps->is_written() && !real_ps->is_readable() &&
  //      !real_ps->is_in_memory()) {
  //  if (real_ps->get_procedure_definition() == NULL) {
  if (real_ps->get_definition() == NULL) {
    // This is just used for it's side effect of printing
    // a message when a procedure is not found.
    if (!proc_is_allocator(real_ps)) {
      ProcedureSymbol *lib_ps = _fileset_mgr->find_library_file_proc(ps);
      if (lib_ps == NULL) {
	if (!is_attribute_annote(ps, "warned_procedure")) {
	  add_attribute_annote(get_suif_env(), ps, "warned_procedure");
      if(!_silent){
        fprintf(stderr, "WARNING: procedure '%s' not found.\n",
		  real_ps->get_name().c_str());
      }
	}
      }
    }
  }


  //  ProcedureType *pt = real_ps->type();
  //  assert(is_basic_procedure_type(pt), "not basic proc type");

  // We'll be meeting our record with this one.

  // Just adds the procsym to a list.
  ecr_computation *comp = get_ecr_comp();
  ecr_node *lambda_ptr = 
    get_ecr_annotation()->find_ecr_procedure(real_ps);
  ecr_node *lambda_node = comp->get_lambda_pointed_to(lambda_ptr);

  //  ecr_node *lambda_node = comp->new_empty_lambda_node();
  lambda_type *the_proc_lambda = lambda_node->get_data()->get_lambda_type();

  //
  // Check for allocation
  //
  if (proc_is_allocator(real_ps)) {
    suif_assert_message(_fileset_mgr->get_proc_block(real_ps) == NULL,
	       ("Allocator for %s has a body", real_ps->get_name().c_str()));
    the_proc_lambda->set_is_alloc();
    // generate an is_alloc annotation.
    add_attribute_annote(real_ps->get_suif_env(), real_ps, k_is_allocation);
    //    return;
  }

  // create a new lambda;

  the_proc_lambda->add_target(real_ps); // just for cute output.
  // for the pointer-only algorithm.
  comp->set_proc_reachable(lambda_node);

  add_reachable_procedure(real_ps);

  //  process_assign(ex, the_new_tau);
  process_assign(ex, lambda_ptr);
}







