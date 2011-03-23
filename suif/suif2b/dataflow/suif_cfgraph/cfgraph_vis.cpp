#include "suifnodes/suif.h"
#include "cfgraph_vis.h"
#include "suifnodes/suif_factory.h"
#include "basicnodes/basic_constants.h"
#include "utils/cloning_utils.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "utils/symbol_utils.h"

bool for_statement_is_guarded(ForStatement *the_for) {
  return(the_for->peek_annote("guarded"));
}


// On entry to any handle_ 
// we expect the following:
// Before calling a handle_
// (1) push the node before the handled nodes onto the _prev_stack
// 
// On entering a handle_
// (2) pop the prev_stack()
// 
// On exiting a handle
// (3) push the entry for the handled node onto the _entry_stack
// (4) push the fall_through (if any) for the handled node onto the 
//      _fall_through_stack
//
// After returning from a handle_ call,
// (5) pop the _entry_stack and _exit_stack.
//
// We use a stack to catch errors.  We could just pass around a 
//  single continuation otherwise.
// 
// The only exception to (2) is that the top level 
//   handle_procedure_definition will not have a prev
//
// Also, any branch to a label can be added if the label exists.
//     otherwise, it is stored into a table and deferred.
// That was way too complex:
// Here's the new version:
//
//
/*
 *
 * The basic pattern here:
 *  Build the nodes and edges for
 *  the suif node. 
 * Call register_label for any label statements
 * Call register_branch for any branches
 * Call add_edge for any edges to add
 *  set the (default_entry, default_exit) before returning.
 */

/*
 * forward declarations of the registered functions
 */
// they look like:
// void handle_static_suif_object(SuifObject *SuifObject);
//
// We build prototypes here to make sure that
// all of the static functions are ok.

#define HANDLE_DISPATCH(lower, name) \
static CFGraphBuilderReturn handle_static_ ## lower(CFGraphState *state, ExecutionObject *obj);
#include "defs.macro"
#undef HANDLE_DISPATCH



