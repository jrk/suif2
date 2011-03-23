

#include "ecr_annotation_manager.h"
#include "ecr_computation.h"

#include "ecrnodes/ecr.h"
#include "ecrnodes/ecr_factory.h"

// For the annotation creation
#include "basicnodes/basic_factory.h" 
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"
#include "common/lstring.h"
#include "common/suif_hash_map.h"
#include "bit_vector/cross_map.h"


#include "ecrnodes/ecr_factory.h"
#include "ecrnodes/ecr.h"
#include "ecrnodes/ecr_constants.h"
#include "iputils/lib_fileset_util.h"

LString k_ecr_tmp_statement_result = "ecr_tmp_statement_result";
LString k_ecr_tmp_value_block_result = "ecr_tmp_value_block_result";
LString k_ecr_tmp_instruction_result = "ecr_tmp_instruction_result";
LString k_ecr_tmp_variable_definition_result = 
"ecr_tmp_variable_definition_result";

/*
 * These are different between SUIF1 and SUIF2
 */
bool is_attribute_annote(AnnotableObject *the_azot, const LString & k_tmp) {
  return(the_azot->lookup_annote_by_name(k_tmp) != NULL);
}

void add_attribute_annote(SuifEnv *s, AnnotableObject *the_azot, 
			  const LString & k_tmp) {
  if (!is_attribute_annote(the_azot, k_tmp)) {
    the_azot->append_annote(create_brick_annote(s, k_tmp));
  }
}

/*
 * Here are my annotations
 */


/*
 * ******************************
 * * 
 * * Annotation Manager
 * *
 * ******************************
 */


ecr_annotation_manager::ecr_annotation_manager(SuifEnv *suif_env, 
					       ecr_computation *ecr_comp,
					       FileSetMgr *fileset_mgr) :
  _suif_env(suif_env), 
  _ecr_comp(ecr_comp),
  _fileset_mgr(fileset_mgr),
  _map(new ecr_annotation_manager::full_ecr_map())
{
}

#if 0
ecr_node *ecr_annotation_manager::
create_ecr_for_obj(AnnotableObject *obj,
		   const LString & k_tmp) {
  ecr_node *ecr = _ecr_comp->new_tau_node();
  if (alias_verbose) {
    stdout_ion->printf("BINDING: (%s) %d -> ",
		       k_tmp.c_str(),
		       get_ecr_nodeset_id(ecr).c_int());
    if (is_kind_of<Symbol>(the_azot)) {
      Symbol *sym = (Symbol *)the_azot;
      stdout_ion->printf("%s", sym->get_name().c_str());
      //sym->print();
    } else if (is_kind_of<Expression>(the_azot)) {
      Expression *i = (Expression *)the_azot;
      stdout_ion->printf("%s:i#??",
			 find_proc_from_suif_object(i)->get_procedure_symbol()->get_name().c_str());
			 //i->number());
    }
    stdout_ion->printf("\n");
#ifdef UGH
    the_azot->print(stdout_ion);
#endif
  }
  if (is_kind_of<VariableSymbol>(the_azot)) {
    VariableSymbol *var = to<VariableSymbol>(the_azot);
    STATIC_CAST(tau_type *,ecr->get_data())->add_target(var);
  }
  return(ecr);
}
#endif

#if 0
ecr_node *ecr_annotation_manager::
create_empty_ecr_by_num(const LString & k_tmp, 
			AnnotableObject *the_azot,
			unsigned num){
  ecr_node *ecr = create_ecr_for_azot(the_azot, k_tmp);
  EcrRefAnnote *an = create_ecr_ref_annote(_suif_env, k_tmp);
  for (unsigned i =0; i <= num; i++) {
    EcrOpRef *opref = create_ecr_op_ref(_suif_env, i, 0, 0);
    opref->clear_ecr();
    an->append_op(opref);
  }
  an->get_op(num)->set_ecr(ecr);
  append_an_annote(the_azot, an);
  return(ecr);
}
#endif

ecr_annotation_manager::ecr_map *ecr_annotation_manager::get_ecr_map(const LString &k_tmp) const
{
  full_ecr_map::iterator iter = _map->find(k_tmp);
  if (iter != _map->end())
    return (*iter).second;
  return(NULL);
}

