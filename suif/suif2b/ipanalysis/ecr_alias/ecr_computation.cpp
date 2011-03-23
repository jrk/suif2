

#include "ecr_computation.h"
#include "ecr_annotation_manager.h"


  //
  // get the equivalence class of a suif object.
  // read an ecr from the suif object, then find it's base.
  // If the object does not have an annotation, then
  // it will become a new ref(tau, lambda)
  // Steensgaard calls this "ecr()"
  //

static unsigned u_max(unsigned a, unsigned b) {
  if (a > b) return a;
  return b;
}


ecr_computation::ecr_computation(ecr_owner *ecr_own, bool only_proc_pointers) :
  _ecr_own(ecr_own),
  _only_proc_pointers(only_proc_pointers)
{}
  

// This is VERY common
void ecr_computation::do_assignment(ecr_node *ex, ecr_node *ey) {
  ecr_node *t1 = get_tau_pointed_to(ex);
  ecr_node *l1 = get_lambda_pointed_to(ex);
  
  ecr_node *t2 = get_tau_pointed_to(ey);
  ecr_node *l2 = get_lambda_pointed_to(ey);
  
  cjoin(t1, t2);
  cjoin(l1, l2);
}

ecr_node *ecr_computation::new_empty_tau_node() {
  ecr_node *nt = _ecr_own->new_node(new tau_type());
  return(nt);
}
ecr_node *ecr_computation::new_empty_lambda_node() {
  ecr_node *nt = _ecr_own->new_node(new lambda_type());
  return(nt);
}

// get a new ecr_type
alpha_type *ecr_computation::new_alpha_type() {
  alpha_type *a = 
    new alpha_type(new_empty_tau_node(),
		   new_empty_lambda_node());
  return(a);
}

void ecr_computation::append_parent(ecr_node *e, ecr_node *parent) {
  if (!_only_proc_pointers) return;
  e = _ecr_own->find(e);
  if (e->get_data()->is_proc_reachable()) {
    set_proc_reachable(parent);
  } else {
    e->append_parent(parent);
  }
}

void ecr_computation::append_pending(ecr_node *e, ecr_node *pending) {
  e = _ecr_own->find(e);
  if (_only_proc_pointers) {
    if (!e->get_data()->is_proc_reachable()) {
      e->append_pending(pending);
    } else {
      join(e, pending);
    }
  } else {
    if (e->get_data()->is_bottom()) {
      e->append_pending(pending);
    } else {
      join(e, pending);
    }
  }
}


void ecr_computation::set_type_to_new_tau(ecr_node *e) {
  set_type(e, new_tau_type());
  append_parent(get_tau_pointed_to(e),
		e);
  append_parent(get_lambda_pointed_to(e),
		e);
}
  

void ecr_computation::set_type_to_new_tau(ecr_node *e,
					  ecr_node *tau, ecr_node *lambda) {
  set_type(e, new_tau_type(tau, lambda));
  append_parent(tau, e);
  append_parent(lambda, e);
}

		     
ecr_node *ecr_computation::new_tau_node() {
  ecr_node *e = _ecr_own->new_node(new_tau_type());
  append_parent(get_tau_pointed_to(e),e);
  append_parent(get_lambda_pointed_to(e),e);
  return(e);
}
ecr_node *ecr_computation::new_tau_node(ecr_node *t, ecr_node *l) {
  ecr_node *e = _ecr_own->new_node(new_tau_type(t, l));
  // Add the parent here.
  append_parent(t, e);
  append_parent(l, e);
  return(e);
}


tau_type *ecr_computation::new_tau_type() {
  tau_type *tau = new tau_type(new_empty_tau_node(),
			       new_empty_lambda_node());
  return(tau);
}

tau_type *ecr_computation::new_tau_type(ecr_node *t, ecr_node *l) {
  tau_type *tau = new tau_type(t, l);
  return(tau);
}

void ecr_computation::set_type(ecr_node *ecr, ecr_type *ty) {
  assert(ecr->is_canonical());
  ecr->set_data(ty);

  join_pending(ecr);
}


  // new semantics.  canonicalize and check for already equivalent.
