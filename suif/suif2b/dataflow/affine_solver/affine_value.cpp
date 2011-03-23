/* Copyright Stanford University SUIF Group 1998,1999 */

#include "affine_value.h"
#include "affine.h"
#include "ecr_alias/expression_string.h"
#include "ecr_alias/ecr_annote.h"
#include "ion/ion.h"
#include "common/suif_list.h"
#include "utils/cloning_utils.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"




// These equality checkers should become part of a larger
// framework...
static bool is_equal_affine_expr(AffineExpression *e1,
			       AffineExpression *e2);

static bool is_equal_linear_expr(LinearExpression *e1,
				 LinearExpression *e2);

static bool is_equal_expr(Expression *a, Expression *b) {
  if (is_kind_of<IntConstant>(a)) {
    if (!is_kind_of<IntConstant>(b)) { return(false); };
    return(to<IntConstant>(a)->get_value() ==
	   to<IntConstant>(b)->get_value());
  }
  if (is_kind_of<LoadVariableExpression>(a)) {
    if (!is_kind_of<LoadVariableExpression>(b)) { return(false); };
    return(to<LoadVariableExpression>(a)->get_source() ==
	   to<LoadVariableExpression>(b)->get_source());
  }
  if (is_kind_of<AffineExpression>(a)) {
    if (!is_kind_of<AffineExpression>(b)) { return false; }
    return(is_equal_affine_expr(to<AffineExpression>(a),
				to<AffineExpression>(b)));
  }
  if (is_kind_of<LinearExpression>(a)) {
    if (!is_kind_of<LinearExpression>(b)) { return false; }
    return(is_equal_linear_expr(to<LinearExpression>(a),
				to<LinearExpression>(b)));
  }
  return(a == b);
}


static bool is_equal_affine_expr(AffineExpression *e1,
			       AffineExpression *e2) {
  if (e1 == NULL && e2 == NULL) return true;
  if (e1 == NULL || e2 == NULL) return false;
  Expression *inv1 = e1->get_invariant();
  Expression *inv2 = e2->get_invariant();
  if (e1->get_linear_sum_count() != 
      e2->get_linear_sum_count()) return false;
  if (!is_equal_expr(inv1, inv2)) { return false; }
  for (unsigned i = 0; i < e1->get_linear_sum_count(); i++) {
    LinearExpression *l1 = e1->get_linear_sum(i);
    LinearExpression *l2 = e2->get_linear_sum(i);
    if (!is_equal_linear_expr(l1, l2)) { return(false); }
  }
  return(true);
}

static bool is_equal_linear_expr(LinearExpression *e1,
				 LinearExpression *e2) {
  if (e1 == NULL && e2 == NULL) return true;
  if (e1 == NULL || e2 == NULL) return false;
  if (e1->get_index() != e2->get_index()) return(false);
  if (!is_equal_expr(e1->get_expression(),
		     e2->get_expression())) { 
    return(false); 
  }
  return(true);
}


// This is just a collection of Useful expression checkers.
static VariableSymbol *local_bottom_var;

class AffineE {
  //  static VariableSymbol *_bottom_var;
  
public:
  static bool is_bottom(AffineExpression *e) {
    Expression *inv = e->get_invariant();
    if (is_kind_of<LoadVariableExpression>(inv)) {
      VariableSymbol *var = to<LoadVariableExpression>(inv)->get_source();
      return (var == local_bottom_var);
    }
    return(false);
  }
  static void init_vars(SuifEnv *s) {
    local_bottom_var = create_variable_symbol(s, NULL, false);
  }
  // assign from src to dst do NOT delete the dst.
  // just clear it out and clone the src into it.
  static void assign(AffineExpression *dst,
		     AffineExpression *src) {
    clear_expr(dst);
    dst->set_invariant(deep_suif_clone(src));
    for (unsigned i = 0; i < src->get_linear_sum_count(); i++) {
      dst->append_linear_sum(deep_suif_clone(src->get_linear_sum(0)));
    }
    dst->set_result_type(NULL);
    dst->set_result_type(src->get_result_type());
  }

  static AffineExpression *clone(AffineExpression *src) {
    return(deep_suif_clone(src));
  }
  static void clear_expr(AffineExpression *expr) {
    delete expr->get_invariant();
    expr->set_invariant(NULL);
    while (expr->get_linear_sum_count() != 0) {
      delete expr->remove_linear_sum(0);
    }
  }
  static bool lub_real_meet(AffineExpression *src_dst, 
			    AffineExpression *src2) {
    if (is_equal_affine_expr(src_dst, src2)) return false;
    clear_expr(src_dst);
    src_dst->
      set_invariant(create_load_variable_expression(src_dst->get_suif_env(),
						    NULL,
						    local_bottom_var));
    return(true);
  }

  static bool lub_meet(AffineExpression *src_dst, AffineExpression *src2) {
    if (AffineE::is_bottom(src_dst)) { return(false); }
    if (src2 == NULL) return(false);
    if (AffineE::is_bottom(src2)) {
      AffineE::assign(src_dst, src2);
      return(true);
    }
    return(lub_real_meet(src_dst, src2));
  }
};

