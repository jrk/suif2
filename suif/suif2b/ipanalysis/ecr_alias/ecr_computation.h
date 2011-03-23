#ifndef ECR_COMPUTATION_H
#define ECR_COMPUTATION_H

#include "ecr_type.h"


class ecr_computation {
  ecr_owner *_ecr_own;
  bool _only_proc_pointers;  // Do only proc analysis.
  			// if a proc is not reachable from the LHS
  			// do not join

private:

  alpha_type *new_alpha_type();

public:
    // Here are the rest of the functions;
  ecr_computation(ecr_owner *ecr_own, bool only_proc_pointers);
  ~ecr_computation() {}

  // create new nodes.
  ecr_node *new_tau_node(ecr_node *t, ecr_node *l);
  ecr_node *new_tau_node();
  void set_type_to_new_tau(ecr_node *t2,
			   ecr_node *tau, ecr_node *lambda); 
  void set_type_to_new_tau(ecr_node *t2);

 private:
  tau_type *new_tau_type();
  tau_type *new_tau_type(ecr_node *t, ecr_node *l);

  ecr_node *new_empty_lambda_node();
  ecr_node *new_empty_tau_node();

  void set_type(ecr_node *ecr, ecr_type *ty);

  void unify_types(ecr_node *ecr_parent, ecr_type *t1, ecr_type *t2);
  void unify_tau(tau_type *e1, tau_type *e2);

  // e1 and e1 are NOT bottom
  void unify_lambda(ecr_node *ecr_parent, lambda_type *e1, lambda_type *e2);

 public:

  void cjoin(ecr_node *e1, ecr_node *e2);

  void do_assignment(ecr_node *ex, ecr_node *ey); // ex = ey

  //
  void set_proc_reachable(ecr_node *e);
  ecr_node *join_pending(ecr_node *e);

  void append_parent(ecr_node *e, ecr_node *parent);
  void append_pending(ecr_node *e, ecr_node *pending);

  void join(ecr_node *e1, ecr_node *e2);

  ecr_node *get_tau_pointed_to(ecr_node *e);
  ecr_node *get_lambda_pointed_to(ecr_node *e);
  

  ecr_node *get_input_tau_pointed_to(lambda_type *lt, unsigned i);
  ecr_node *get_input_lambda_pointed_to(lambda_type *lt, unsigned i);
  ecr_node *get_output_tau_pointed_to(lambda_type *lt, unsigned i);
  ecr_node *get_output_lambda_pointed_to(lambda_type *lt, unsigned i);
  ecr_node *get_varargs_tau_pointed_to(lambda_type *lt);
  ecr_node *get_varargs_lambda_pointed_to(lambda_type *lt);


  /*
   * Darn, I need the ecr_node for this.
   */
  //  void extend_lambda_outputs(lambda_type *lt, unsigned num);
  //  void extend_lambda_inputs(lambda_type *lt, unsigned num);
  //  void reset_lambda_min_vararg(lambda_type *lt, unsigned num);
  void extend_lambda_outputs(ecr_node *parent, lambda_type *lt, unsigned num);
  void extend_lambda_inputs(ecr_node *parent, lambda_type *lt, unsigned num);
  void reset_lambda_min_vararg(ecr_node *parent, lambda_type *lt, unsigned num);

  void print(ion *iont); 
};





#endif /* ECR_COMPUTATION_H */