void ecr_computation::cjoin(ecr_node *e1, ecr_node *e2) {
  e1 = _ecr_own->find(e1);
  e2 = _ecr_own->find(e2);
  if (e1 == e2) { return; }

  if (alias_verbose) {
    stdout_ion->printf("cjoin(%d <- %d)\n",e1->owner()->get_nodeset_id(e1), 
		       e2->owner()->get_nodeset_id(e2));
  }
    
  //assert(e1->is_canonical());
  //assert(e2->is_canonical());
  append_pending(e2, e1);
  if (_only_proc_pointers) {
    append_pending(e1, e2);  // need to know the other way as well.
  }
  /*
  if (_only_proc_pointers) {
    if (!e2->get_data()->is_proc_reachable()) {
      append_pending(e2, e1);
    } else {
      this->join(e1, e2);
    }
  } else if (e2->get_data()->is_bottom()) {
    append_pending(e2, e1);
  } else {
    this->join(e1, e2);
  }
  */
}


void ecr_computation::unify_types(ecr_node *ecr_parent, 
				  ecr_type *t1,
				  //  ecr_node *ecr_t2,
				  ecr_type *t2
				  ) {
//void ecr_computation::unify_types(ecr_type *t1, 
//				  ecr_type *t2) {
  //  ecr_type *t1 = ecr_t1->get_data();
  //ecr_type *t2 = ecr_t2->get_data();
  assert(!t1->is_bottom());
  assert(!t2->is_bottom());
  assert(t1->is_tau() == t2->is_tau());
  if (t1->is_tau()) {
    this->unify_tau(t1->get_tau_type(),
		    t2->get_tau_type());
  } else {
    this->unify_lambda(ecr_parent, 
		       t1->get_lambda_type(),
		       t2->get_lambda_type());
  }
}

void ecr_computation::set_proc_reachable(ecr_node *e) {
  // Do nothing if no proc pointers
  if (!_only_proc_pointers) return;

  e = e->find();
  //  if (e->get_data()->is_proc_reachable()) return;
  // If we are switching it.
  // 1) set the value.
  //    join pendings
  // 2) walk over all of the parents and 
  //    set_proc_reachable
  e->get_data()->set_proc_reachable();
  ecr_node *new_e = join_pending(e);

  while (!new_e->is_parents_empty()) {
    ecr_node *parent = new_e->pop_parent();
    set_proc_reachable(parent);
    new_e = new_e->find();
  }
}

// And return the representative of the old node.
ecr_node *ecr_computation::join_pending(ecr_node *e) {
  ecr_node *new_e = e;
  while (!e->is_pending_empty()) {
    ecr_node *join_e = new_e->pop_pending();
    this->join(new_e, join_e);
    new_e = new_e->find();
  }
  return(new_e);
}

  // e1 and e1 are NOT bottom
void ecr_computation::unify_tau(tau_type *e1, tau_type *e2) {
  // targets moves to e1.
  e1->move_targets(e2);

  ecr_node *t1 = e1->get_tau_pointed_to();
  ecr_node *t2 = e2->get_tau_pointed_to();

  ecr_node *l1 = e1->get_lambda_pointed_to();
  ecr_node *l2 = e2->get_lambda_pointed_to();
    
  this->join(t1, t2);
  this->join(l1, l2);
}


/*
 * the next 3 functions probably belong in ecr_computation.
 */
//void ecr_computation::extend_lambda_inputs(lambda_type *lt, unsigned num) {
void ecr_computation::extend_lambda_inputs(ecr_node *ecr_parent,
					   lambda_type *lt, unsigned num) {
  //  lambda_type *lt = ecr_lt->get_data()->get_lambda_type();

  if (num == 0) return;
  unsigned old_num = lt->num_inputs();
  for (unsigned i = old_num; i < num; i++) {
    lt->set_input(i, new_empty_tau_node(), 
		  new_empty_lambda_node());
    append_parent(get_input_tau_pointed_to(lt, i),
		  ecr_parent);
    append_parent(get_input_lambda_pointed_to(lt, i),
		  ecr_parent);

    if (lt->has_varargs() &&
	i >= lt->min_vararg()) {
      ecr_node *vararg_t = get_varargs_tau_pointed_to(lt);
      ecr_node *vararg_l = get_varargs_lambda_pointed_to(lt);
      ecr_node *arg_t = get_input_tau_pointed_to(lt, i);
      ecr_node *arg_l = get_input_lambda_pointed_to(lt, i);
      join(arg_t, vararg_t);
      join(arg_l, vararg_l);
    }
  }
}

