
#include "var_prop.h"
#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "sgraph/sgraph_iter.h"
#include "sgraph/sgraph.h"
#include "super_graph/super_graph.h"
#include "suif_cfgraph/suif_cfgraph_query.h"
#include "suifnodes/suif.h"
#include "utils/expression_utils.h"
#include "utils/symbol_utils.h"
#include "basicnodes/basic_constants.h"
#include "bit_vector/cross_map.h"
#include "suifnodes/suif.h"
#include "suifkernel/print_subsystem.h"
#include "basicnodes/basic_constants.h"

bool CopyPropVectorValue::is_safe_var(VariableSymbol *var) {
  if (var->get_definition()) return false;
  if (var->get_is_address_taken()) return false;
  SuifObject *obj = var->get_symbol_table()->get_parent();
  if ((to<QualifiedType>(var->get_type()))->has_qualification_member(k_volatile))
    return(false);

  if (is_kind_of<ScopeStatement>(obj) ||
      is_kind_of<ProcedureDefinition>(obj))
    return(true);
  return(false);
}


CopyPropValue::CopyPropValue() : _type(CopyTop), _var(0), _value(0)
{}

CopyPropValue::CopyPropValue(VariableSymbol *var, IInteger value) : 
  _type(CopyVar), _var(var), _value(value)
{}

CopyPropValue::CopyPropValue(IInteger value) : 
  _type(CopyInt), _var(0), _value(value)
{}

CopyPropValue::CopyPropValue(const CopyPropValue &other) :
  _type(other._type),
  _var(other._var),
  _value(other._value)
{}

CopyPropValue &CopyPropValue::operator=(const CopyPropValue &other)
{
  _type = other._type;
  _var = other._var;
  _value = other._value;
  return(*this);
}

bool CopyPropValue::lub_meet(const CopyPropValue &other)
{
  if (_type == CopyBottom)
    return(false);
  if (other._type == CopyTop)
    return(false);
  if (_type == CopyTop) {
    (*this) = other;
    return(true);
  }
  if (_type == other._type
      && _var == other._var
      && _value == other._value)
    return(false);
  set_bottom();
  return(true);
}

bool CopyPropValue::add(const CopyPropValue &other)
{
  if (_type == CopyBottom)
    return(false);
  if (other._type == CopyTop)
    return(false);
  if (_type == CopyTop) {
    return(false);
  }
  
  if (_type == CopyVar && other._type == CopyVar) {
    set_bottom();
    return(true);
  }
  
  if (_type == CopyVar) {
    if (other._value == 0)
      return(false);
    _value += other._value;
    return(true);
  }

  if (other._type == CopyVar) {
    _type = other._type;
    _value += other._value;
    _var = other._var;
    return(true);
  }

  if (other._value == 0)
    return(false);
  _value += other._value;
  return(true);
}

bool CopyPropValue::restrict_type(DataType *t)
{
  if (_type == CopyBottom)
    return(false);
  if (_type == CopyTop)
    return(false);
  IInteger val = restrict_int_to_data_type(t, _value);
  bool changed = val != _value;
  _value = val;
  return(changed);
}

bool CopyPropValue::multiply(const CopyPropValue &other)
{
  if (_type == CopyBottom)
    return(false);
  if (other._type == CopyTop)
    return(false);
  if (_type == CopyTop) {
    return(false);
  }
  
  if (_type == CopyVar && other._type == CopyVar) {
    set_bottom();
    return(true);
  }
  
  if (_type == CopyVar) {
    if (other._value == 0) {
      (*this) = CopyPropValue(0);
      return(true);
    }
    if (other._value == 1) {
      return(false);
    }
    set_bottom();
    return(true);
  }

  if (other._type == CopyVar) {
    if (_value == 0) {
      return(false);
    }
    if (_value == 1) {
      (*this) = other;
      return(true);
    }
    set_bottom();
    return(true);
  }

  // Constants

  if (_value == 0)
    return(false);
  if (other._value == 1)
    return(false);
  _value *= other._value;
  return(true);
}