void init_affine_value(SuifEnv *s) {
  s->require("utils");
  AffineE::init_vars(s);
}

AffineValue::AffineValue(EcrManagerAnnote *ecr,
			 BuildCStringState *estring, 
			 bool unknown_is_bottom) :
  _ecr(ecr),
  _estring(estring),
  _var_map(new VarMap),
  _ecr_map(new EcrMap),
  _unknown_is_bottom(unknown_is_bottom)
{}

AffineValue::~AffineValue() {
  delete _var_map;
  delete _ecr_map;
}


SDValue *AffineValue::clone() const {
  AffineValue *new_val = new AffineValue(_ecr, _estring, _unknown_is_bottom);
  // why can't we have COPY, etc available?
  for (VarMap::iterator iter = this->_var_map->begin();
       iter != this->_var_map->end(); iter++) {
    //VarMap::pair pair = (*iter);
    new_val->_var_map->enter_value((*iter).first, (*iter).second);
  }
  for (EcrMap::iterator iter = this->_ecr_map->begin();
       iter != this->_ecr_map->end(); iter++) {
    //    EcrMap::pair pair = (*iter);
    new_val->_ecr_map->enter_value((*iter).first, (*iter).second);
  }
  return(new_val);
}

//BuildCStringState print_state;

void AffineValue::print(ion *the_ion) const {
  // env needs to be inited somewhere
  the_ion->printf("Begin: affine_value print\n");
  for (VarMap::iterator iter = this->_var_map->begin();
       iter != this->_var_map->end(); iter++) {
    //    VarMap::pair pair = (*iter);
    VariableSymbol *var = (*iter).first;
    AffineExpression *expr = (*iter).second;
    the_ion->printf("VAR(%s)=",
		    _estring->build_variable_symbol_string(var).c_str());
    the_ion->printf("%s\n",
		    _estring->build_expression_string(expr).c_str());
  }
  for (EcrMap::iterator iter = this->_ecr_map->begin();
       iter != this->_ecr_map->end(); iter++) {
    //    EcrMap::pair pair = (*iter);
    EcrSetObject *ecr = (*iter).first;
    IndirectReferenceList *irlist = (*iter).second;

    the_ion->printf("ECR(%d)=",ecr->get_id().c_int());
    for (Iter<IndirectReferenceStatement*> ir_iter =
	   irlist->get_reference_iterator();
	 ir_iter.is_valid(); ir_iter.next()) {
      //	 ir_iter != irlist->end(); ir_iter++) {
      IndirectReferenceStatement *ir = ir_iter.current();
      the_ion->printf("%s\n",
		      _estring->build_statement_string(ir).c_str());
    }
  }
  the_ion->printf("End: affine_value print\n");
}

void AffineValue::clear() {
  for (VarMap::iterator iter = _var_map->begin();
       iter != _var_map->end(); iter++) {
    delete (*iter).second;
  }
  while(_var_map->begin() != _var_map->end()) {
    VarMap::iterator it = _var_map->begin();
    _var_map->erase(it);
  }
  for (EcrMap::iterator iter = _ecr_map->begin();
       iter != _ecr_map->end(); iter++) {
    delete (*iter).second;
  }
  for (EcrMap::iterator it = _ecr_map->begin();
       it != _ecr_map->end(); 
       it = _ecr_map->begin()) { // strange increment....
    this->_ecr_map->erase(it);
  }
}
SDValue &AffineValue::assign(const SDValue &val) {
  // Clear out the old info.
  clear();

  const AffineValue &old_val = (const AffineValue&)val;

  // duplicate the maps
  //read in the new.
  // why can't we have COPY, etc available?
  for (VarMap::iterator iter = old_val._var_map->begin();
       iter != old_val._var_map->end(); iter++) {
    //    VarMap::pair pair = (*iter);
    _var_map->enter_value((*iter).first, 
			  deep_suif_clone((*iter).second));
  }
  for (EcrMap::iterator iter = old_val._ecr_map->begin();
       iter != old_val._ecr_map->end(); iter++) {
    //    EcrMap::pair pair = (*iter);
    _ecr_map->enter_value((*iter).first, 
			  deep_suif_clone((*iter).second));
  }
  return(*this);
}



bool AffineValue::
lub_meet_indirect_reference(IndirectReferenceList *irlist,
			    IndirectReferenceList *other_irlist) {
  // Walk over the list and check for equality
  // This is currently an N^2 algorithm.
  // to convert it, we need to canonicalize the
  // representation or build a binary tree for easy comparison
  list<int> removal_list;
  int ref_id = 0;
  for (Iter<IndirectReferenceStatement*> ir_iter =
	 irlist->get_reference_iterator();
       ir_iter.is_valid(); ir_iter.next(), ref_id++) {
    IndirectReferenceStatement *ir = ir_iter.current();
    
    // If it does exists in the other list
    AffineExpression *ir_address = ir->get_address();
    for (Iter<IndirectReferenceStatement*> other_ir_iter =
	 other_irlist->get_reference_iterator();
	 other_ir_iter.is_valid(); ir_iter.next()) {
      IndirectReferenceStatement *other_ir = other_ir_iter.current();
      AffineExpression *other_ir_address = other_ir->get_address();
      if (!is_equal_affine_expr(ir_address, other_ir_address)) {
	continue;
      }
      // OK, check for equality or remove the value
      if (!is_equal_affine_expr(ir->get_value(),
				other_ir->get_value())) {
	// push to front so at removal time they don't change "names"
	removal_list.push_front(ref_id); 
	break;
      }
    }
  }
  bool changed = false;
  // now remove the ones on the removal_list
  for (list<int>::iterator iter =
	 removal_list.begin(); iter != removal_list.end(); iter++) {
    int ir_stmt_id = *iter;
    irlist->remove_reference(ir_stmt_id);
    changed = true;
  }
  return(changed);
}


