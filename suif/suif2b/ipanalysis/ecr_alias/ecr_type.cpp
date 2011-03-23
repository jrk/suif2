/*
 * ****************
 * *
 * * class alpha_type
 * *
 * * Just a holder for a tau x lambda pair.
 * *
 * ****************
 */


#include "ecr_type.h"
#include "basicnodes/basic.h"
/*
void ecr_type::set_proc_reachable() {
  _is_proc_reachable = true;
}
*/

alpha_type::alpha_type(ecr_node *tau, ecr_node *lambda) {
  _ref_tau = tau; _ref_lambda = lambda;
}

void alpha_type::print(ion *out) const {
  const ecr_node *t = this->tau()->find_top();
  const ecr_node *l = this->lambda()->find_top();
  if (t->get_data()->is_bottom()
      && !alias_print_bot) {
    out->put_s("bot");
  } else {
    out->printf("T%d", t->owner()->get_nodeset_id(t));
  }
  out->put_s(" x ");
  if (l->get_data()->is_bottom()
      && !alias_print_bot) {
    out->put_s("bot");
  } else {
    out->printf("L%d", l->owner()->get_nodeset_id(t));
  }
    
  /*
    out->printf("T%d x L%d", 
    this->tau()->find_top()->get_id(),
    this->lambda()->find_top()->get_id());
    */
}
  /*
  void make_canonical() {
    ecr_node *tau = this->lambda()->find_top();
    ecr_node *lambda = this->lambda()->find_top();
    // clearly, we need to release the references here.
    _ref_tau = tau;
    _ref_lambda = lambda;
  }
  */

/*
 * ****************
 * *
 * * class tau_type
 * *
 * * Either bottom or a ref(alpha)
 * *
 * ****************
 */

tau_type::tau_type() { 
  _is_bottom = true; _ref_alpha = NULL;
}
tau_type::tau_type(ecr_node *t, ecr_node *l) {
  suif_assert(t->get_data()->is_tau());
  suif_assert(l->get_data()->is_lambda());
  _is_bottom = false;
  _ref_alpha = new alpha_type(t, l);
}

void tau_type::print(ion *out) const {
  if (is_bottom()) {
    out->put_s("bot");
  } else {
    out->put_s("ref(");
    this->alpha()->print(out);
    out->put_s(")");
  }
  out->put_s("{");
  unsigned i;
  for (i = 0; i< num_targets(); i++) {
    VariableSymbol *var = get_target(i);
    // DLH mods
    //    char *cn = var->parent()->chain_name();
    //    if (cn != NULL) {
    //      out->put_s(cn);
    //      out->put_s(".");
    //    }
    //    out->put_s(var->name());
    /// @@@ used to be a print_full
    //    var->print_full(out);
    out->printf("%s", var->get_name().c_str());
    //    var->print(((file_ion *)out)->fp());
    if (i+1 != num_targets()) {
      out->put_s(",");
    }
  }
  out->put_s("}");
  
}

void tau_type::add_target(VariableSymbol *var) {
  //unsigned i;
  //  for (i = 0; i< num_targets(); i++) {
  //    if (get_target(i) == var) { return; }
  //  }
  _targets.push_front(var);
  if (is_bottom()) { _is_bottom = false; }
}

// move all from second to first
void tau_type::move_targets(tau_type *e) {
  // move from e to this,
  // then duplicate this
  while (e->num_targets() != 0) {
    VariableSymbol *var = (*(e->_targets.begin()));
    e->_targets.pop_front();
    this->add_target(var);
  }
  /*
  unsigned i;
  for (i = 0; i < this->num_targets(); i++) {
    VariableSymbol *var = this->get_target(i);
    e->_targets.push_front(var);
  }
  */
}

/*
 * ****************
 * *
 * * class lambda_type
 * *
 * * Either bottom or a lam(alpha, alpha...)(alpha, alpha...)
 * *
 * ****************
 */

lambda_type::lambda_type() { 
  _is_bottom = true;
  _has_varargs = false;
  _min_vararg = 0; // invalid until _has_vararg is set.
  //  _is_alloc = false;
}

/*
void lambda_type::set_has_varargs(alpha_type *a) {
  _has_varargs = true;
  _num_static_args = this->_ins.size();
  set_varargs_alpha(a);
}
*/