void ecr_computation::reset_lambda_min_vararg(ecr_node *ecr_parent,
					      lambda_type *lt, unsigned num) {
  //  lambda_type *lt = ecr_lt->get_data()->get_lambda_type();
  //void ecr_computation::reset_lambda_min_vararg(lambda_type *lt, unsigned num) {
  //  extend_lambda_inputs(lt, num);
  extend_lambda_inputs(ecr_parent, lt, num);
  if (lt->has_varargs()) {
    unsigned old_min_varargs = lt->min_vararg();
    lt->set_min_vararg(num);
    ecr_node *vararg_t = get_varargs_tau_pointed_to(lt);
    ecr_node *vararg_l = get_varargs_lambda_pointed_to(lt);
    for (unsigned i = num; i < old_min_varargs; i++) {
      // join the taus and lambdas
      ecr_node *arg_t = get_input_tau_pointed_to(lt, i);
      ecr_node *arg_l = get_input_lambda_pointed_to(lt, i);
      join(arg_t, vararg_t);
      join(arg_l, vararg_l);
    }
  } else {
    lt->set_has_varargs(true);
    lt->set_varargs(new_empty_tau_node(),
		    new_empty_lambda_node());
    append_parent(get_varargs_tau_pointed_to(lt),
		  ecr_parent);
    append_parent(get_varargs_lambda_pointed_to(lt),
		  ecr_parent);

    lt->set_min_vararg(num);


    ecr_node *vararg_t = get_varargs_tau_pointed_to(lt);
    ecr_node *vararg_l = get_varargs_lambda_pointed_to(lt);
    for (unsigned i = num; i < lt->num_inputs(); i++) {
      // join the taus and lambdas
      ecr_node *arg_t = get_input_tau_pointed_to(lt, i);
      ecr_node *arg_l = get_input_lambda_pointed_to(lt, i);

      join(arg_t, vararg_t);
      join(arg_l, vararg_l);
      

    }
  }
}

void ecr_computation::extend_lambda_outputs(ecr_node *ecr_parent,
					    lambda_type *lt, unsigned num) {
  //    lambda_type *lt = ecr_lt->get_data()->get_lambda_type();
    //void ecr_computation::extend_lambda_outputs(lambda_type *lt, unsigned num) {
  if (num == 0) return;
  unsigned old_num = lt->num_outputs();
  for (unsigned i = old_num; i < num; i++) {
    lt->set_output(i, new_empty_tau_node(), 
		  new_empty_lambda_node());
    append_parent(get_output_tau_pointed_to(lt, i),
		  ecr_parent);
    append_parent(get_output_lambda_pointed_to(lt, i),
		  ecr_parent);
  }
}



