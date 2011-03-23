#include "var_defs_map.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
#include "common/suif_list.h"
#include "bit_vector/bit_vector.h"
#include "suif_cfgraph/suif_cfgraph_forwarders.h"
#include "suif_cfgraph/suif_cfgraph_query.h"
#include "bit_vector/cross_map.h"


VarDefsMap::VarDefsMap() :
  _var_defs(new var_def_vector()),
  _var_map(new CrossMap<VariableSymbol*>),
  _st_map(new CrossMap<CFGraphNode*>),
  //  _statements(new suif_vector<Statement*>()),
  //  _variables(new suif_vector<VariableSymbol*>()),
  _external_vars(new BitVector()),
  _ref_count(1)
{
  
}

VarDefsMap::~VarDefsMap() {
  delete _st_map;
  delete _var_map;
  delete _var_defs;
  //  delete _statements;
  //  delete _variables;
  delete _external_vars;
  assert(_ref_count == 0);
}


bool VarDefsMap::remove_ref() {
  _ref_count--; 
  assert(_ref_count >= 0);
  return(_ref_count == 0);
}

void VarDefsMap::add_ref() {
  _ref_count++; 
}

VarDefsMap::StatementId VarDefsMap::lookup_statement_id(CFGraphNode *st) const {
  return(_st_map->lookup_id(st));
}
VarDefsMap::VariableId VarDefsMap::lookup_variable_id(VariableSymbol *var) const {
  return(_var_map->lookup_id(var));
}

VarDefsMap::StatementId VarDefsMap::
retrieve_statement_id(CFGraphNode *st) {
  return(_st_map->retrieve_id(st));
}
VarDefsMap::VariableId VarDefsMap::
retrieve_variable_id(VariableSymbol *var) {
  return(_var_map->retrieve_id(var));
}

CFGraphNode *VarDefsMap::get_statement(StatementId id) const {
  return(_st_map->get_node(id));
}
VariableSymbol *VarDefsMap::get_variable(VariableId id) const {
  return(_var_map->get_node(id));
}

// might return NULL
VarDefsMap::StatementIdList *VarDefsMap::
get_var_defs(VariableId id) const {
  if (_var_defs->size() <= id) { return(0); }
  return((*_var_defs)[id]);
}

// might return NULL
void VarDefsMap::
add_var_def(VariableId id, StatementId st_id) {
  // Don't even bother to check the size...
  while (_var_defs->size() <= id) {
    _var_defs->push_back(0);
  }
  StatementIdList *l = (*_var_defs)[id];
  if (!l) {
    l = new StatementIdList();
    (*_var_defs)[id] = l;
  }
  for (StatementIdList::iterator iter = l->begin();
       iter != l->end(); iter++) {
    if ((*iter) == st_id) return;
  }
  l->push_back(st_id);
}

void VarDefsMap::mark_external_var(VariableId id) {
  _external_vars->set_bit(id, true);
}
  

#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
/*
 * This really should be implemented by a visitor pattern
 */

extern void init_var_defs_map(VarDefsMap *var_map, 
			      CFGraphQuery *q,
			      //     CFGraphAnnote *cfg,
			      ProcedureDefinition *pd) {
  //			      ProcedureDefinition *pd) {
  // Again, we really need to define a visitor to do this.
  // However, this is enough for now.

  BitVector store_set;
  BitVector addr_taken_set;
  //CFGraphAnnote *an = q->get_suif_cfgraph_annote();

  for (SNodeIter iter = q->get_node_iterator();
       iter.is_valid(); iter.next()) {
    SGraphNode n = iter.current();
    CFGraphNode *node = q->get_node(n);

    if (q->is_executable(node)) {
      ExecutionObject *eo = q->get_executable(node);
      suif_assert(eo != NULL);

      if (is_kind_of<StoreVariableStatement>(eo)) {
	StoreVariableStatement *store = to<StoreVariableStatement>(eo);
	VarDefsMap::StatementId st_id = var_map->retrieve_statement_id(node);
	VariableSymbol *var = store->get_destination();
	VarDefsMap::VariableId var_id = var_map->retrieve_variable_id(var);
	var_map->add_var_def(var_id, st_id);
	if (var->get_is_address_taken()) {
	  var_map->mark_external_var(var_id);
	  addr_taken_set.set_bit(var_id, true);
	}
      }
      if (is_kind_of<SymbolAddressExpression>(eo)) {
	SymbolAddressExpression *expr = to<SymbolAddressExpression>(eo);
	if (is_kind_of<VariableSymbol>(expr->get_addressed_symbol())) {
	  VariableSymbol *var = to<VariableSymbol>(expr->get_addressed_symbol());
	  VarDefsMap::VariableId var_id = var_map->retrieve_variable_id(var);
	  if (var->get_is_address_taken()) {
	    var_map->mark_external_var(var_id);
	    addr_taken_set.set_bit(var_id, true);
	  }
	}
      }
      if (is_kind_of<StoreStatement>(eo)) {
	//StoreStatement *store = to<StoreStatement>(eo);
	VarDefsMap::StatementId st_id = var_map->retrieve_statement_id(node);
	store_set.set_bit(st_id, true);
      }

    }

    // Iterate for Variable uses.
    for (Iter<ExecutionObject> lviter = object_iterator<ExecutionObject>(pd);
	 lviter.is_valid(); lviter.next()) {
      ExecutionObject *eo = &lviter.current();
      for (Iter<VariableSymbol *> iter = eo->get_source_var_iterator();
	   iter.is_valid(); iter.next()) {
	VariableSymbol *var = iter.current();
	VarDefsMap::VariableId var_id = var_map->retrieve_variable_id(var);
	if (var->get_is_address_taken()) {
	  var_map->mark_external_var(var_id);
	  addr_taken_set.set_bit(var_id, true);
	}
      }
    }
  }

  // a side effect
  // Get this in a BIT.
  // We need to mark that the statement has a side somehow
  /*
    if (is_kind_of<CallExpression>(eo)) {
    // add the statement
    CallExpression *call = to<CallExpression>(eo);
    // find the statement
    Statement *st = find_statement_owner(call);
    var_map->retrieve_statement(store);
    }
  */
    


  // At the end of it all, we SHOULD mark the 
  // variables that are addr taken as defined by the stores
  //
  // and the Volatile vars as defined EVERYWHERE
  for (unsigned st_id = 0; st_id < store_set.num_significant_bits(); st_id++) {
    if (store_set.get_bit(st_id)) {
      for (unsigned var_id = 0; var_id < addr_taken_set.num_significant_bits(); var_id++) {
	if (addr_taken_set.get_bit(var_id)) {
	  var_map->add_var_def(var_id, st_id);
	}
      }
    }
  }

}
				     

