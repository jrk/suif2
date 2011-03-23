#ifndef ECR_TYPE_H
#define ECR_TYPE_H

/*
 * (c) 1997 Stanford University
 *
 * code for the suif compiler
 * Implementation of the type system for steensgaard
 * type inference.
 *  Includes vararg support.
 *  Plans to include UNIQUE object support.
 * Initial Implementation for SUIF:
 *     David Heine
 */

#include "ecr_alias_forwarders.h"

#include "fast_union.h"
#include "basicnodes/basic_forwarders.h"

class ecr_type;
class tau_type;
class lambda_type;
class alpha_type;

// Uses the following globals:
extern bool alias_print_bot;
extern bool alias_verbose;


typedef union_find_node<ecr_type *> ecr_node;
typedef union_find_owner<ecr_type *> ecr_owner;


// typedef slist_tos<ecr_node *> ecr_node_tos;

// 
// This the info held in an equivalence class representative
// basically, each has a pending list of other ecrs (not necessarily
// cannonical)
//

/*
 * ****************
 * *
 * * class ecr_type
 * *
 * * The type classes tau and lambda inherit from this
 * * This allows for polymorphism that looks similar
 * * To the steensgaard paper.
 * *
 * ****************
 */
  
class ecr_type : public printable { 
  bool _is_proc_reachable;
public:
  ecr_type() { _is_proc_reachable = false; }
  virtual ~ecr_type() {}
  virtual bool is_bottom() const = 0;   // abstract... must be subclassed
  virtual bool is_tau() const { return(false); }
  virtual bool is_lambda() const { return(false); }
  virtual lambda_type *get_lambda_type() const { suif_assert(0); return(0); }
  virtual tau_type *get_tau_type() const { suif_assert(0); return(0); }
  virtual void print(ion *out) const = 0;

  bool is_proc_reachable() const { return _is_proc_reachable; }
  void set_proc_reachable() { _is_proc_reachable = true; }
};

/*
 * ****************
 * *
 * * class alpha_type
 * *
 * * Just a holder for a tau x lambda pair.
 * *
 * ****************
 */


class alpha_type {
  // Not owned here, just referenced
  ecr_node *_ref_tau; 
  ecr_node *_ref_lambda;
  alpha_type() {};
public:
  alpha_type(ecr_node *tau, ecr_node *lambda);

  ecr_node *tau() const { return(_ref_tau); }
  ecr_node *lambda() const { return(_ref_lambda); }
  virtual void print(ion *out) const;

};

/*
 * ****************
 * *
 * * class tau_type
 * *
 * * Either bottom or a ref(alpha)
 * *
 * ****************
 */

class tau_type : public ecr_type {
  bool _is_bottom;
  alpha_type *_ref_alpha; // tau owns this alpha!
  list<VariableSymbol *> _targets; 

  const alpha_type *alpha() const { 
    suif_assert_message(!is_bottom(),
			("Attempt to access a tau or lambda from a BOT"));
    return(_ref_alpha); 
  }

public:
  tau_type();
  tau_type(ecr_node *t, ecr_node *l);
  ~tau_type() { if (!is_bottom()) delete _ref_alpha; }
  bool is_tau() const { return(true); }
  tau_type *get_tau_type() const { return((tau_type *)this); }
  bool is_bottom() const { return(_is_bottom); }

  virtual void print(ion *out) const;

  // Helper functions. Only valid if this is NOT bottom.
  ecr_node *get_tau_pointed_to() const {
    return(alpha()->tau());
  }
  ecr_node *get_lambda_pointed_to() const {
    return(alpha()->lambda());
  }


  void add_target(VariableSymbol *var);

  // copy all targets from arg  to this.
  void move_targets(tau_type *e);
  unsigned num_targets() const {   return(_targets.size()); }
  VariableSymbol *get_target(unsigned i) const { return(_targets[i]); }
};

/*
 * ****************
 * *
 * * class lambda_type
 * *
 * * Either bottom or a lam(alpha, alpha...)(alpha, alpha...)
 * *  Also may be marked "has_varargs"
 * *  Also may be marked "is_alloc"
 * *
 * ****************
 */