// We should NEVER MEET an is_alloc with a non-is-alloc.
void lambda_type::set_is_alloc() {
  _alloc_status.meet_with_allocation();
  //  _is_alloc = true;
  _is_bottom = false;
}
void lambda_type::set_is_not_alloc() {
  _alloc_status.meet_with_non_allocation();
  //  _is_alloc = true;
  _is_bottom = false;
}
/*
void lambda_type::set_num_static_args(unsigned num_static_args) {
  assert(has_varargs());
  _num_static_args = num_static_args; 
}
unsigned lambda_type::num_static_args() const { 
  assert(has_varargs());
  return (_num_static_args); 
}
*/
unsigned lambda_type::num_input_args() const { 
  /*  if (has_varargs()) { return(num_static_args()); } */
  return(num_inputs());
}
/*
void lambda_type::set_force_varargs_meet(bool force_varargs_meet) {
  assert(has_varargs());
  _force_varargs_meet = force_varargs_meet;
}
bool lambda_type::force_varargs_meet() const { 
  assert(has_varargs());
  return (_force_varargs_meet);
}
*/
const alpha_type *lambda_type::varargs_alpha() const {
  /*  assert(has_varargs()); */
  return(_varargs_alpha);
}
void lambda_type::set_varargs_alpha(alpha_type *a) { 
  /*  assert(has_varargs()); */
  if (is_bottom()) { _is_bottom = false; }
  _varargs_alpha = a;
}

void lambda_type::set_varargs(ecr_node *tau, ecr_node *lambda) { 
  /*  assert(has_varargs()); */
  if (is_bottom()) { _is_bottom = false; }
  _varargs_alpha = new alpha_type(tau, lambda);
}



void lambda_type::set_input_alpha(unsigned num, alpha_type *a) { 
  if (is_bottom()) { _is_bottom = false; }
  while (num >= _ins.size()) {_ins.push_back(0); }
  _ins[num] = a;
}

void lambda_type::set_input(unsigned num, ecr_node *tau, ecr_node *lambda) { 
  if (is_bottom()) { _is_bottom = false; }
  while (num >= _ins.size()) {_ins.push_back(0); }
  _ins[num] =  new alpha_type(tau, lambda);
}
void lambda_type::set_output_alpha(unsigned num, alpha_type *a) { 
  if (is_bottom()) { _is_bottom = false; }
  while (num >= _outs.size()) {_outs.push_back(0); }
  _outs[num] = a;
}

void lambda_type::set_output(unsigned num, ecr_node *tau, ecr_node *lambda) { 
  if (is_bottom()) { _is_bottom = false; }
  while (num >= _outs.size()) {_outs.push_back(0); }
  _outs[num] = new alpha_type(tau, lambda);
}

void lambda_type::print(ion *out) const {
  if (is_bottom()) {
    out->put_s("bot");
  } else {
    out->put_s("lambda(");
    unsigned i;
    for (i= 0; i< _ins.size(); i++) {
      _ins[i]->print(out);
      if (i+1 != _ins.size()) {
	out->put_s(", ");
      }
      /*
      if (this->has_varargs() &&
	  i+1 == num_static_args()) {
	out->put_s(" = [");
      }
      */
    }
    if (this->has_varargs()) {
      out->put_s(" ,... == ");
      out->put_s("(");
      _varargs_alpha->print(out);
      out->put_s(")");
    }

    out->put_s(")(");
    for (i= 0; i< _outs.size(); i++) {
      _outs[i]->print(out);
      if (i+1 != _outs.size()) {
	out->put_s(",");
      }
    }
    out->put_s(")");
  }
  out->put_s("{");
  unsigned i;
  for (i = 0; i< num_targets(); i++) {
    out->put_s(get_target(i)->get_name().c_str());
    if (i+1 != num_targets()) {
      out->put_s(",");
    }
  }
  out->put_s("}");
}

void lambda_type::add_target(ProcedureSymbol *ps) {
  unsigned i;
  for (i = 0; i< num_targets(); i++) {
    if (get_target(i) == ps) { return; }
  }
  _targets.push_front(ps);
  if (is_bottom()) { _is_bottom = false; }
}

// move all from other to this
void lambda_type::move_targets(lambda_type *e) {
  // move from e to this,
  // then duplicate this
  while (e->num_targets() != 0) {
    ProcedureSymbol *ps = (*(e->_targets.begin()));
    e->_targets.pop_front();
    this->add_target(ps);
  }
  /*
  unsigned i;
  for (i = 0; i < this->num_targets(); i++) {
    ProcedureSymbol *ps = this->get_target(i);
    e->_targets.push_front(ps);
  }
  */
}