bool AffineValue::lub_meet(const SDValue &src) {
  AffineValue &val = (AffineValue&)(*this);
  const AffineValue &other = (const AffineValue&)src;
  
  // This is the simplest form of meet.
  // it is a pointwise comparison on all of the
  // elements.  For elements NOT in the maps,
  // they are TOP.
  // 
  // For the Ecrs, there are a set of values.
  // Meet the ones that are the same and REMOVE
  // any differences (absence implies WEAK unknown value) 

  // Do it the easy way, walk and meet on both values.
  bool changed = false;
  for (VarMap::iterator iter = val._var_map->begin();
       iter != val._var_map->end(); iter++) {
    VariableSymbol *var = (*iter).first;
    AffineExpression *expr = (*iter).second;
    // check for bottom - No change.
    if (AffineE::is_bottom(expr)) { continue; }

    VarMap::iterator other_iter = other._var_map->find(var);
    if (other_iter == other._var_map->end()) {
      // anything with TOP stays the same.
      continue;
    }
    AffineExpression *other_expr = (*other_iter).second;
    // have to do a real meet.
    if (AffineE::lub_meet(expr, other_expr)) {
      changed = true;
    }
  }
  // Now try the other way. we ONLY have to look for values that
  // are in the other map but not in this one
  for (VarMap::iterator other_iter = other._var_map->begin();
       other_iter != other._var_map->end(); other_iter++) {
    VariableSymbol *var = (*other_iter).first;
    AffineExpression *other_expr = (*other_iter).second;
    VarMap::iterator iter = val._var_map->find(var);
    if (other_iter == other._var_map->end()) {
      //copy it
      AffineExpression *expr = AffineE::clone(other_expr);
      val._var_map->enter_value(var, expr);
      changed = true;
    }
  }


  // Now for the harder part:: the ECR info merge.
  // 
  for (EcrMap::iterator iter = this->_ecr_map->begin();
       iter != this->_ecr_map->end(); iter++) {
    //    EcrMap::pair pair = (*iter);
    EcrSetObject *ecr = (*iter).first;
    IndirectReferenceList *irlist = (*iter).second;

    EcrMap::iterator f = other._ecr_map->find(ecr);
    if (f == other._ecr_map->end()) {
      // In this, not in the other.
      // leave it
      continue;
    }
    IndirectReferenceList *other_irlist = (*iter).second;
    // remove any that are in one but not the other.
    if (lub_meet_indirect_reference(irlist, other_irlist)) {
      changed = true;
    }
  }
  // Now the other way.  If it's in other but not this, 
  // add it.
  for (EcrMap::iterator iter = other._ecr_map->begin();
       iter != other._ecr_map->end(); iter++) {
    //    EcrMap::pair pair = (*iter);
    EcrSetObject *ecr = (*iter).first;
    IndirectReferenceList *irlist = (*iter).second;

    EcrMap::iterator f = this->_ecr_map->find(ecr);
    if (f == other._ecr_map->end()) {
      this->_ecr_map->enter_value(ecr, deep_suif_clone(irlist));
      changed = true;
      continue;
    }
    // otherwise, we've already dealt with it.
  }
  return(changed);
}
  
      
  
bool AffineValue::irregular_widen(const SDValue &src) {
  // @@@ fix
  return(lub_meet(src));
}
bool AffineValue::kleene(const SDValue &src) {
  //@@@ fix
  return(lub_meet(src));
}
bool AffineValue::enter_sub_region(SuperGraphRegion *parent_region,
				   SuperGraphRegion *sub_region,
				   const SDValue &sub_region_src) {
  // @@@ fix
  return(lub_meet(sub_region_src));
}
  
bool AffineValue::exit_sub_region(SuperGraphRegion *sub_region,
				  SuperGraphRegion *parent_region,
				  const SDValue &sub_region_src) {
  //@@@ fix
  return(lub_meet(sub_region_src));
}
  

bool AffineValue::lt_eq(const SDValue &src) const {
  suif_assert_message(0, ("lt_eq NOT defined on this object"));
  return(false);
}
bool AffineValue::eq(const SDValue &src) const {
  // lt_eq(a,b) lt_eq(b,a)
  if (!lt_eq(src)) return false;
  if (!src.lt_eq(*this)) return false;
  return(true);
}


void init_affine_cloning(CloneSubSystem *) {
  
}


