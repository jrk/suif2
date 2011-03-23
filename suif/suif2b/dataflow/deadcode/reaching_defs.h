#ifndef RDEFS_H
#define RDEFS_H

#include "common/common_forwarders.h"
#include "suif_cfgraph/suif_cfgraph_forwarders.h"
#include "dflowsolver/flat_value.h"
#include "dflowsolver/flat_solver.h"
#include "basicnodes/basic_forwarders.h"
#include "bit_vector/bit_vector_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include "var_defs_map.h"




/*
 * reaching values.  The set of mods that reach each point
 */
class ReachingDefsValue : public FValue {
  VarDefsMap *_var_map; // not owned.

  bool _is_top; // if top, then all ones. if the BitVector does this
  // we'll ignore it
  BitVector sf_owned *_reach_set; // each value just lists the
  // statements that reach this point.
  
public:
  ReachingDefsValue(VarDefsMap *var_map);
  ReachingDefsValue(const ReachingDefsValue &);
  ~ReachingDefsValue();
  SDValue &assign(const SDValue &other);

  virtual bool lub_meet(const SDValue &src);


  virtual SDValue *clone() const;
  virtual void print(ion *the_ion) const;
  virtual bool lt_eq_defined() const { return true; }
  virtual bool eq_defined() const { return true; }
  virtual bool lt_eq(const SDValue &val) const;
  virtual bool eq(const SDValue &val) const;
  
  bool add_node(SGraphNode node);
  bool is_node_member(SGraphNode node) const;
  bool remove_node(SGraphNode node);
  bool remove_nodes(VarDefsMap::StatementIdList *nodes);

  const BitVector *get_reaching_defs_set() const; 

};

  
/*
 * using the dataflow_solver:
 * set graph, top value
 * create_bottom() and create_top() MUST be over-ridden
 */
class ReachingDefsSolver : public FSolver {
  VarDefsMap *_var_map; // owned.  References in the vari
  //CFGraphAnnote *_cfg_an;
  CFGraphQuery *_q;
 public:
  ReachingDefsSolver(VarDefsMap *the_var_map, SuperGraph *graph,
		     CFGraphQuery *q);
  ~ReachingDefsSolver();
  const VarDefsMap *get_var_defs_map() { return _var_map; }

  virtual bool apply_transfer(SGraphNode);

  // This is just a type interface
  const ReachingDefsValue *get_reaching_defs_out(SGraphNode node) const;
  const ReachingDefsValue *get_reaching_defs_in(SGraphNode node) const;
  //  ReachingDefsValue *get_bottom() const;

};

//extern void init_var_defs_map(VarDefsMap *var_map, ProcedureDefinition *pd);
//extern void init_var_defs_map(VarDefsMap *var_map, 
//			      CFGraphAnnote *cfg);


  
#endif /* RDEFS_H */
