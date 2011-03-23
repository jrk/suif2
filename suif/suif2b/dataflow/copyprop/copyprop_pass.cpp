/* file "copyprop_pass.cpp" */


/*
       Copyright (c) 1999,2000 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


/*
      This is the main implementation file for copy prop
*/


#include "suifpasses/passes.h"
#include "var_prop.h"
#include "iokernel/cast.h"
#include "basicnodes/basic.h"
#include "suif_cfgraph/suif_cfgraph_query.h"
#include "super_graph/super_graph.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "sgraph_algs/sgraph_algs.h"
//#include "ecr_alias/expression_string.h"
#include "ion/ion.h"
#include "bit_vector/bit_vector.h"
#include "suifnodes/suif.h"
#include "utils/expression_utils.h"
#include "bit_vector/cross_map.h"
#include "utils/semantic_helper.h"
#include "utils/trash_utils.h"
#include "suifkernel/print_subsystem.h"
#include "suifkernel/utilities.h"
#include "suifkernel/command_line_parsing.h"

class CopyPropPass : public PipelinablePass {
  OptionLiteral *_verbose_output;

public:
  CopyPropPass(SuifEnv *suif_env, 
	const LString &name = "copyprop");
  void initialize();

  Module *clone() const { return (Module *)this; }
  void do_procedure_definition(ProcedureDefinition *pd);
};




/**
 * Simple copy propagation.
 * This pass is NOT idempotent.
 * running it multiple times may work better than
 * running it only once.
 * In addition, it is helpful to run
 *   various cleanup passes after this pass
 *   like deadcode, gc_symbol_table, etc
 */
CopyPropPass::CopyPropPass(SuifEnv *suif_env, const LString &name) :
  PipelinablePass(suif_env, name),
  _verbose_output(NULL)
{}

void
CopyPropPass::initialize() {
  Module::initialize();
  _command_line->set_description("Data-flow based copy propagation pass");
  _verbose_output = new OptionLiteral("-v");
  OptionSelection *sel = new OptionSelection();
  sel->add(_verbose_output);
  OptionLoop *opt = new OptionLoop(sel, true);
  _command_line->add(opt);
}

class ExprReplacement {
public:
  Expression *_orig;
  Expression *_repl;
  ExprReplacement(Expression *orig, Expression *repl) : 
    _orig(orig), _repl(repl)
  {}
    
};
static bool double_entered(list<ExprReplacement> *repl,
			   Expression *orig) {
  for (list<ExprReplacement>::iterator iter = repl->begin();
       iter != repl->end(); iter++) {
    if ((*iter)._orig == orig)
      return(true);
  }
  return (false);
}
#if 0
static void print_cfgraph(CFGraphAnnote *cfg_an, 
			  SGraph *the_super_graph,
			  ion *the_ion)
{
  PrintSubSystem *psub = cfg_an->get_suif_env()->get_print_subsystem();
  suif_vector<String> cfg_names;
  for (SNodeIter bv_iter(the_super_graph->get_node_iterator());
       bv_iter.is_valid(); bv_iter.next()) {
    SGraphNode node = bv_iter.current();
    CFGraphNode *cnode = cfg_an->get_node(node);
    if (!cnode->get_is_executable()
	&& !is_kind_of<LabelLocationStatement>(cnode->get_base()))
      continue;
    
    AnnotableObject *aobj = cnode->get_owned_object();
    if (aobj == NULL) {
      aobj = cnode->get_base();
    }
    String str = psub->print_to_string("cprint",  aobj);
    while (cfg_names.size() <= node) { cfg_names.push_back(emptyString); }
    cfg_names[node] = str;
  }
  stderr_ion->printf("The CFG\n");
  print_named(the_super_graph, stderr_ion,
	      &cfg_names);
  
}
#endif