ecr_annotation_manager::ecr_map *ecr_annotation_manager::
retrieve_ecr_map(const LString &k_tmp)
{
  ecr_map *map = get_ecr_map(k_tmp);
  if (map) return(map);

  map = new ecr_map;
  // (*_map)[k_tmp] = map;
  _map->enter_value(k_tmp, map);
  return(map);
}



// retrieve it and update it along the way.
ecr_node *ecr_annotation_manager::retrieve_ecr(ecr_map *map,
					       AnnotableObject *obj)
{
  ecr_map::iterator iter = map->find(obj);
  if (iter != _map->end())
    {
      ecr_node *ecr = (*iter).second;
      // update it here.
      ecr_node *ecr_new = ecr->find_top();
      if (ecr_new != ecr) {
	add_ecr_ref(ecr_new);
	remove_ecr_ref(ecr);
	// (*map)[obj] = ecr_new;
	map->enter_value(obj, ecr_new);
      }
      return(ecr_new);
    }
  
  // Otherwise, build one
  ecr_node *ecr = _ecr_comp->new_tau_node();
  if (is_kind_of<VariableSymbol>(obj)) {
    VariableSymbol *var = to<VariableSymbol>(obj);
    ecr->get_data()->get_tau_type()->add_target(var);
    //    STATIC_CAST(tau_type *,ecr->get_data())->add_target(var);
  }
  // reference count starts at 2???
  // (*map)[obj] = ecr;
  map->enter_value(obj, ecr);
  add_ecr_ref(ecr);
  return(ecr);
}
#if 0  
  if (alias_verbose) {
    stdout_ion->printf("BINDING: (%s) %d -> ",
		       k_tmp.c_str(),
		       get_ecr_nodeset_id(ecr).c_int());
    if (is_kind_of<Symbol>(the_azot)) {
      Symbol *sym = (Symbol *)the_azot;
      stdout_ion->printf("%s", sym->get_name().c_str());
      //sym->print();
    } else if (is_kind_of<Expression>(the_azot)) {
      Expression *i = (Expression *)the_azot;
      stdout_ion->printf("%s:i#??",
			 find_proc_from_suif_object(i)->get_procedure_symbol()->get_name().c_str());
			 //i->number());
    }
    stdout_ion->printf("\n");
#ifdef UGH
    the_azot->print(stdout_ion);
#endif
  }
  if (is_kind_of<VariableSymbol>(the_azot)) {
    VariableSymbol *var = to<VariableSymbol>(the_azot);
    STATIC_CAST(tau_type *,ecr->get_data())->add_target(var);
  }
  return(ecr);
#endif

ecr_node *ecr_annotation_manager::retrieve_named_ecr(const LString &k_tmp, 
						     AnnotableObject *obj)
{
  ecr_map *emap = retrieve_ecr_map(k_tmp);
  ecr_node *node = retrieve_ecr(emap, obj);
  return(node);
}

#if 0
ecr_node *ecr_annotation_manager::retrieve_named_ecr_by_num(const LString & k_tmp,
						   AnnotableObject *the_azot, 
						   unsigned num){
  Annote *the_an = peek_an_annote(the_azot, k_tmp);
  EcrRefAnnote *an = to<EcrRefAnnote>(the_an);
  
  if (an == NULL) {
    return(create_empty_ecr_by_num(k_tmp, the_azot, num));
  }
  while ((int)num >= an->get_op_count()) {
    EcrOpRef *ref = 
      create_ecr_op_ref(_suif_env, 
			an->get_op_count(),
			0, 0);
    ref->clear_ecr();
    an->append_op(ref);
  }
  ecr_node *the_ecr = an->get_op(num)->get_ecr();//ecr_by_num(num);
  if (the_ecr == NULL) {
    ecr_node *ecr = create_ecr_for_azot(the_azot, k_tmp);
    an->get_op(num)->set_ecr(ecr);
  }
  return(an->get_op(num)->get_ecr());
}
#endif

static bool is_external_symbol_table(SymbolTable *symtab) {
  SuifObject *par = symtab->get_parent();
  if (par == NULL) return false;
  if (!is_kind_of<FileSetBlock>(par)) return false;
  return (to<FileSetBlock>(par)->get_external_symbol_table() == symtab);
}

static bool is_global_variable(VariableSymbol *var) {
  SymbolTable *symtab = var->get_symbol_table();
  if (symtab == NULL) return NULL;
  return(is_external_symbol_table(symtab));
}