class allocation_status {
  enum alloc_stat {UNKNOWN_ALLOC, IS_ALLOC, NOT_ALLOC, MAYBE_ALLOC };
  enum alloc_stat _the_status;
public:
  allocation_status() { _the_status = UNKNOWN_ALLOC; }
  allocation_status(const allocation_status &other) {
    _the_status = other._the_status; }
  void meet_with_allocation() { 
    switch(_the_status) {
    case UNKNOWN_ALLOC: _the_status = IS_ALLOC; break;
    case NOT_ALLOC: _the_status = MAYBE_ALLOC; break;
    default: break;
    }
  }
  void meet_with_non_allocation() { 
    switch(_the_status) {
    case UNKNOWN_ALLOC: _the_status = NOT_ALLOC; break;
    case IS_ALLOC: _the_status = MAYBE_ALLOC; break;
    default: break;
    }
  }
  void meet(const allocation_status &other) {
    switch(_the_status) {
    case UNKNOWN_ALLOC: 
      _the_status = other._the_status; break;
    case IS_ALLOC: 
      if (other._the_status == NOT_ALLOC) {_the_status = MAYBE_ALLOC;} break;
    case NOT_ALLOC: 
      if (other._the_status == IS_ALLOC) {_the_status = MAYBE_ALLOC;} break;
    default: break;
    }
  }
  bool is_alloc() const { return _the_status == IS_ALLOC; }
  bool may_be_alloc() const {
    return (_the_status == IS_ALLOC) || (_the_status == MAYBE_ALLOC); }
};

class lambda_type : public ecr_type {
  bool _is_bottom;
  suif_vector<alpha_type *> _ins;   // lambda owns these
  suif_vector<alpha_type *> _outs;  // lambda owns these
  //  vector<ProcedureSymbol *>   _targets;  // lambda owns these
  list<ProcedureSymbol *>   _targets;  // lambda owns these

  // The rest of this is ALL for varargs support.
  //  bool _has_varargs;
  bool _has_varargs;             // if the varargs is met with an in.
  unsigned _min_vararg;             // first arg that is variable
                                    // varargs
  alpha_type *_varargs_alpha;       // alpha for the var_arg
  //  unsigned _num_static_args;        // number of static args.
  //  bool _force_varargs_meet;      // whether it has seen a va_arg

  //  bool _is_alloc;                // whether this can be an allocator
  //  bool _might_be_alloc;                // whether this can be an allocator
  allocation_status _alloc_status;

  const alpha_type *varargs_alpha() const;
  const alpha_type *input_alpha(unsigned num) const { return(_ins[num]); }
  const alpha_type *output_alpha(unsigned num) const  { return(_outs[num]); }

  void set_varargs_alpha(alpha_type *a);
  void set_input_alpha(unsigned num, alpha_type *a);
  void set_output_alpha(unsigned num, alpha_type *a);

public:
  lambda_type();
  bool is_lambda() const { return(true); }
  lambda_type *get_lambda_type() const { return((lambda_type *)this); }
  bool is_bottom() const { return(_is_bottom); }


  // Here's the vararg, inputs and outputs.

  bool has_varargs() const { return(_has_varargs); }
  void set_has_varargs(bool val) { _has_varargs = val; }

  unsigned min_vararg() const { suif_assert(has_varargs()); return(_min_vararg); }
  void set_min_vararg(unsigned val) { _min_vararg = val; }


  ecr_node *get_varargs_tau_pointed_to() const 
    { return varargs_alpha()->tau(); }
  ecr_node *get_varargs_lambda_pointed_to() const 
    { return varargs_alpha()->lambda(); }
  void set_varargs(ecr_node *tau, ecr_node *lambda);

  

  unsigned num_inputs() const { return(_ins.size()); }
  ecr_node *get_input_tau_pointed_to(unsigned num) const 
    { return input_alpha(num)->tau(); }
  ecr_node *get_input_lambda_pointed_to(unsigned num) const 
    { return input_alpha(num)->lambda(); }
  void set_input(unsigned num, ecr_node *tau, ecr_node *lambda);

  unsigned num_outputs() const { return(_outs.size()); }
  ecr_node *get_output_tau_pointed_to(unsigned num) const 
    { return output_alpha(num)->tau(); }
  ecr_node *get_output_lambda_pointed_to(unsigned num) const 
    { return output_alpha(num)->lambda(); }
  void set_output(unsigned num, ecr_node *tau, ecr_node *lambda);

  // **

  //  void set_has_varargs(alpha_type *a);

  bool is_alloc() const { return(_alloc_status.is_alloc()); }
  bool may_be_alloc() const { return(_alloc_status.may_be_alloc()); }
  void set_is_alloc();
  void set_is_not_alloc();

  allocation_status get_allocation_status() const { 
    return _alloc_status; }

  void set_allocation_status(allocation_status status) { 
    _alloc_status = status; }
  

  //  void set_num_static_args(unsigned num_static_args);
  //  unsigned num_static_args() const;

  //  void set_force_varargs_meet(bool force_varargs_meet);
  //  bool force_varargs_meet() const;

  // This is VALID for varargs or no varargs.
  unsigned num_input_args() const;


  virtual void print(ion *out) const;
  void add_target(ProcedureSymbol *ps);

  // copy all targets from arg  to this.
  void move_targets(lambda_type *e);
  unsigned num_targets() const { return(_targets.size()); }
  ProcedureSymbol *get_target(unsigned i) const { return(_targets[i]); }


};

#undef VariableSymbol
#undef ProcedureSymbol

#endif /* ECR_TYPE_H */