void cfgraph_pass_init_suif_maps(SuifCFGraphBuilderModule *mod) {
#define HANDLE_DISPATCH(lower, name) \
  mod->add_builder_fn(name ## :: get_class_name(), \
		      handle_static_ ## lower ##);
#include "defs.macro"
#undef HANDLE_DISPATCH
}


/*
 * New usage model:
 *  We are trying to build the cfg in 
 *   On pass plus fix-up.
 *  At each node, do the following:
 *    Save the old entry and exits.
 *  Create any needed Entry and Exit, label nodes for this node.
 *   walk the children
 *  resume the old entry and exits
 *  
 *  At each thing
 */

static CFGraphBuilderReturn
handle_static_execution_object(CFGraphState *cstate,
				   ExecutionObject *obj) 
{
  suif_assert_message(false, ("Could not handle ExecObject %s",
			      obj->getClassName().c_str()));
  return(CFGraphBuilderReturn(NULL, NULL));
}

static CFGraphBuilderReturn
do_simple_statement(CFGraphState *cstate,
		    Statement *the_statement)
{
  // A LEAF
  CFGraphNode *the_entry = cstate->get_entry_node(the_statement, true);
  //CFGraphNode *the_exit = cstate->get_exit_node(the_statement);
  //  cstate->connect_edge(the_entry, the_exit);
  //  return(CFGraphBuilderReturn(the_entry, the_exit));
  return(CFGraphBuilderReturn(the_entry, the_entry));
}

static CFGraphBuilderReturn
handle_static_eval_statement(CFGraphState *cstate,
				 ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}

/*
 *
 * The basic pattern here:
 *  Build the nodes and edges for
 *  the suif node. 
 * Call register_label for any label statements
 * Call register_branch for any branches
 * Call add_edge for any edges to add
 *  set the (default_entry, default_exit) before returning.
 */
static CFGraphBuilderReturn
handle_static_for_statement(CFGraphState *cstate,
				ExecutionObject *obj)
{
  ForStatement *the_for = to<ForStatement>(obj);

  CFGraphNode *the_entry = cstate->get_entry_node(the_for, false);
  CFGraphNode *the_exit = cstate->get_exit_node(the_for);
  //  
  //           Begin
  //             |
  //             v
  //        (1)LB,UB,step assignment
  //             |
  //             v
  // +----- (2) TEST2
  // |           |
  // |           v
  // |     (3) PREPAD
  // |           |
  // |           v
  // |      BODY BEGIN<--+
  // |           |       |
  // |      continue -+  |
  // |           |    |  |
  // |   +--- break:  |  |
  // |   |       |    |  |
  // |   |   BODY END |  |
  // |   |       |    |  |
  // |   |       v    |  |
  // |   | (4) cont:<-+  |
  // |   |       |       |
  // |   |       v       |
  // |   | (5)Increment  |
  // |   |       |       |
  // |   |       v       |
  // |   |   (6)TEST ----+
  // |   |       | 
  // |   |       v 
  // +---+->(7)break:
  //            |
  //            v
  //           END

  // 
  // (1) (5) (6) must be created.
  //    (2) is created if the loop is not guarded
  //        and there is a prepad.
  //        If the loop is not guarded and there is no prepad
  //        (1) -> (6)
  // 
  // (1) has the effect of evaluating lb,ub,step and assigning
  //     fake_lb,fake_ub,fake_step
  // (2)(6) evaluates test (index CMP fake_ub)
  // (3) Created if there is a prepad.
  // (4) No effect
  // (5) index += fake_step
  // (7) No effect
  //
  // Special cases:
  //   Not guarded:  (2) is removed
  //   No prepad:   (3) is removed.  If it is not guarded,
  //                (1)->(6)
  //
  // Special nodes that must build Expression tree
  // representations.

  // need 3 new variables
  VariableSymbol *for_lb_var = 
    cstate->create_variable("for_lb", the_for, 
			    retrieve_qualified_type(the_for->get_lower_bound()->get_result_type()));
  VariableSymbol *for_ub_var = 
    cstate->create_variable("for_ub", the_for, 
			    retrieve_qualified_type(the_for->get_upper_bound()->get_result_type()));
  VariableSymbol *for_step_var = 
    cstate->create_variable("for_step", the_for, 
			    retrieve_qualified_type(the_for->get_step()->get_result_type()));
  VariableSymbol *for_index_var = the_for->get_index();
    /*
    create_variable("for_index", the_for, 
		    the_for->index().Type());
		    */

  SuifEnv *s = cstate->get_suif_env();

  Expression *for_lb_op = deep_suif_clone(the_for->get_lower_bound());

  StoreVariableStatement *assign_lb =
    create_store_variable_statement(s,
				    for_lb_var,
				    for_lb_op);
  

  Expression *for_ub_op = deep_suif_clone(the_for->get_upper_bound());

  StoreVariableStatement *assign_ub =
    create_store_variable_statement(s,
				    for_ub_var,
				    for_ub_op);
  Expression *for_step_op = deep_suif_clone(the_for->get_step());

  StoreVariableStatement *assign_step =
    create_store_variable_statement(s,
				    for_step_var,
				    for_step_op);

  //  Expression *index_exp = create_var_use(the_for->get_index());

  StoreVariableStatement *assign_index =
    create_store_variable_statement(s,
				    for_index_var,
				    create_var_use(for_lb_var));
  Expression *for_test = 
    create_binary_expression(s, unqualify_data_type(for_index_var->get_type()),
			     the_for->get_comparison_opcode(),
			     create_var_use(the_for->get_index()),
			     create_var_use(for_ub_var));

  bool guarded = for_statement_is_guarded(the_for);
  bool has_pre_pad = the_for->get_pre_pad() != 0;
  Expression *for_test2 = NULL;
  if (!guarded && has_pre_pad)
    {
      for_test2 = deep_suif_clone(for_test);
    }

  // Don't we have a builder for this?
  Expression *add_expr = 
    create_binary_expression(s, 
			     unqualify_data_type(for_index_var->get_type()),
			     k_add,
			     create_var_use(the_for->get_index()),
			     create_var_use(for_step_var));
    
  StoreVariableStatement *increment_eval =
    create_store_variable_statement(s, for_index_var, add_expr);

  // attach to the for node by name
  CFGraphNode *assign_lb_node = 
    cstate->add_statement_node(the_for->get_lower_bound(),
			      "for_assign_lb", 
			       true,
			       true,
			       assign_lb);
  CFGraphNode *assign_ub_node =
    cstate->add_statement_node(the_for->get_upper_bound(),
			      "for_assign_ub", 
			       true,
			       true,
			      assign_ub);
  CFGraphNode *assign_step_node =
    cstate->add_statement_node(the_for->get_step(),
			      "for_assign_step", 
			       true,
			       true,
			       assign_step);
  CFGraphNode *assign_index_node =
    cstate->add_statement_node(the_for, 
			      "for_assign_index", 
			       true,
			       false,
			       assign_index);

  cstate->connect_edge(assign_lb_node, assign_ub_node);
  cstate->connect_edge(assign_ub_node, assign_step_node);
  cstate->connect_edge(assign_step_node, assign_index_node);
  
  CFGraphNode *increment_node = 
    cstate->add_statement_node(the_for, 
			       "for_increment", 
			       true,
			       false,
			       increment_eval);

  
  CFGraphNode *test_node = 
    cstate->add_expression_node(the_for, 
			      "for_test",
			      false,
			      for_test);

  CFGraphNode *test2_node = NULL;
  if (for_test2)
    test2_node = 
      cstate->add_expression_node(the_for, 
				  "for_test2",
				  false,
				  for_test2);
    
  // The visit may not get these.
  // I wonder if there is a more general way to get code
  // label symbols with a higher level construct.

  // these labels have NO explicit label location.
  CFGraphNode *continue_node = 
    cstate->create_label_node(the_for, "continue", the_for->get_continue_label());
  CFGraphNode *break_node = 
    cstate->create_label_node(the_for, "break", the_for->get_break_label());

  // build pre_pad
  
  CFGraphNode * assign_entry = assign_lb_node;
  CFGraphNode * assign_exit = assign_index_node;
  CFGraphNode * increment_entry = increment_node;
  CFGraphNode * increment_exit = increment_node;


  cstate->connect_edge(the_entry, assign_entry);
  CFGraphNode *prev = assign_exit; // This is just for ease of building

  if (test2_node) 
    {
      cstate->connect_edge(assign_exit, test2_node);
      cstate->connect_edge(test2_node, break_node);
      prev = test2_node;
    }

  CFGraphBuilderReturn bdy = 
    cstate->get_builder()->build(cstate, the_for->get_body());


  if (has_pre_pad) {
    CFGraphBuilderReturn pp = 
      cstate->build(the_for->get_pre_pad());
    cstate->connect_entry_exit(prev, pp, bdy.get_entry());
  } else {
    if (test2_node)
      cstate->connect_edge(test2_node, bdy.get_entry());
    else
      cstate->connect_edge(prev, test_node);
      
  }
  cstate->connect_edge(bdy.get_exit(), continue_node);
  cstate->connect_edge(continue_node, increment_entry);
  cstate->connect_edge(increment_exit, test_node);
  cstate->connect_edge(test_node, bdy.get_entry());
  cstate->connect_edge(test_node, break_node);
  cstate->connect_edge(break_node, the_exit);

  return(CFGraphBuilderReturn(the_entry, the_exit));
}


static CFGraphBuilderReturn
handle_static_if_statement(CFGraphState *cstate,
			       ExecutionObject *obj)
{
  IfStatement *the_if = to<IfStatement>(obj);

  CFGraphNode *the_entry = cstate->get_entry_node(the_if, false);
  CFGraphNode *the_exit = cstate->get_exit_node(the_if);
  //           Begin
  //             |
  //             v
  //           TEST
  //           /  \ 
  //          v    v
  //         IF  ELSE
  //          \   / 
  //           \ /  
  //            v
  //           END

  CFGraphNode *test_node = 
    cstate->create_test_node(the_if, true, the_if->get_condition());
  
  cstate->connect_edge(the_entry, test_node);

  CFGraphBuilderReturn thn = cstate->build(the_if->get_then_part());
  cstate->connect_entry_exit(test_node, thn, the_exit);

  CFGraphBuilderReturn els = cstate->build(the_if->get_else_part());
  cstate->connect_entry_exit(test_node, els, the_exit);

  return(CFGraphBuilderReturn(the_entry, the_exit));
}

static CFGraphBuilderReturn
handle_static_while_statement(CFGraphState *cstate,
				  ExecutionObject *obj)
{
  WhileStatement *the_while = to<WhileStatement>(obj);

  CFGraphNode *the_entry = cstate->get_entry_node(the_while, false);
  CFGraphNode *the_exit = cstate->get_exit_node(the_while);
  //           Begin
  //             |
  //             v
  //       +-> TEST---+
  //       |     |    |
  //       |     v    |
  //       +--(CONT)  |
  //       |     |    |
  //       |     v    |
  //       +--(BREAK)-+
  //       |     |    |
  //       |     v    |
  //       +-- BODY   |
  //             |    |
  //             v    | 
  //            END <-+
  
  CFGraphNode *test_node = 
    cstate->create_test_node(the_while, true, the_while->get_condition());
  cstate->connect_edge(the_entry, test_node);
  cstate->connect_edge(test_node, the_exit);

  CFGraphNode *continue_node = 
    cstate->create_label_node(the_while, "continue", the_while->get_continue_label());
  cstate->connect_edge(continue_node, test_node);

  CFGraphNode *break_node = 
    cstate->create_label_node(the_while, "break", the_while->get_break_label());
  
  cstate->connect_edge(break_node, the_exit);
  
  CFGraphBuilderReturn bdy = cstate->build(the_while->get_body());
  cstate->connect_entry_exit(test_node, bdy, test_node);

  return(CFGraphBuilderReturn(the_entry, the_exit));
}

static CFGraphBuilderReturn
handle_static_do_while_statement(CFGraphState *cstate,
				     ExecutionObject *obj)
{
  DoWhileStatement *the_while = to<DoWhileStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_while, false);
  CFGraphNode *the_exit = cstate->get_exit_node(the_while);
  //           Begin
  //             |
  //             v
  //           BODY <----+
  //             |       |
  //             v       | 
  //          (CONT)--+  |
  //             |    |  |
  //             v    |  |
  //       +--(BREAK) |  |
  //       |          |  |
  //       |     |----+  |
  //       |     v       |
  //       |   TEST------+
  //       |     |
  //       |     v
  //       +---->END
  CFGraphNode *test_node = 
    cstate->create_test_node(the_while, true, the_while->get_condition());
  cstate->connect_edge(test_node, the_exit);

  CFGraphNode *continue_node = 
    cstate->create_label_node(the_while,
			      "continue", the_while->get_continue_label());
  CFGraphNode *break_node = 
    cstate->create_label_node(the_while, 
			      "break", the_while->get_break_label());
  
  cstate->connect_edge(break_node, the_exit);
  cstate->connect_edge(continue_node, test_node);
  
  CFGraphBuilderReturn bdy = cstate->build(the_while->get_body());
  cstate->connect_entry_exit(the_entry, bdy, test_node);
  if (the_while->get_body() != NULL) {

    cstate->connect_edge(test_node, bdy.get_entry());
  } else {
    cstate->connect_edge(test_node, test_node);
  }

  return(CFGraphBuilderReturn(the_entry, the_exit));
}
static CFGraphBuilderReturn
handle_static_scope_statement(CFGraphState *cstate,
				  ExecutionObject *obj)
{
  ScopeStatement *the_scope = to<ScopeStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_scope, false);
  CFGraphNode *the_exit = cstate->get_exit_node(the_scope);
  //
  
  CFGraphBuilderReturn bdy = cstate->build(the_scope->get_body());
  cstate->connect_entry_exit(the_entry, bdy, the_exit);

  return(CFGraphBuilderReturn(the_entry, the_exit));
}
static CFGraphBuilderReturn
handle_static_return_statement(CFGraphState *cstate,
				   ExecutionObject *obj)
{
  ReturnStatement *the_ret = to<ReturnStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_ret, true);
  //  CFGraphNode *the_exit = cstate->get_exit_node(the_ret);
  //
  //  ProcedureDefinition *pd = get_procdef_from_scoped_object(the_ret);
  ProcedureDefinition *pd = get_procedure_definition(the_ret);
  cstate->connect_edge(the_entry, cstate->get_exit_node(pd));
		       //the_exit);
  
  return(CFGraphBuilderReturn(the_entry, NULL));
} 
static CFGraphBuilderReturn
handle_static_jump_statement(CFGraphState *cstate,
				 ExecutionObject *obj)
{
  JumpStatement *the_jump = to<JumpStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_jump, true);
  //  CFGraphNode *the_exit = cstate->get_exit_node(the_jump);
  //
  cstate->connect_edge_to_label(the_entry, 
				the_jump->get_target());
  return(CFGraphBuilderReturn(the_entry, NULL));
} 

  // Any label with address taken in this procedure.