bool CopyPropValue::subtract(const CopyPropValue &other)
{
  if (_type == CopyBottom)
    return(false);
  if (other._type == CopyTop)
    return(false);
  if (_type == CopyTop) {
    (*this) = other;
    return(true);
  }
  
  // They are both: Var/Int 
  if (_type == CopyVar && other._type == CopyVar) {
    if (_var == other._var) {
      _type = CopyInt;
      _var = 0;
      _value -= other._value;
      return(true);
    }
    set_bottom();
    return(true);
  }
  
  if (_type == CopyVar) {
    if (other._value == 0)
      return(false);
    _value -= other._value;
    return(true);
  }

  if (other._type == CopyVar) {
    // Not affine... negative var doesn't count.
    set_bottom();
    return(true);
  }

  if (other._value == 0)
    return(false);
  _value -= other._value;
  return(true);
}

bool CopyPropValue::negate()
{
  CopyPropValue cval(0);
  bool changed = cval.subtract(*this);
  (*this) = cval;
  return(changed);
}

bool CopyPropValue::is_bottom() const { return _type == CopyBottom; }
bool CopyPropValue::is_top() const { return _type == CopyTop; }
bool CopyPropValue::is_var() const { return _type == CopyVar; }
bool CopyPropValue::is_int() const { return _type == CopyInt; }

VariableSymbol *CopyPropValue::get_var() const {
  suif_assert(is_var() || is_int());
  return(_var);
}
IInteger CopyPropValue::get_int() const {
  suif_assert(is_var() || is_int());
  return(_value);
}

void CopyPropValue::set_bottom() { 
  _type = CopyBottom; 
  _var = 0;
  _value = 0;
}

Expression *CopyPropValue::build_expression(DataType *result) const {
  suif_assert_message(!is_bottom() && !is_top(),
		      ("Invalid value for expression conversion"));
  SuifEnv *s = result->get_suif_env();
  Expression *e = NULL;
  if (is_var()) {
    e = create_var_use(_var);
    if (_value != 0) {
      e = build_dyadic_expression(k_add, e, create_int_constant(s, _value));
    }
  } else {
    e = create_int_constant(s, _value);
  }    
  if (e->get_result_type() != result) {
    e = build_cast(e, result);
  }
  return(e);
}

void CopyPropValue::print(ion *the_ion) const {
  switch(_type) {
  case CopyTop:
    the_ion->put_s("top");
    break;
  case CopyBottom:
    the_ion->put_s("bottom");
    break;
  case CopyVar:
    the_ion->put_s(_var->get_name().c_str());
    if (_value != 0) {
      the_ion->put_c('+');
      the_ion->put_s(_value.to_String().c_str());
    }
    break;
  case CopyInt:
    the_ion->put_s(_value.to_String().c_str());
    break;
  }
}





CopyPropVectorValue::CopyPropVectorValue(CrossMap<VariableSymbol*> *var_map) :
  _var_map(var_map),
  //  _is_top(true),
  _values(new value_map)
  //  _store_values(new store_map)
{
}

CopyPropVectorValue::~CopyPropVectorValue() {
  delete _values;
  //  delete _store_values;
}


bool CopyPropVectorValue::lub_meet(const SDValue &src) {
  const CopyPropVectorValue &vsrc = (const CopyPropVectorValue &)src;
  assert(_var_map == vsrc._var_map);
  bool changed = false;
  size_t size = _values->size();
  if (vsrc._values->size() > size) {
    size = vsrc._values->size();
  }
  list<VariableSymbol*> dead_vars;
  for (size_t i = 0 ; i < size; i++) {
    // More in the other, NULL is TOP. Meet will skip these
    if ( i >= vsrc._values->size() )
      break;
    if ( i >= _values->size() ) {
      CopyPropValue val = (*vsrc._values)[i];
      if (!val.is_top())
	changed = true;
      _values->push_back((*vsrc._values)[i]);
    } else {
      CopyPropValue val = (*_values)[i];
      if ((*_values)[i].lub_meet((*vsrc._values)[i]))
	  changed = true;
    }
  }
  return(changed);
}

