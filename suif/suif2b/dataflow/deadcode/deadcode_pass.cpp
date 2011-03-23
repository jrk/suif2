/* file "deadcode_pass.cc" */


/*
       Copyright (c) 1999 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


/*
      This is the main implementation file of the ecr_alias_pass,
      a library for interprocedural alias analysis
*/


#include "suifpasses/passes.h"
#include "reaching_defs.h"
#include "liveness.h"
#include "iokernel/cast.h"
#include "basicnodes/basic.h"
#include "suif_cfgraph/suif_cfgraph.h"
#include "super_graph/super_graph.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/print_subsystem.h"
#include "sgraph_algs/sgraph_algs.h"
#include "ion/ion.h"
#include "bit_vector/bit_vector.h"
#include "suifnodes/suif.h"
#include "utils/expression_utils.h"
#include "utils/trash_utils.h"
#include "suifkernel/command_line_parsing.h"
#include "suif_cfgraph/suif_cfgraph_query.h"

class DeadcodePass : public PipelinablePass {
  OptionLiteral *_verbose_output;
  OptionLiteral *_dot_output;

public:
  DeadcodePass(SuifEnv *suif_env, 
	const LString &name = "deadcode_pass");
  void initialize();

  Module *clone() const { return (Module *)this; }
  void do_procedure_definition(ProcedureDefinition *pd);
};





DeadcodePass::DeadcodePass(SuifEnv *suif_env, const LString &name) :
  PipelinablePass(suif_env, name),
  _verbose_output(NULL),
  _dot_output(NULL)
{}

void
DeadcodePass::initialize() {
  Module::initialize();
  _command_line->set_description("Data-flow based deadcode elimination pass");

  _verbose_output = new OptionLiteral("-v");
  _dot_output = new OptionLiteral("-dot");
  OptionSelection *sel = new OptionSelection();
  sel->add(_verbose_output);
  sel->add(_dot_output);
  OptionLoop *opt = new OptionLoop(sel, true);
  _command_line->add(opt);
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
  the_ion->printf("The CFG\n");
  print_named(the_super_graph, the_ion,
	      &cfg_names);
  
}

static void print_dot(CFGraphAnnote *cfg_an, 
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
  //stderr_ion->printf("The CFG\n");
  export_named_dot(the_super_graph, the_ion,
		   &cfg_names);
  
}
#endif