static CFGraphBuilderReturn
handle_static_jump_indirect_statement(CFGraphState *cstate,
					  ExecutionObject *obj)
{
  JumpIndirectStatement *the_jump = to<JumpIndirectStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_jump, true);
  //  CFGraphNode *the_exit = cstate->get_exit_node(the_jump);
  //
  cstate->add_pending_indirect_jump(the_entry);
  
  return(CFGraphBuilderReturn(the_entry, NULL));
} 
static CFGraphBuilderReturn
handle_static_branch_statement(CFGraphState *cstate,
				   ExecutionObject *obj)
{
  BranchStatement *the_branch = to<BranchStatement>(obj);

  CFGraphNode *the_entry = cstate->get_entry_node(the_branch, true);
  CFGraphNode *the_exit = cstate->get_exit_node(the_branch);
  //
  cstate->connect_edge_to_label(the_entry, 
				the_branch->get_target());
  cstate->connect_edge(the_entry, the_exit);
  
  
  return(CFGraphBuilderReturn(the_entry, the_exit));
} 

static CFGraphBuilderReturn
handle_static_multi_way_branch_statement(CFGraphState *cstate,
					     ExecutionObject *obj)
{
  MultiWayBranchStatement *the_mwb = to<MultiWayBranchStatement>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_mwb, true);
  //  CFGraphNode *the_exit = cstate->get_exit_node(the_mwb);
  //
  CFGraphNode *test_node = 
    cstate->create_test_node(the_mwb, true, the_mwb->get_decision_operand());
  cstate->connect_edge(the_entry, test_node);


  cstate->connect_edge_to_label(test_node, 
				the_mwb->get_default_target());
  typedef indexed_list<IInteger,CodeLabelSymbol* >::pair case_pair;

  Iter<case_pair > iter = the_mwb->get_case_iterator();
  for (; iter.is_valid(); iter.next()) {
    case_pair p = iter.current();
    cstate->connect_edge_to_label(test_node, 
				  p.second);
  }
  
  return(CFGraphBuilderReturn(the_entry, NULL));
} 
static CFGraphBuilderReturn
handle_static_label_location_statement(CFGraphState *cstate,
					   ExecutionObject *obj)
{
  LabelLocationStatement *the_label = to<LabelLocationStatement>(obj);

  CodeLabelSymbol *lab = the_label->get_defined_label();
  CFGraphNode *label_node = cstate->create_label_node(the_label, "label", 
						      lab);
  cstate->register_label_location(lab, the_label);
  if (lab->get_is_address_taken()) {
    cstate->add_address_taken_label(lab);
  }
		     
  return(CFGraphBuilderReturn(label_node, label_node));
}