void CopyPropPass::do_procedure_definition(ProcedureDefinition *pd) {
  bool verbose = _verbose_output->is_set();

  CrossMap<VariableSymbol*> *var_map = 
    new CrossMap<VariableSymbol*>();
  //  init_var_defs_map(var_map, pd);

  CFGraphQuery q(get_suif_env(), pd);
  //  CFGraphAnnote *cfg_an = 
  //    to<CFGraphAnnote>(pd->peek_annote("cfg_proc_annote"));
  //  suif_assert_message(cfg_an != NULL, ("cfg_proc_annote not on ProcedureDefinition"
  //				   "- run suif_cfgraph_pass"));
  
  //  init_var_defs_map(var_map, cfg_an, pd);
  // we should probably remove the unreachables
  //
  //  SuperGraph *the_super_graph = new SuperGraph(cfg_an->get_cfg(), 
  //					       cfg_an->get_entry_node(),
  //					       cfg_an->get_exit_node());

  SuperGraph *the_super_graph = q.get_super_graph();
#if 1
  //  the_super_graph->print_debug();
#endif
  CopyPropSolver *copyprop_solver =
    new CopyPropSolver(var_map, q.get_super_graph(),
		       &q, verbose);
  
  copyprop_solver->init_solver_to_top(); // automatically set to top
  {
    // Initial value, all of the safe parameters are BOTTOM.
    // The rest are TOP.
    CopyPropVectorValue init_val(var_map);
    CopyPropValue bot_val;
    bot_val.set_bottom();
    for (Iter<ParameterSymbol*> param_iter =
	   pd->get_formal_parameter_iterator();
	 param_iter.is_valid(); param_iter.next()) {
      ParameterSymbol *param = param_iter.current();
      if (CopyPropVectorValue::is_safe_var(param))
	init_val.set_variable_value(param, bot_val);
    }
    copyprop_solver->init_entries(init_val); // for this case
  }
  
  copyprop_solver->solve_region(the_super_graph->top_region());

  PrintSubSystem *psub = get_suif_env()->get_print_subsystem();

  // Iterate over every graph node.
  {for (SNodeIter bv_iter(the_super_graph->get_node_iterator());
       bv_iter.is_valid(); bv_iter.next()) {
    SGraphNode node = bv_iter.current();
    //    if (solution.get_bit(node)) continue;
    // check it
    CFGraphNode *cnode = q.get_node(node);
    if (!q.is_executable(cnode)) continue;
    ExecutionObject *eo = q.get_executable(cnode);
    // get the value here.
    const CopyPropVectorValue *val =
      copyprop_solver->get_copy_prop_out_value(node);
    if (verbose) {
      String str = psub->print_to_string("cprint", eo);
      fprintf(stdout, "Node #%d: %s\n",node, str.c_str());
      fprintf(stdout, "Node #%d: Value= ",node);
      if (val != NULL) {
	val->print(stdout_ion);
      } else {
	fprintf(stdout, "Node #%d: Value= (nil)",node);
      }
      fprintf(stdout, "\n");
    }
  }}

  list<ExprReplacement> todo;

  // Expression Replacements.  I think it should
  {for (SNodeIter bv_iter(the_super_graph->get_node_iterator());
       bv_iter.is_valid(); bv_iter.next()) {
    SGraphNode node = bv_iter.current();
    //    if (solution.get_bit(node)) continue;
    // check it
    CFGraphNode *cnode = q.get_node(node);
    if (!q.is_executable(cnode)) continue;
    // we can't replace a virtually dismantled object

    ExecutionObject *eo = q.get_real_executable(cnode);
    if (eo == NULL)
      continue;

    /*
    if (q.is_fake_executable(cnode))
      {
	eo = q.get_fake_direct(cnode);
	if (eo == NULL) continue;
      }
    else
      eo = q.get_executable(cnode);
    */
    
    // get the value here.
    const CopyPropVectorValue *val =
      copyprop_solver->get_copy_prop_in_value(node);
    if (val == NULL) continue;
    
    // For every variable use
    for (Iter<LoadVariableExpression> iter =
	   object_iterator<LoadVariableExpression>(eo);
	 iter.is_valid(); iter.next()) {
      LoadVariableExpression *load = &iter.current();
      VariableSymbol *var = load->get_source();
      
      CopyPropValue local_val = val->get_variable_value(var);
      if (local_val.is_top()) continue;
      if (local_val.is_bottom()) continue;
      Expression *new_expr = 
	local_val.build_expression(load->get_result_type());
      suif_assert_message(load->get_parent() != NULL,
			  ("unconnected value"));
      if (double_entered(&todo, load)) {
	String str = psub->print_to_string("cprint", load);
	String aostr = psub->print_to_string("cprint", eo);
	suif_assert_message(!double_entered(&todo, load),
			    ("double entry: %s on %s", str.c_str(),
			     aostr.c_str()));
      }
      if (verbose) {  
	String str = psub->print_to_string("cprint", load);
	String aostr = psub->print_to_string("cprint", eo);
	String replstr = psub->print_to_string("cprint", new_expr);
	printf("replace: '%s' => '%s' on '%s'\n",
	       str.c_str(), replstr.c_str(), aostr.c_str());
      }
      todo.push_back(ExprReplacement(load, new_expr));
    }
  }}
  delete copyprop_solver;
  if (verbose) {
    q.print_graph(stderr_ion);
  }

  q.invalidate_annotes();

  // Now do the replacements
  for (list<ExprReplacement>::iterator iter = todo.begin();
       iter != todo.end(); iter++) {
    Expression *orig = (*iter)._orig;
    Expression *repl = (*iter)._repl;
    trash_it(get_suif_env(), 
	     replace_expression(orig, repl));
  }
}

extern "C" void EXPORT init_copyprop(SuifEnv *suif_env) {
  suif_env->require_DLL("suifnodes");
  suif_env->require_DLL("dflowsolver");
  suif_env->require_DLL("suifprinter");
  suif_env->require_DLL("suif_cfgraph");

  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  CopyPropPass *pass = new CopyPropPass(suif_env); 
  ms->register_module(pass);

}