void DeadcodePass::do_procedure_definition(ProcedureDefinition *pd) {
  bool verbose = _verbose_output->is_set();
  bool dot_output = _dot_output->is_set();

  VarDefsMap *var_map = new VarDefsMap(); // ref counted.
  //  init_var_defs_map(var_map, pd);

  //  CFGraphAnnote *cfg_an = 
  //    to<CFGraphAnnote>(pd->peek_annote("cfg_proc_annote"));
  
  // Build it if needed.
  CFGraphQuery q(get_suif_env(), pd);

  /*
  suif_assert_message(cfg_an != NULL, 
		      ("cfg_proc_annote not on ProcedureDefinition"
		       "- run suif_cfgraph_pass"));
  */
  
  init_var_defs_map(var_map, &q, pd);
  // we should probably remove the unreachables
  //
  SuperGraph *the_super_graph = new SuperGraph(q.get_cfg(),
					       q.get_entry_node(),
					       q.get_exit_node());
  //  					       cfg_an->get_entry_node(),
  //  					       cfg_an->get_exit_node());
#if 1
  //  the_super_graph->print_debug();
#endif
  ReachingDefsSolver *rdefs_solver =
    new ReachingDefsSolver(var_map, the_super_graph, &q);
  
  rdefs_solver->init_solver_to_top(); // automatically set to top
  //  solver->init_entries(get_bottom()); // for this case
  
  rdefs_solver->solve_region(the_super_graph->top_region());

  if (verbose) {
    fprintf(stdout, "reaching Solution to Proc: %s\n", 
	    pd->get_procedure_symbol()->get_name().c_str());
    rdefs_solver->print(stdout_ion);
  }


  SGraphList reverse_graph;
  build_reverse_graph(&reverse_graph, q.get_cfg());
  
  SuperGraph *reverse_super_graph = 
    new SuperGraph(&reverse_graph,
		   q.get_exit_node(),
		   q.get_entry_node());
  

  LivenessSolver *live_solver = 
    new LivenessSolver(var_map, rdefs_solver, reverse_super_graph, &q);
  
  live_solver->init_solver_to_top(); // automatically set to top
  //  solver->init_entries(get_bottom()); // for this case
  
  live_solver->solve_region(reverse_super_graph->top_region());
  // What to do with it??
  // get the entry value and assume that non-marked statements
  // are dead. OK print it for now
  if (verbose) {
    fprintf(stdout, "liveness Solution to Proc: %s\n", 
	    pd->get_procedure_symbol()->get_name().c_str());
    live_solver->print(stdout_ion);
  }


  // Here is the real solution.
  // look at the liveness result at the procedure entry:
  
  const LivenessValue *val = 
    live_solver->get_liveness();//the_super_graph->top_region()->get_entry());
  BitVector solution;
  if (val) {
    solution = (*val->get_live_set());
  }

  ion *the_ion = stdout_ion;

  PrintSubSystem *psub = get_suif_env()->get_print_subsystem();

  if (verbose) {
    the_ion->printf("Begin: Live statements\n");
    for (BitVectorIter bv_iter(&solution);
	 bv_iter.is_valid(); bv_iter.next()) {
      //    VarDefsMap::StatementId stmt_id = bv_iter.current();
      SGraphNode node = bv_iter.current();
      CFGraphNode *cnode = q.get_node(node);
      if (q.is_executable(cnode)) {
	ExecutionObject *eo = q.get_executable(cnode);
	String str = psub->print_to_string("cprint",  eo);
	the_ion->printf("Node(%u): %s\n", node, str.c_str());
      }
    }
    the_ion->printf("End: Live statements\n");
  }  
  

  // OK, time to walk over all of the statements and
  // remove the StoreVariableStatements that
  //    are not live
  list<StoreVariableStatement*> dead_stores;
  for (SNodeIter bv_iter(the_super_graph->get_node_iterator());
       bv_iter.is_valid(); bv_iter.next()) {
    SGraphNode node = bv_iter.current();
    if (solution.get_bit(node)) continue;
    // check it
    CFGraphNode *cnode = q.get_node(node);
    if (!q.is_executable(cnode)) continue;
    ExecutionObject *eo = q.get_executable(cnode);
    if (q.is_fake_executable(cnode)) continue;

    if (is_kind_of<StoreVariableStatement>(eo)) {
      // It's outta here
      StoreVariableStatement *store = to<StoreVariableStatement>(eo);
      // Don't get rid of stores to address taken variables
      if (!store->get_destination()->get_is_address_taken()) {
	if (verbose) {
	  String str = psub->print_to_string("cprint",  store);
	  the_ion->printf("Removing Stmt(%u): %s\n", node,
			  str.c_str());
	}
	dead_stores.push_back(store);
      }
    }
  }
  if (verbose) {
    q.print_graph(stderr_ion);
  }
  if (dot_output) {
    const char *fname = 
      (String(pd->get_procedure_symbol()->get_name()) + ".dot").c_str();
    FILE *fp = fopen(fname, "w");
    if (!fp) {
      suif_warning("Could not open file '%s'\n", fname);
    } else {
      file_ion the_ion(fp);
      q.print_dot(&the_ion);
      fclose(fp);
    }
  }
  // What to do with it??
  //  fprintf(stdout, "Solution to Proc: %s\n", pd->get_procedure_symbol()->get_name().c_str());
  //  solver->print(stdout_ion);


  delete live_solver;
  delete rdefs_solver;

  q.invalidate_annotes();

  // Now remove the dead statements;
  for (list<StoreVariableStatement*>::iterator siter =
	 dead_stores.begin(); 
       siter != dead_stores.end(); siter++) {
    StoreVariableStatement *store = (*siter);
    remove_statement(store);
    trash_it(get_suif_env(), store);
  }
  
}

extern "C" void EXPORT init_basicnodes(SuifEnv *suif_env);
extern "C" void EXPORT init_suifnodes(SuifEnv *suif_env);
extern "C" void EXPORT init_bit_vector(SuifEnv *suif_env);
extern "C" void EXPORT init_dflowsolver(SuifEnv *suif_env);
extern "C" void EXPORT init_ecr_alias(SuifEnv *suif_env);

extern "C" void EXPORT init_deadcode(SuifEnv *suif_env) {
  suif_env->require_module("suifnodes");
  suif_env->require_module("dflowsolver");
  suif_env->require_module("suifprinter");
  suif_env->require_module("suif_cfgraph");

  ModuleSubSystem *ms = suif_env->get_module_subsystem();
  DeadcodePass *pass = new DeadcodePass(suif_env); 
  ms->register_module(pass);

}

//extern void init_ecr_annote_cloning(CloneSubSystem *css) {}