static CFGraphBuilderReturn
handle_static_mark_statement(CFGraphState *cstate,
				 ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}

static CFGraphBuilderReturn
handle_static_va_start_statement(CFGraphState *cstate,
				     ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}
static CFGraphBuilderReturn
handle_static_va_end_statement(CFGraphState *cstate,
				   ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}
static CFGraphBuilderReturn
handle_static_store_statement(CFGraphState *cstate,
				  ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}
static CFGraphBuilderReturn
handle_static_store_variable_statement(CFGraphState *cstate,
					   ExecutionObject *obj)
{
  return(do_simple_statement(cstate, to<Statement>(obj)));
}


static CFGraphBuilderReturn
handle_static_statement_list(CFGraphState *cstate,
				 ExecutionObject *obj)
{
  StatementList *the_statement_list = to<StatementList>(obj);
  CFGraphNode *the_entry = cstate->get_entry_node(the_statement_list, false);
  //  CFGraphNode *the_exit = NULL;
  //CFGraphNode *the_exit = cstate->get_exit_node(the_statement_list);
  //
  CFGraphNode *prev = the_entry;
  bool has_statements = false;

  for (int i =0; i< the_statement_list->get_statement_count(); i++) {
    Statement *st = the_statement_list->get_statement(i);
    if (st == NULL) continue;
    
    CFGraphBuilderReturn ret = cstate->build(st);
    if (ret.get_entry() == NULL) {
      suif_assert(ret.get_exit() == NULL);
      continue;
    }
    has_statements = true;
    cstate->connect_edge(prev, ret.get_entry());
    //cstate->connect_entry_exit(prev, ret, NULL);
    prev = ret.get_exit();
  }
  if (has_statements)
    return(CFGraphBuilderReturn(the_entry, prev));
  return(CFGraphBuilderReturn(the_entry, the_entry));
}
  
  
// instructions with control flow
static CFGraphBuilderReturn
handle_static_call_statement(CFGraphState *cstate,
				 ExecutionObject *obj)
{
  CallStatement *cal = to<CallStatement>(obj);

  CFGraphNode *the_entry = cstate->get_proc_call_node(cal);
  CFGraphNode *the_exit = cstate->get_proc_return_node(cal);

  cstate->connect_edge_if_empty(the_entry, the_exit);

  return(CFGraphBuilderReturn(the_entry, the_exit));
}
