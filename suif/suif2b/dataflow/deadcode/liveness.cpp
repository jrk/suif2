
#include "liveness.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "sgraph/sgraph_iter.h"
#include "super_graph/super_graph.h"
//#include "suif_cfgraph/suif_cfgraph.h"
#include "suif_cfgraph/suif_cfgraph_query.h"
#include "suifnodes/suif.h"
#include "suifkernel/utilities.h"
#include "reaching_defs.h"
#include "basicnodes/basic_constants.h"
#include "suifkernel/print_subsystem.h"

static bool is_safe_var(VariableSymbol *var) {
  if (var->get_definition()) return false;
  if (var->get_is_address_taken()) return false;
  SuifObject *obj = var->get_symbol_table()->get_parent();
  if (var->get_type()->has_qualification_member(k_volatile))
    return(false);

  if (is_kind_of<ScopeStatement>(obj) ||
      is_kind_of<ProcedureDefinition>(obj))
    return(true);
  return(false);
}



/*
 * strangely enough, we could actually use the
 * ReachingDefsValue here.  It's the same.
 */
LivenessValue::LivenessValue(VarDefsMap *var_map) :
  _var_map(var_map),
  _is_top(false),
  _live_set(new BitVector)
{
  _var_map->add_ref();
}
LivenessValue::~LivenessValue() {
  delete _live_set;
  if(_var_map->remove_ref()) delete _var_map;
}


bool LivenessValue::lub_meet(const SDValue &src) {
  const LivenessValue &vsrc = (const LivenessValue &)src;
  assert(_var_map == vsrc._var_map);
  return(_live_set->do_or_with_test(*vsrc._live_set));
}