ecr_node *ecr_annotation_manager::find_ecr_variable(VariableSymbol *var) {
  VariableSymbol *target = var;
  if (is_global_variable(var)) {
    //_fileset_mgr->set_suppress_var_sym_warning(true);
    target = _fileset_mgr->find_real_var_sym(var);
    //_fileset_mgr->set_suppress_var_sym_warning(false);
    if (target == NULL) target = var;
  }
  return(retrieve_named_ecr(k_ecr_variable, target));
}


ecr_node *ecr_annotation_manager::find_ecr_from_dst(Statement *st,
						    unsigned opn) {
  Iter<VariableSymbol*> iter = st->get_destination_var_iterator();
  iter.set_to(opn);
  suif_assert(iter.is_valid());
  VariableSymbol *var = iter.current();
  return(find_ecr_variable(var));
}

unsigned get_dest_op_count(Expression *e) {
  //  return get_num_results_from_type(e->get_result_type());
  // until we have multiple results, this is always 1
  return(1);
}

void ecr_annotation_manager::setup_procedure(ProcedureSymbol *ps) {
  (void)retrieve_named_ecr(k_ecr_procedure_symbol, ps);
}

ecr_node *ecr_annotation_manager::find_ecr_procedure(ProcedureSymbol *ps) {
  setup_procedure(ps); return(retrieve_named_ecr(k_ecr_procedure_symbol, ps)); 
}

