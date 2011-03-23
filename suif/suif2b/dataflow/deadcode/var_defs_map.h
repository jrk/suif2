#ifndef VAR_DEFS_MAP_H
#define VAR_DEFS_MAP_H

#include "common/machine_dependent.h"
#include "common/common_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "bit_vector/bit_vector_forwarders.h"
#include "bit_vector/cross_map_forwarders.h"
#include "suif_cfgraph/suif_cfgraph_forwarders.h"
#include "suif_cfgraph/cfgraph_forwarders.h"

class VarDefsMap {
  typedef size_t StatementId;
  typedef size_t VariableId;

  typedef list<StatementId> StatementIdList;
  typedef suif_vector<StatementIdList *> var_def_vector;


  var_def_vector *_var_defs;          
  
  CrossMap<VariableSymbol*> *_var_map;
  CrossMap<CFGraphNode*> *_st_map;

  // these are 
  //    StoreVariableStatement
  //    StoreStatement
  //    Eval of a CallExpression

  BitVector *_external_vars; // vars that may be modified by a call or STORE.
  int _ref_count;
  
  // Given a load use, the store

 public:
  VarDefsMap();
  ~VarDefsMap();

  // Reference counting:
  // after removing a reference, delete if true.  i.e.
  //   if (map->remove_ref()) delete map;
  void add_ref(); // reference
  bool remove_ref(); 

  //  Statement *get_statement(StatementId) const; // 
  CFGraphNode *get_statement(StatementId) const; // 
  VariableSymbol *get_variable(VariableId) const;
  void mark_external_var(VariableId);

  //  StatementId retrieve_statement_id(Statement *); // create if not here
  StatementId retrieve_statement_id(CFGraphNode *); // create if not here
  VariableId retrieve_variable_id(VariableSymbol *); 
  //statement_id lookup_statement_id(SGraphNode, Statement *) const; // create if not here
  //  StatementId lookup_statement_id(Statement *) const; // create if not here
  StatementId lookup_statement_id(CFGraphNode *) const; // create if not here
  VariableId lookup_variable_id(VariableSymbol *) const; 
  
  void add_var_def(VariableId, StatementId); // mark them
  
  // Might return NULL
  StatementIdList *get_var_defs(VariableId) const;
  //  SGraphNodeList *get_var_node_defs(variable_id) const; // get the statement
  //  ids defined

  BitVector *get_external_var_set() const;
};


//extern void init_var_defs_map(VarDefsMap *var_map, ProcedureDefinition *pd);
extern void init_var_defs_map(VarDefsMap *var_map,
			      CFGraphQuery *q,
			      //			      CFGraphAnnote *cfg,
			      ProcedureDefinition *pd);

  
#endif /* VAR_DEFS_MAP_H */