// e1 and e1 are NOT bottom
//void ecr_computation::unify_lambda(lambda_type *the_lambda1, lambda_type *the_lambda2) {
void ecr_computation::unify_lambda(ecr_node *ecr_parent, 
				   lambda_type *the_lambda1, lambda_type *the_lambda2) {
  //  lambda_type *the_lambda1 = ecr_l1->get_data()->get_lambda_type();
  //  lambda_type *the_lambda2 = ecr_l2->get_data()->get_lambda_type();
  // copy the procedure list from each to each
  // @@@
  // Move the proc list from 2 to 1.
  the_lambda1->move_targets(the_lambda2);
  // meet the allocation status
  allocation_status status = the_lambda1->get_allocation_status();
  status.meet(the_lambda2->get_allocation_status());
  the_lambda1->set_allocation_status(status);
  the_lambda2->set_allocation_status(status);

  // check the ins, then varargs, then outs
  unsigned i;
  unsigned max_args = u_max(the_lambda1->num_inputs(),
			    the_lambda2->num_inputs());
  //  extend_lambda_inputs(the_lambda1, max_args);
  //  extend_lambda_inputs(the_lambda2, max_args);
  extend_lambda_inputs(ecr_parent, the_lambda1, max_args);
  extend_lambda_inputs(ecr_parent, the_lambda2, max_args);

  for (i = 0; i< max_args; i++) {
    ecr_node *t1, *t2, *l1, *l2;
    t1 = get_input_tau_pointed_to(the_lambda1, i);
    l1 = get_input_lambda_pointed_to(the_lambda1, i);
    t2 = get_input_tau_pointed_to(the_lambda2, i);
    l2 = get_input_lambda_pointed_to(the_lambda2, i);
    
    this->join(t1, t2);
    this->join(l1, l2);
  }

  unsigned max_results = u_max(the_lambda1->num_outputs(),
			       the_lambda2->num_outputs());
  extend_lambda_outputs(ecr_parent, the_lambda1, max_results);
  extend_lambda_outputs(ecr_parent, the_lambda2, max_results);
  //  extend_lambda_outputs(the_lambda1, max_results);
  //  extend_lambda_outputs(the_lambda2, max_results);

  for (i = 0; i< the_lambda1->num_outputs(); i++) {
    ecr_node *t1 = the_lambda1->get_output_tau_pointed_to(i);
    ecr_node *t2 = the_lambda2->get_output_tau_pointed_to(i);

    ecr_node *l1 = the_lambda1->get_output_lambda_pointed_to(i);
    ecr_node *l2 = the_lambda2->get_output_lambda_pointed_to(i);
    
    this->join(t1, t2);
    this->join(l1, l2);
  }
  if (the_lambda1->has_varargs() ||
      the_lambda1->has_varargs()) {
    // it is always true that with varargs, the first vararg <= num args
    unsigned min_vararg = max_args;
    if (the_lambda1->has_varargs()) {
      if (min_vararg > the_lambda1->min_vararg()) {
	min_vararg = the_lambda1->min_vararg();
      }
      if (min_vararg > the_lambda2->min_vararg()) {
	min_vararg = the_lambda2->min_vararg();
      }
      reset_lambda_min_vararg(ecr_parent, the_lambda1, min_vararg);
      reset_lambda_min_vararg(ecr_parent, the_lambda2, min_vararg);
      //      reset_lambda_min_vararg(the_lambda1, min_vararg);
      //      reset_lambda_min_vararg(the_lambda2, min_vararg);
    }
    {
      ecr_node *t1 = the_lambda1->get_varargs_tau_pointed_to();
      ecr_node *t2 = the_lambda2->get_varargs_tau_pointed_to();
      
      ecr_node *l1 = the_lambda1->get_varargs_lambda_pointed_to();
      ecr_node *l2 = the_lambda2->get_varargs_lambda_pointed_to();
      
      this->join(t1, t2);
      this->join(l1, l2);
    }
  }
}

