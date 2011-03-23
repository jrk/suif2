#ifndef VAR_PROP_H
#define VAR_PROP_H

#include "common/common_forwarders.h"
#include "suif_cfgraph/suif_cfgraph_forwarders.h"
#include "suif_cfgraph/cfgraph_forwarders.h"
#include "dflowsolver/flat_value.h"
#include "dflowsolver/flat_solver.h"
#include "basicnodes/basic_forwarders.h"
#include "bit_vector/bit_vector_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include "bit_vector/cross_map_forwarders.h"
#include "common/i_integer.h"




/*
 * reaching values.  The set of mods that reach each point
 */
class CopyPropValue {
  typedef enum { CopyTop, CopyVar, CopyInt, CopyBottom } CopyVal;
  CopyVal _type;
  VariableSymbol *_var; // never a volatile.

  //StoreVariableStatement *_loc; // never a volatile.
  IInteger _value;
  
public:
  CopyPropValue();
  CopyPropValue(VariableSymbol *var, IInteger value);
  CopyPropValue(IInteger value);
  CopyPropValue(const CopyPropValue &);
  CopyPropValue &operator=(const CopyPropValue &);

  bool lub_meet(const CopyPropValue &other);
  bool is_bottom() const;
  bool is_top() const;
  bool is_var() const;
  bool is_int() const;
  VariableSymbol *get_var() const;
  IInteger get_int() const;
  Expression *build_expression(DataType *result) const;
  void set_bottom();
  void print(ion *the_ion) const;
  bool add(const CopyPropValue &other);
  bool subtract(const CopyPropValue &other);
  bool negate();
  bool multiply(const CopyPropValue &other);
  bool restrict_type(DataType *type);
};

class CopyPropVectorValue : public FValue {
  CrossMap<VariableSymbol *> *_var_map; // not owned.

  //  bool _is_top; // if top, then all ones. if the BitVector does this
  // we'll ignore it
  typedef suif_vector<CopyPropValue> value_map;
  //typedef suif_vector<StoreVariableStatement*> store_map;
  value_map *_values;
  //  store_map *_store_values;

  //  suif_hash_map<VariableSymbol*, CopyPropValue> _var_map;
  //  BitVector sf_owned *_reach_set; // each value just lists the
  // statements that reach this point.
  
public:
  CopyPropVectorValue(CrossMap<VariableSymbol*> *_var_id);
  CopyPropVectorValue(const CopyPropVectorValue &);
  ~CopyPropVectorValue();
  SDValue &assign(const SDValue &other);
  
  virtual bool lub_meet(const SDValue &src);
  
  
  virtual SDValue *clone() const;
  virtual void print(ion *the_ion) const;
  virtual bool lt_eq_defined() const { return false; }
  virtual bool eq_defined() const { return false; }
  virtual bool lt_eq(const SDValue &val) const;
  virtual bool eq(const SDValue &val) const;

  CopyPropValue get_variable_value(VariableSymbol *var) const;
  void set_variable_value(VariableSymbol *var, const CopyPropValue &val);
  bool kill_variable(VariableSymbol *var);

  // This is just a useful function.
  static bool is_safe_var(VariableSymbol *var);
};

  
/*
 * using the dataflow_solver:
 * set graph, top value
 * create_bottom() and create_top() MUST be over-ridden
 */
class CopyPropSolver : public FSolver {
  CrossMap<VariableSymbol*> *_var_map; // owned.
  CFGraphQuery *_q;
  bool _verbose;
 public:
  CopyPropSolver(CrossMap<VariableSymbol*> *var_map, 
		 SuperGraph *graph,
		 CFGraphQuery *q,
		 bool verbose);
  ~CopyPropSolver();
  const CrossMap<VariableSymbol*> *get_var_map() const { return _var_map; }

  virtual bool apply_transfer(SGraphNode);

  // This is just a type interface
  const CopyPropVectorValue *get_copy_prop_out_value(SGraphNode node) const;
  const CopyPropVectorValue *get_copy_prop_in_value(SGraphNode node) const;

};



  
#endif /* RDEFS_H */
