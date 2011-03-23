#ifndef DATAFLOW_LIVENESS_H
#define DATAFLOW_LIVENESS_H

#include "common/common_forwarders.h"
#include "suif_cfgraph/suif_cfgraph_forwarders.h"
#include "dflowsolver/region_solver.h"
#include "dflowsolver/flat_value.h"
#include "basicnodes/basic_forwarders.h"
#include "bit_vector/bit_vector_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include "var_defs_map.h"

#include "reaching_defs.h"



/*
 * reaching values.  The set of mods that reach each point
 */
class LivenessValue : public FValue {
  VarDefsMap *_var_map; // not owned.

  bool _is_top; // if top, then all ones. if the BitVector does this
  // we'll ignore it
  BitVector sf_owned *_live_set; // each value just lists the
  // statements that reach this point.
  
public:
  LivenessValue(VarDefsMap *var_map);
  LivenessValue(const LivenessValue &);
  ~LivenessValue();
  SDValue &assign(const SDValue &other);

  virtual bool lub_meet(const SDValue &src);


  virtual SDValue *clone() const;
  virtual void print(ion *the_ion) const;
  virtual bool lt_eq_defined() const { return true; }
  virtual bool eq_defined() const { return true; }
  virtual bool lt_eq(const SDValue &val) const;
  virtual bool eq(const SDValue &val) const;
  
  bool add_node(SGraphNode node);
  bool get_node(SGraphNode node);
  bool is_node_member(SGraphNode node) const;
  bool remove_node(SGraphNode node);
  bool remove_nodes(VarDefsMap::StatementIdList *nodes);

  const BitVector *get_live_set() const; 

};

  
/*
 * using the dataflow_solver:
 * set graph, top value
 * create_bottom() and create_top() MUST be over-ridden
 */
class LivenessSolver : public FSolver {
  VarDefsMap *_var_map; // owned.  References in the vari
  ReachingDefsSolver *_rdefs; // owned.  References in the vari
  //CFGraphAnnote *_cfg_an;
  CFGraphQuery *_q;
 public:
  LivenessSolver(VarDefsMap *the_var_map, 
		 ReachingDefsSolver *_rdefs,
		 SuperGraph *graph,
		 CFGraphQuery* q);
  ~LivenessSolver();
  

  virtual bool apply_transfer(SGraphNode);

  // This is the end result
  const LivenessValue *get_liveness() const;
  
  //  LivenessValue *get_bottom() const;

};

#endif /* DATAFLOW_LIVENESS_H */