void ecr_computation::join(ecr_node *e1, ecr_node *e2) {
  e1 = _ecr_own->find(e1);
  e2 = _ecr_own->find(e2);
  if (e1 == e2) { return; }

  unsigned node_id = e1->owner()->get_nodeset_id(e1);
  unsigned other_node_id = e2->owner()->get_nodeset_id(e2);

  //assert(e1->is_canonical());
  //assert(e2->is_canonical());
  ecr_type *t1 = e1->get_data();
  ecr_type *t2 = e2->get_data();

  bool proc_reachable = (t1->is_proc_reachable() ||
			    t2->is_proc_reachable());

  ecr_node *e = _ecr_own->fast_union(e1, e2);

  ecr_node *other_e = e1;
  ecr_type *t = e->get_data();
  ecr_type *other_t = t1;

  if (e == e1) {
    other_e = e2;
    other_t = t2;
  } else {
    unsigned tmp = node_id; node_id = other_node_id; other_node_id = tmp;
  }

  if (alias_verbose) {
    stdout_ion->printf("join(%d <- %d)\n", node_id, other_node_id);
  }

  // Now e is the base node, other_e is the one merged.
  // t is the base node type, 
  // other_t is the one to merge into the base node.
  if (other_t->is_bottom()) {
    // Don't need to change the type
    if (t->is_bottom()) {
      // move the pendings
      e->move_pending_from(other_e);
      e->move_parents_from(other_e);
      //other_t->remove_pending();
    } else {
      // join the pendings
      join_pending(e);
      if (proc_reachable) {
	this->set_proc_reachable(e);
	this->set_proc_reachable(other_e);
      }
    }
  } else {
    if (t->is_bottom()) {
      // move the type and join with this pending.
      // @@@ yuk.  memory problem here

      e->set_data(other_t);
      e->move_pending_from(other_e);
      e->move_parents_from(other_e);

      join_pending(e);
    } else {
      //      this->unify_types(t, other_t);
      // The order here is important.
      // The targets on the other_t should be moved to the t.
      this->unify_types(e, t, other_t);
      // Force the joins if
      // they don't agree about the procs_visible
      // should this go here or in the unify_types
      if (proc_reachable) {
	this->set_proc_reachable(e);
	this->set_proc_reachable(other_e);
      }
    }
  }
}
ecr_node *ecr_computation::get_tau_pointed_to(ecr_node *e) { 
  ecr_node *canon_e = _ecr_own->find(e);
  ecr_type *t = canon_e->get_data();
  assert(!t->is_bottom());
  assert(t->is_tau());
  tau_type *et = STATIC_CAST(tau_type *, t);

  // get the tau
  ecr_node *en_tau = et->get_tau_pointed_to();
  en_tau = _ecr_own->find(en_tau);
  assert(en_tau->get_data()->is_tau());
  assert(en_tau->is_canonical());
  return(en_tau);
}

ecr_node *ecr_computation::get_lambda_pointed_to(ecr_node *e) { 
  ecr_node *canon_e = _ecr_own->find(e);
  ecr_type *t = canon_e->get_data();
  assert(!t->is_bottom());
  assert(t->is_tau());
  tau_type *et = STATIC_CAST(tau_type *, t);

  // get the lambda
  ecr_node *en_lambda = et->get_lambda_pointed_to();
  en_lambda = _ecr_own->find(en_lambda);
  assert(en_lambda->get_data()->is_lambda());
  assert(en_lambda->is_canonical());
  return(en_lambda);
}

ecr_node *ecr_computation::get_input_tau_pointed_to(lambda_type *lt,
						    unsigned i) {
  ecr_node *canon_e = _ecr_own->find(lt->get_input_tau_pointed_to(i));
  return(canon_e);
}
ecr_node *ecr_computation::get_input_lambda_pointed_to(lambda_type *lt,
						       unsigned i) {
  ecr_node *canon_e = _ecr_own->find(lt->get_input_lambda_pointed_to(i));
  return(canon_e);
}

ecr_node *ecr_computation::get_output_tau_pointed_to(lambda_type *lt,
						    unsigned i) {
  ecr_node *canon_e = _ecr_own->find(lt->get_output_tau_pointed_to(i));
  return(canon_e);
}
ecr_node *ecr_computation::get_output_lambda_pointed_to(lambda_type *lt,
						       unsigned i) {
  ecr_node *canon_e = _ecr_own->find(lt->get_output_lambda_pointed_to(i));
  return(canon_e);
}

ecr_node *ecr_computation::get_varargs_tau_pointed_to(lambda_type *lt) {
  suif_assert_message(lt->has_varargs(), ("ERROR: varargs not available"));
  ecr_node *canon_e = _ecr_own->find(lt->get_varargs_tau_pointed_to());
  return(canon_e);
}
ecr_node *ecr_computation::get_varargs_lambda_pointed_to(lambda_type *lt) {
  suif_assert_message(lt->has_varargs(), ("ERROR: varargs not available"));
  ecr_node *canon_e = _ecr_own->find(lt->get_varargs_lambda_pointed_to());
  return(canon_e);
}
void ecr_computation::print(ion *iont) {
  _ecr_own->print(iont); 
}