/*
bool CopyPropVectorValue::eq(const SDValue &src) const {
  const CopyPropVectorValue &vsrc = (const CopyPropVectorValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  return(*_reach_set == (*vsrc._reach_set));
}
// lt_eq if "src" is a subset of "this"
bool CopyPropVectorValue::lt_eq(const SDValue &src) const {
  const CopyPropVectorValue &vsrc = (const CopyPropVectorValue &)src;
  assert(_var_map == vsrc._var_map); // same lattice
  BitVector diff(*_reach_set);
  return(!diff.do_or_with_test(*vsrc._reach_set));
}
*/
bool CopyPropVectorValue::eq(const SDValue &src) const {
  suif_assert_message(false, ("Bogus call to eq()"));
  return(false);
}
bool CopyPropVectorValue::lt_eq(const SDValue &src) const {
  suif_assert_message(false, ("Bogus call to lt_eq()"));
  return(false);
}

SDValue *CopyPropVectorValue::clone() const {
  CopyPropVectorValue *new_val = new CopyPropVectorValue(_var_map);
  (*new_val->_values) = (*_values);
  return(new_val);
}

SDValue &CopyPropVectorValue::assign(const SDValue &src) {
  const CopyPropVectorValue &other = (const CopyPropVectorValue &)src;
  //if (_var_map->remove_ref()) delete _var_map;
  _var_map = other._var_map;
  //_var_map->add_ref();
  //  _is_top = other._is_top;
  (*_values) = (*other._values);
  return(*this);
}

void CopyPropVectorValue::print(ion *the_ion) const {
  for (size_t i = 0; i < _values->size(); i++) {
    CopyPropValue val = (*_values)[i];

    if (val.is_top()) continue;
    the_ion->put_s(_var_map->get_node(i)->get_name().c_str());
    the_ion->put_s(" = ");
    val.print(the_ion);
    the_ion->put_s("; ");
  }
}

/*
bool CopyPropVectorValue::add_node(SGraphNode node) {
  if (_reach_set->get_bit(node)) return(false);
  _reach_set->set_bit(node, true);
  return(true);
}
bool CopyPropVectorValue::is_node_member(SGraphNode node) const {
  return(_reach_set->get_bit(node));
}
bool CopyPropVectorValue::remove_node(SGraphNode node) {
  if (!_reach_set->get_bit(node)) return(false);
  _reach_set->set_bit(node, false);
  return(true);
}

bool CopyPropVectorValue::remove_nodes(SGraphNodeList *nodes) {
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

const BitVector *CopyPropVectorValue::get_reaching_defs_set() const {
  return(_reach_set);
}
*/

CopyPropValue CopyPropVectorValue::get_variable_value(VariableSymbol *var) const {
  CopyPropValue top;
  if (!_var_map->is_member(var)) {
    return(top);
  }
  size_t id = _var_map->lookup_id(var);
  if (id >= _values->size())
    return(top);
  return((*_values)[id]);
}

void CopyPropVectorValue::set_variable_value(VariableSymbol *var, const CopyPropValue &val) {
  CopyPropValue top;
  if (!_var_map->is_member(var)) {
    if (val.is_top()) return;
  }
  size_t id = _var_map->retrieve_id(var);
  if (id >= _values->size()) {
    if (val.is_top()) return;
    while (id >= _values->size()) {
      _values->push_back(top);
    }
  }
  (*_values)[id] = val;
}

bool CopyPropVectorValue::kill_variable(VariableSymbol *var) {
  bool changed = false;
  CopyPropValue bottom;
  bottom.set_bottom();

  for (size_t id = 0; id < _values->size(); id++) {
    CopyPropValue val = (*_values)[id];
    if (val.is_var() && val.get_var() == var) {
      (*_values)[id].set_bottom();
      changed = true;
    }
  }
  return(changed);
}

CopyPropSolver::CopyPropSolver(CrossMap<VariableSymbol*> *var_map,
			       SuperGraph *graph,
			       CFGraphQuery *q,
			       bool verbose) :
  FSolver(graph),
  _var_map(var_map),
  _q(q),
  _verbose(verbose)
{
  
}

CopyPropSolver::~CopyPropSolver() {}