bool LivenessValue::eq(const SDValue &src) const {
  const LivenessValue &vsrc = (const LivenessValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  return(*_live_set == (*vsrc._live_set));
}
// lt_eq if "src" is a subset of "this"
bool LivenessValue::lt_eq(const SDValue &src) const {
  const LivenessValue &vsrc = (const LivenessValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  BitVector diff(*_live_set);
  return(!diff.do_or_with_test(*vsrc._live_set));
}

SDValue *LivenessValue::clone() const {
  LivenessValue *new_val = new LivenessValue(_var_map);
  (*new_val->_live_set) = (*_live_set);
  return(new_val);
}

SDValue &LivenessValue::assign(const SDValue &src) {
  const LivenessValue &other = (const LivenessValue &)src;
  if (_var_map->remove_ref()) delete _var_map;
  _var_map = other._var_map;
  _var_map->add_ref();
  _is_top = other._is_top;
  (*_live_set) = (*other._live_set);
  return(*this);
}

void LivenessValue::print(ion *the_ion) const {
  /*
  bool is_first = true;
  for (BitVectorIter bv_iter(_live_set);
       bv_iter.is_valid(); bv_iter.next()) {
    //    VarDefsMap::StatementId stmt_id = bv_iter.current();
    VarDefsMap::VariableId id = bv_iter.current();
    VariableSymbol *var = _var_map->get_variable(id);
    
    //    CFGraphNode *cnode = _var_map->get_variable(id);
    //    AnnotableObject *aobj = cnode->get_owned_object();
    //    if (aobj == NULL)
    //	aobj = cnode->get_base();
    PrintSubSystem *psub = var->get_suif_env()->get_print_subsystem();
    String str = psub->print_to_string("cprint",  var);
    if (!is_first)
      the_ion->printf(", ");
    the_ion->printf("%s, ", str.c_str());
    is_first = false;
  }
  the_ion->printf("\n");
  */
  _live_set->print(the_ion);
}

bool LivenessValue::add_node(SGraphNode node) {
  if (_live_set->get_bit(node)) return(false);
  _live_set->set_bit(node, true);
  return(true);
}
bool LivenessValue::is_node_member(SGraphNode node) const {
  return(_live_set->get_bit(node));
}
bool LivenessValue::remove_node(SGraphNode node) {
  if (!_live_set->get_bit(node)) return(false);
  _live_set->set_bit(node, false);
  return(true);
}

bool LivenessValue::remove_nodes(VarDefsMap::StatementIdList *nodes) {
  bool changed = false;
  for (VarDefsMap::StatementIdList::iterator iter = nodes->begin();
       iter != nodes->end(); iter++) {
    VarDefsMap::StatementId node = (*iter);
    // statement_id is interchangable with INT
    if (remove_node(node))
      changed = true;
  }
  return(changed);
}

const BitVector *LivenessValue::get_live_set() const {
  return(_live_set);
}


LivenessSolver::LivenessSolver(VarDefsMap *the_var_map,
			       ReachingDefsSolver *rdefs,
			       SuperGraph *graph,
			       CFGraphQuery *q) :
  FSolver(graph),
  _var_map(the_var_map),
  _rdefs(rdefs),
  _q(q)
{
  
}

LivenessSolver::~LivenessSolver() {}


/*
 * Liveness:
 * 
 * The transfer function:
 *   If this statement is REACHABLE and is
 *      RETURN, CALL, STORE or StoreVariable with addr_taken
 *      assume this statement is live
 *   If this statement is LIVE,
 *     mark all statements that reach the variables used here as live
 *   otherwise, just pass the value.
 * 
 * Initialization
 *   All cfg-reachable return values are live
 *   All cfg-reachable externally visible definitions are live
 *     i.e. addr taken variables and STOREs to unknown addresses
 *   All cfg-reachable procedure_calls are live
 *     i.e. addr taken variables and STOREs to unknown addresses
 *   
 *
 *  We keep this information as
 *    a BitVector of Live Statements at every point
 *    There is also a var_map that associates variables
 *    with their definitions
 *  Later consider the interprocedural problem:
 */

/*
bool has_call_expression(AnnotableObject *eo) {
  Iter<CallExpression> iter = object_iterator<CallExpression>(eo);
  return(iter.is_valid());
}
*/

bool LivenessSolver::
apply_transfer(SGraphNode node) {
  CFGraphNode *cnode = _q->get_node(node);
  //bool changed = false;
  //  AnnotableObject *eo = cnode->get_base();
      //AnnotableObject *obj = cnode->get_owned_object();
  //  bool fake_execution = _q->is_executable(cnode);
  bool fake_execution = _q->is_fake_executable(cnode);

  LivenessValue val(_var_map);
  if (!is_in_val_top(node)) {
    val.assign(get_in_val(node));
  }

  // Normally I would apply a visitor here to the node
  // However, this is so simple, I'm not going to bother.
  if (_q->is_executable(cnode)) {
    ExecutionObject *eo = _q->get_executable(cnode);
    if (!val.is_node_member(node)) {
      bool statement_is_live = false;
      if (is_kind_of<ReturnStatement>(eo)) {
	statement_is_live = true;
      }
      // Really this should be
      // statement_has_side_effects.
      // i.e volatile use/def, 
      //     store/load
      //     impure call
      if (is_kind_of<StoreStatement>(eo)) {
	statement_is_live = true;
      }
      if (is_kind_of<StoreVariableStatement>(eo)) {
	StoreVariableStatement *store = to<StoreVariableStatement>(eo);
	VariableSymbol *var = store->get_destination();
	if (!is_safe_var(var))
	  statement_is_live = true;
      }
      // branches are alway live here.
      // Of course we could do conditional liveness
      if (is_kind_of<BranchStatement>(eo)) {
	statement_is_live = true;
      }
      if (fake_execution) {
	statement_is_live = true;
      }
      //      if (!statement_is_live && has_call_expression(eo)) {
      if (!statement_is_live && is_kind_of<CallStatement>(eo)) {
	statement_is_live = true;
      }
      if (statement_is_live) {
	val.add_node(node);
      }
    }
  
    // if the statement is live
    if (val.is_node_member(node)) {
      // get set of reaching statements
      // same node number in both graphs!!
      const ReachingDefsValue *reach = _rdefs->get_reaching_defs_in(node);
      if (reach) { // don't bother if nothing reaches.
	// iterate over the variable uses
	// This is Not QUITE right
	const BitVector *reaching_defs = reach->get_reaching_defs_set();
      
	// Find All vars uses in this expression that reach this...
	for (Iter<LoadVariableExpression> iter = 
	       object_iterator<LoadVariableExpression>(eo); 
	     iter.is_valid(); iter.next()) {
	  LoadVariableExpression *lv = &iter.current();
	  VariableSymbol *var = lv->get_source();
	  suif_assert_message(var != NULL, ("Null variable use found"));
	
	  const VarDefsMap *var_map = _rdefs->get_var_defs_map();
	  VarDefsMap::VariableId var_id = var_map->lookup_variable_id(var);
	  VarDefsMap::StatementIdList *kills = var_map->get_var_defs(var_id);

	  if (kills) {
	    // Now add the ones that aren't there now.
	    for (VarDefsMap::StatementIdList::iterator iter = kills->begin();
		 iter != kills->end(); iter++) {
	      VarDefsMap::StatementId st_id = *iter;
	      CFGraphNode *node = var_map->get_statement(st_id);
	      SGraphNode add_node = node->get_id();
	      if (reaching_defs->get_bit(st_id)) {
		if (!val.is_node_member(add_node)) {
		  val.add_node(add_node);
		}
	      }
	    }
	  }
	}
      }
    }
  }
  if (is_out_val_top(node)
      || (!get_out_val(node).eq(val))) {
    set_out_val(node, val);// ignore kills. nothing is defined yet
    return(true);
  }
  return(false);
}

const LivenessValue *LivenessSolver::get_liveness() const {
  SGraphNode node = get_graph()->top_region()->get_exit();
  if (is_out_val_top(node)) return NULL;
  // Copy it??
  const SDValue &v = get_out_val(node);
  return((const LivenessValue *)&v);
}

  