ecr_node *ecr_annotation_manager::find_ecr_expression_result(Expression *i) {
  return(retrieve_named_ecr(k_ecr_tmp_instruction_result, i)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_variable_definition_result(VariableDefinition *v) {
  return(retrieve_named_ecr(k_ecr_tmp_variable_definition_result, v));
}
#if 0
ecr_node *ecr_annotation_manager::
find_ecr_statement_multi_result(Statement *i, unsigned opn) {
  return(retrieve_named_ecr_by_num(k_ecr_tmp_statement_multi_result, i, opn)); 
}
#endif

ecr_node *ecr_annotation_manager::
find_ecr_statement_result(Statement *i) 
{
  return(retrieve_named_ecr(k_ecr_tmp_statement_result, i)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_value_block_result(ValueBlock *valblock) {
  suif_assert_message(valblock, ("Null Value block"));
  return(retrieve_named_ecr(k_ecr_tmp_value_block_result, valblock)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_call_target(ExecutionObject *cal) {
  return(retrieve_named_ecr(k_ecr_call_targets, cal)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_allocation(ExecutionObject *cal) {
  return(retrieve_named_ecr(k_ecr_allocation, cal)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_store_to(StoreStatement *the_store) {
  return(retrieve_named_ecr(k_ecr_store_to, the_store)); 
}

ecr_node *ecr_annotation_manager::
find_ecr_load_from(LoadExpression *the_load) {
  return(retrieve_named_ecr(k_ecr_load_from, the_load)); 
}



void remove_ecr_ref(ecr_node *node) {
  node->owner()->remove_ref(node);
}
void add_ecr_ref(ecr_node *node) {
  node->owner()->add_ref(node);
}

IInteger get_ecr_nodeset_id(ecr_node *node) {
  return(node->owner()->get_nodeset_id(node));
}

ecr_node *find_top_ecr(ecr_node *node) {
  return(node->find_top());
}

ecr_node *find_ecr(ecr_node *node) {
  return(node->find());
}

ecr_node *find_the_tau_pointed_to(ecr_node *node) {
  ecr_node *out_ecr = ((tau_type *)node->get_data())->get_tau_pointed_to();
  out_ecr = out_ecr->find();
  return(out_ecr);
}

ecr_node *find_the_lambda_pointed_to(ecr_node *node) {
  ecr_node *out_ecr = ((tau_type *)node->get_data())->get_lambda_pointed_to();
  out_ecr = out_ecr->find();
  return(out_ecr);
}
bool is_ecr_bottom(ecr_node *node) {
  return(node->get_data()->is_bottom());
}

ProcedureDefinition *find_proc_from_suif_object(SuifObject *s) {
  SuifObject *par = s->get_parent();
  while (par != NULL) {
    if (is_kind_of<ProcedureDefinition>(par)) {
      return(to<ProcedureDefinition>(par));
    }
    par = par->get_parent();
  }
  return NULL;
}  

void ecr_annotation_manager::clear_map(const LString &k_tmp)
{
  ecr_map *map = get_ecr_map(k_tmp);
  if (map == NULL) return;
  
  
}

#if 0
EcrRefAnnote *ecr_annotation_manager::get_ref_annote(EcrOpRef *op) const {
  return(to<EcrRefAnnote>(op->get_parent()));
}

void ecr_annotation_manager::clear_ecr(EcrOpRef *op)
{ 
  // we need this because _ecr won't be initialized without it.
  op->_ecr = NULL;
}

void ecr_annotation_manager::set_ecr(EcrOpRef *op, ecr_node *node) 
{
  suif_assert(op->_ecr == NULL || node == NULL);
  if (op->_ecr != NULL) {
    op->remove_ecr_ref(_ecr);
  }	
  if (node != NULL) {
    op->add_ecr_ref(node); 
  }
  op->_ecr = node;
}

ecr_node *ecr_annotation_manager::get_ecr(EcrOpRef *op) const
{ 
  return op->_ecr; 
}

ecr_node *ecr_annotation_manager::update_ecr(EcrOpRef *op)
{
  ecr_node *the_ecr = op->get_ecr();
  if (the_ecr == NULL) { return(NULL); }
  ecr_node *top = op->find_top_ecr(the_ecr);
  // temporary to keep alive.
  op->add_ecr_ref(top);
  op->remove_ecr();
  op->set_ecr(top);
  op->remove_ecr_ref(top);
  return(top);
}

void ecr_annotation_manager::remove_ecr(EcrOpRef *op)
{
  ecr_node *the_ecr = op->_ecr;
  if (the_ecr != NULL) {
    op->_ecr = NULL;
    op->remove_ecr_ref(the_ecr);
  }
}

void ecr_annotation_manager::remove_ecrs(EcrRefAnnote *an)
{
  for (int i = 0; i < an->get_op_count(); i++) {
    an->get_op(i)->remove_ecr();
  }
}
void ecr_annotation_manager::update_ecrs(EcrRefAnnote *an) 
{
  for (int i = 0; i < get_op_count(); i++) {
    an->get_op(i)->update_ecr();
  }
}
#endif


static EcrSetTauObject *
retrieve_ecr_set_tau_object(suif_hash_map<ecr_node*, EcrSetObject*> *ecr_id_map,
			    EcrManagerAnnote *emgr,
			    ecr_node *ecr);
static EcrSetLambdaObject *
retrieve_ecr_set_lambda_object(suif_hash_map<ecr_node*, EcrSetObject*> *ecr_id_map,
			       EcrManagerAnnote *emgr,
			       ecr_node *ecr);

static EcrSetObject *retrieve_ecr_set_object(suif_hash_map<ecr_node*, EcrSetObject *> *ecr_id_map,
					     EcrManagerAnnote *emgr,
					     ecr_node *ecr) {
  SuifEnv *s = emgr->get_suif_env();
  ecr = find_top_ecr(ecr);
  suif_hash_map<ecr_node*, EcrSetObject *>::iterator iter = 
    ecr_id_map->find(ecr);
  if (iter != ecr_id_map->end()) {
    return((*iter).second);
  }
  // Build it
  IInteger id = emgr->get_ecr_set_count();
  ecr_type *t = ecr->get_data();

  if (t->is_tau()) {
    EcrSetTauObject *obj = 
      create_ecr_set_tau_object(s, id, NULL, -1, NULL, -1);
    emgr->append_ecr_set(obj);
    // (*ecr_id_map)[ecr] = obj;
    ecr_id_map->enter_value(ecr, obj);

    tau_type *tau = t->get_tau_type();
    if (!tau->is_bottom()) {
      ecr_node *ptr_node = tau->get_tau_pointed_to();
      EcrSetTauObject *ptr_obj =
	retrieve_ecr_set_tau_object(ecr_id_map, emgr, ptr_node);
      
      ecr_node *proc_node = tau->get_lambda_pointed_to();
      EcrSetLambdaObject *proc_obj =
	retrieve_ecr_set_lambda_object(ecr_id_map, emgr, proc_node);
      
      obj->set_points_to(ptr_obj);
      obj->set_points_to_id(ptr_obj->get_id());
      
      obj->set_points_to_procedure(proc_obj);
      obj->set_points_to_procedure_id(proc_obj->get_id());
    }
    return(obj);
  } else {
    EcrSetLambdaObject *obj = 
      create_ecr_set_lambda_object(s, id);
    emgr->append_ecr_set(obj);
    // (*ecr_id_map)[ecr] = obj;
    ecr_id_map->enter_value(ecr, obj);
    
    lambda_type *lambda = t->get_lambda_type();

    for (size_t i = 0; i< lambda->num_inputs(); i++) {
      ecr_node *ptr_node = lambda->get_input_tau_pointed_to(i);
      EcrSetTauObject *ptr_obj =
	retrieve_ecr_set_tau_object(ecr_id_map, emgr, ptr_node);
	
      ecr_node *proc_node = lambda->get_input_lambda_pointed_to(i);
      EcrSetLambdaObject *proc_obj =
	retrieve_ecr_set_lambda_object(ecr_id_map, emgr, proc_node);
	
      EcrAlphaSetObject *alpha = 
	create_ecr_alpha_set_object(s, ptr_obj, proc_obj);
      obj->append_argument(alpha);
    }
      

    {for (size_t i = 0; i< lambda->num_outputs(); i++) {
      ecr_node *ptr_node = lambda->get_output_tau_pointed_to(i);
      EcrSetTauObject *ptr_obj =
	retrieve_ecr_set_tau_object(ecr_id_map, emgr, ptr_node);
      
      ecr_node *proc_node = lambda->get_output_lambda_pointed_to(i);
      EcrSetLambdaObject *proc_obj =
	retrieve_ecr_set_lambda_object(ecr_id_map, emgr, proc_node);

      EcrAlphaSetObject *alpha = 
	create_ecr_alpha_set_object(s, ptr_obj, proc_obj);
      obj->append_argument(alpha);
    }}
    return(obj);
  }
}
					     
static EcrSetTauObject *
retrieve_ecr_set_tau_object(suif_hash_map<ecr_node*, EcrSetObject *> *ecr_id_map,
						EcrManagerAnnote *emgr,
						ecr_node *ecr) {
  EcrSetObject *obj = retrieve_ecr_set_object(ecr_id_map, emgr, ecr);
  return(to<EcrSetTauObject>(obj));
}

static EcrSetLambdaObject *
retrieve_ecr_set_lambda_object(suif_hash_map<ecr_node*, EcrSetObject *> *ecr_id_map,
			       EcrManagerAnnote *emgr,
			       ecr_node *ecr) {
  EcrSetObject *obj = retrieve_ecr_set_object(ecr_id_map, emgr, ecr);
  return(to<EcrSetLambdaObject>(obj));
}

void ecr_annotation_manager::place_annotations(FileSetBlock *fsb) {
  SuifEnv *s = fsb->get_suif_env();
  EcrManagerAnnote *emgr = 
    create_ecr_manager_annote(s, k_ecr_manager_annote);
  fsb->append_annote(emgr);
  
  // Now, walk through the 
  //  stores/loads/variables/procedures/call sites/allocation sites

  // keep a map of ecr object to a number
suif_hash_map<ecr_node*, EcrSetObject *> ecr_id_map;
  
  static LString annotes_set[] = {
    k_ecr_variable,
      k_ecr_procedure_symbol,
      k_ecr_call_targets,
      k_ecr_allocation,
      k_ecr_store_to,
      k_ecr_load_from
      };
#define NUM_ANNOTES (sizeof(annotes_set)/sizeof(LString))

  for (size_t inum = 0; inum< NUM_ANNOTES; inum++) {
    LString k_tmp = annotes_set[inum];
    ecr_map *emap = get_ecr_map(k_tmp);
    if (emap != NULL) {
      for (ecr_map::iterator iter = emap->begin(); iter != emap->end();
	   iter++) {
	AnnotableObject *obj = (*iter).first;
	ecr_node *ecr = (*iter).second;
	EcrSetTauObject *ecr_obj = 
	  retrieve_ecr_set_tau_object(&ecr_id_map,
				      emgr,
				      ecr);
	EcrRefAnnote *an = create_ecr_ref_annote(s, k_tmp,
						 ecr_obj, ecr_obj->get_id());
	obj->append_annote(an);
	ecr_obj->append_ecr_ref(an);
      }
    }
  }
}