CopyPropValue get_cvalue(Expression *expr,
			 const CopyPropVectorValue &env) {
  IInteger val = get_expression_constant(expr);
  CopyPropValue bot;
  bot.set_bottom();
  
  if (!val.is_undetermined()) {
    return(CopyPropValue(val));
  }
  if (is_kind_of<LoadVariableExpression>(expr)) {
    LoadVariableExpression *load = to<LoadVariableExpression>(expr);
    VariableSymbol *source_var = load->get_source();
    if (!CopyPropVectorValue::is_safe_var(source_var)) {
      return(bot);
    }
    CopyPropValue cval = env.get_variable_value(source_var);
    if (cval.is_bottom())
      return(CopyPropValue(source_var, 0));
    return(cval);
  }
  if (is_kind_of<UnaryExpression>(expr)) {
    UnaryExpression *u_expr = to<UnaryExpression>(expr);
    
    CopyPropValue cval = get_cvalue(u_expr->get_source(), env);
    if (cval.is_bottom())
      return(bot);
    if (u_expr->get_opcode() == k_convert) {
      cval.restrict_type(u_expr->get_result_type());
      return(cval);
    }
    if (u_expr->get_opcode() == k_negate) {
      cval.negate();
      cval.restrict_type(u_expr->get_result_type());
      return(cval);
    }
    return(bot);
    
  }
  if (is_kind_of<BinaryExpression>(expr)) {
    BinaryExpression *bin_expr = to<BinaryExpression>(expr);

    CopyPropValue cval1 = get_cvalue(bin_expr->get_source1(), env);
    if (cval1.is_bottom())
      return(bot);

    CopyPropValue cval2 = get_cvalue(bin_expr->get_source2(), env);
    if (cval2.is_bottom())
      return(bot);

    if (bin_expr->get_opcode() == k_add) {
      cval1.add(cval2);
      cval1.restrict_type(bin_expr->get_result_type());
      return(cval1);
    }
    if (bin_expr->get_opcode() == k_subtract) {
      cval1.subtract(cval2);
      cval1.restrict_type(bin_expr->get_result_type());
      return(cval1);
    }
    if (bin_expr->get_opcode() == k_multiply) {
      cval1.multiply(cval2);
      cval1.restrict_type(bin_expr->get_result_type());
      return(cval1);
    }
    return(bot);
  }
  return(bot);
}

bool CopyPropSolver::
apply_transfer(SGraphNode node) {
  // Find the node associated with the input
  // check it for:
  // StoreVariableStatement
  CFGraphNode *cnode = _q->get_node(node);

  CopyPropVectorValue val(_var_map);
  if (!is_in_val_top(node)) {
    val.assign(get_in_val(node));
  }

  if (_q->is_executable(cnode)) {
    ExecutionObject *eo = _q->get_executable(cnode);
    
    // Normally I would apply a visitor here to the node
    // However, this is so simple, I'm not going to bother.
    if (is_kind_of<StoreVariableStatement>(eo)) {
      if (_verbose) {
	printf("evaluating: %s\n",
		eo->get_suif_env()->get_print_subsystem()
		->print_to_string("cprint", eo).c_str());
      }
      StoreVariableStatement *store = to<StoreVariableStatement>(eo);
      VariableSymbol *var = store->get_destination();
      if (CopyPropVectorValue::is_safe_var(var)) {
	CopyPropValue cval = get_cvalue(store->get_value(), val);
	// set the var to the value
	val.set_variable_value(var, cval);
      }
    }
    if (is_kind_of<Statement>(eo)) {
      Statement *st = to<Statement>(eo);
      for (Iter<VariableSymbol*> dest_iter = 
	     st->get_destination_var_iterator(); dest_iter.is_valid();
	   dest_iter.next()) {
	VariableSymbol *dest = dest_iter.current();
	val.kill_variable(dest);
      }
    }
  } else {
    // no effect
  }
  
  return (lub_meet_out_val(node, val));
}
    
const CopyPropVectorValue *CopyPropSolver::get_copy_prop_out_value(SGraphNode node) const {
  if (is_out_val_top(node)) return NULL;
  // Copy it??
  const SDValue &v = get_out_val(node);
  return((const CopyPropVectorValue *)&v);
}
const CopyPropVectorValue *CopyPropSolver::get_copy_prop_in_value(SGraphNode node) const {
  if (is_in_val_top(node)) return NULL;
  // Copy it??
  const SDValue &v = get_in_val(node);
  return((const CopyPropVectorValue *)&v);
}
  
