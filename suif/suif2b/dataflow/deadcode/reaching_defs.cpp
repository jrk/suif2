
#include "reaching_defs.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "sgraph/sgraph_iter.h"
#include "sgraph/sgraph.h"
#include "super_graph/super_graph.h"
//#include "suif_cfgraph/suif_cfgraph.h"
#include "suif_cfgraph/suif_cfgraph_query.h"
#include "suifnodes/suif.h"






ReachingDefsValue::ReachingDefsValue(VarDefsMap *var_map) :
  _var_map(var_map),
  _is_top(false),
  _reach_set(new BitVector)
{
  _var_map->add_ref();
}
ReachingDefsValue::~ReachingDefsValue() {
  delete _reach_set;
  if(_var_map->remove_ref()) delete _var_map;
}


bool ReachingDefsValue::lub_meet(const SDValue &src) {
  const ReachingDefsValue &vsrc = (const ReachingDefsValue &)src;
  assert(_var_map == vsrc._var_map);
  return(_reach_set->do_or_with_test(*vsrc._reach_set));
}

bool ReachingDefsValue::eq(const SDValue &src) const {
  const ReachingDefsValue &vsrc = (const ReachingDefsValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  return(*_reach_set == (*vsrc._reach_set));
}
// lt_eq if "src" is a subset of "this"
bool ReachingDefsValue::lt_eq(const SDValue &src) const {
  const ReachingDefsValue &vsrc = (const ReachingDefsValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  BitVector diff(*_reach_set);
  return(!diff.do_or_with_test(*vsrc._reach_set));
}

SDValue *ReachingDefsValue::clone() const {
  ReachingDefsValue *new_val = new ReachingDefsValue(_var_map);
  (*new_val->_reach_set) = (*_reach_set);
  return(new_val);
}

SDValue &ReachingDefsValue::assign(const SDValue &src) {
  const ReachingDefsValue &other = (const ReachingDefsValue &)src;
  if (_var_map->remove_ref()) delete _var_map;
  _var_map = other._var_map;
  _var_map->add_ref();
  _is_top = other._is_top;
  (*_reach_set) = (*other._reach_set);
  return(*this);
}

void ReachingDefsValue::print(ion *the_ion) const {
  _reach_set->print(the_ion);
}

bool ReachingDefsValue::add_node(SGraphNode node) {
  if (_reach_set->get_bit(node)) return(false);
  _reach_set->set_bit(node, true);
  return(true);
}
bool ReachingDefsValue::is_node_member(SGraphNode node) const {
  return(_reach_set->get_bit(node));
}
bool ReachingDefsValue::remove_node(SGraphNode node) {
  if (!_reach_set->get_bit(node)) return(false);
  _reach_set->set_bit(node, false);
  return(true);
}

bool ReachingDefsValue::remove_nodes(SGraphNodeList *nodes) {
  bool changed = false;
  for (VarDefsMap::StatementIdList::iterator iter = nodes->begin();
       iter != nodes->end(); iter++) {
    VarDefsMap::StatementId node = (*iter);
    // This is NOT the cfgraph node number, just the statement
    //Statement *st = _var_map->get_statement(st);
    
    // statement_id is interchangable with INT
    if (remove_node(node))
      changed = true;
  }
  return(changed);
}

const BitVector *ReachingDefsValue::get_reaching_defs_set() const {
  return(_reach_set);
}

ReachingDefsSolver::ReachingDefsSolver(VarDefsMap *the_var_map,
				       SuperGraph *graph,
				       CFGraphQuery *q) :
  FSolver(graph),
  _var_map(the_var_map),
  _q(q)
{
  
}

ReachingDefsSolver::~ReachingDefsSolver() {}

bool ReachingDefsSolver::
apply_transfer(SGraphNode node) {
  // Find the node associated with the input
  // check it for:
  // StoreVariableStatement
  // StoreStatement
  // Case:
  //   StoreVariableStatement
  //     kill all StoreVars of this variable
  //     add this StoreVar
  //   StoreStatement
  //     add this StoreStatement
  //   default:
  //     transfer
  CFGraphNode *cnode = _q->get_node(node);
  //bool changed = false;
  //AnnotableObject *aobj = cnode->get_base();

  ReachingDefsValue val(_var_map);
  if (!is_in_val_top(node)) {
    val.assign(get_in_val(node));
  }

  if (_q->is_executable(cnode)) {
    ExecutionObject *eo = _q->get_executable(cnode);

    // Normally I would apply a visitor here to the node
    // However, this is so simple, I'm not going to bother.

    if (is_kind_of<StoreVariableStatement>(eo)) {
      StoreVariableStatement *store = to<StoreVariableStatement>(eo);
      VariableSymbol *var = store->get_destination();

      VarDefsMap::VariableId var_id = _var_map->lookup_variable_id(var);
      VarDefsMap::StatementIdList *kills = 
	_var_map->get_var_defs(var_id);
    
      /*
	SGraphNodeList l;
	// convert the kills to statements..
	for (VarDefsMap::StatementIdList::iterator it = kills->begin();
	it != kills->end(); it++) {
	VarDefsMap::StatementId st_id = *it;
	Statement *st = _var_map->get_statement(st_id);
	// find the cfgraph node for this statement...
      
	CFGraphNode *cnode = find_cfgraph_node(_cfg_an, st);
	l.push_back(cnode->get_id());
	}
      */

      //    val.remove_nodes(&l); //kills
      val.remove_nodes(kills); //kills
      //    val.add_node(_var_map->lookup_statement_id(store));
      val.add_node(_var_map->lookup_statement_id(cnode));
    } else if (is_kind_of<StoreStatement>(eo)) {
      // add the store statement to the list
      //StoreStatement *store = to<StoreStatement>(eo);
      //    val.add_node(_var_map->lookup_statement_id(store));
      val.add_node(_var_map->lookup_statement_id(cnode));
    } else {
      // just a transfer
    }
  }
  return (lub_meet_out_val(node, val));
}
    
const ReachingDefsValue *ReachingDefsSolver::get_reaching_defs_out(SGraphNode node) const {
  if (is_out_val_top(node)) return NULL;
  // Copy it??
  const SDValue &v = get_out_val(node);
  return((const ReachingDefsValue *)&v);
}

const ReachingDefsValue *ReachingDefsSolver::get_reaching_defs_in(SGraphNode node) const {
  if (is_in_val_top(node)) return NULL;
  // Copy it??
  const SDValue &v = get_in_val(node);
  return((const ReachingDefsValue *)&v);
}
  
