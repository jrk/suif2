#include "suif_cfgraph.h"
#include "suif_cfgraph_factory.h"
#include "suif_cfgraph_query.h"
#include "suifkernel/print_subsystem.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "ion/ion.h"
#include "sgraph_algs/sgraph_algs.h"
#include "sgraph_algs/cfgraph_algs.h"
#include "sgraph/sgraph_iter.h"
#include "super_graph/super_graph.h"
#include "cfgraph_module.h"
#include "utils/trash_utils.h"
#include <assert.h>


CFGraphQuery::CFGraphQuery(SuifEnv *env, ProcedureDefinition *pd):
  _env(env), _ann(NULL), 
  _algos(NULL), _reverse_algos(NULL), 
  _graph(NULL), _reverse_graph(NULL), _super_graph(NULL)
{
  suif_assert_message(pd, 
    ("Trying to create a CFGraphQuery for an undefined procedure"));
  
  Annote *an = pd->peek_annote("cfg_proc_annote");
  if (an == NULL) {
    Module *m = env->get_module_subsystem()->retrieve_module("suif_cfgraph_builder");
    suif_assert_message(m,
			("No suif_cfgraph_builder in the system"));
    SuifCFGraphBuilderModule *bld = (SuifCFGraphBuilderModule *)m;
    bld->build_cfgraph(pd);
    an = pd->peek_annote("cfg_proc_annote");
    suif_assert_message(an,
			("suif_cfgraph_builder failed"));
  }

  // construct the CFG
  _ann = to<CFGraphAnnote>(an);
  _graph = build_cfg(_ann);

  // construct a forward builder, construct dominators
  _algos = new DFBuild(_graph, 
				_ann->get_entry_node(),
		       _ann->get_exit_node());
  _algos->do_build_dominators();

   build_stmt_node_mapping();

  // construct a backward builder, construct post-dominators
   _reverse_graph = new SGraphList();
   build_reverse_graph(_reverse_graph, _graph);
  _reverse_algos = new DFBuild(_reverse_graph, 
				_ann->get_exit_node(),
				_ann->get_entry_node());
  _reverse_algos->do_build_dominators();

  // build the super graph
  _super_graph = new SuperGraph(_graph,
				_ann->get_entry_node(),
				_ann->get_exit_node());
}

CFGraphQuery::~CFGraphQuery(){
  delete _algos;
  delete _reverse_algos;
  delete _graph;
  delete _reverse_graph;
  delete _super_graph;
}

void CFGraphQuery::invalidate_annotes() 
{
  Annote *an = 
    to<AnnotableObject>(_ann->get_parent())->take_annote(_ann->get_name());
  suif_assert(_ann == an);

  while (_ann->get_node_count() != 0) {
    CFGraphNode *node = _ann->get_node(0);
    _ann->remove_node(0);
    ExecutionObject *eo = node->get_owned_object();
    if (eo) {
      node->set_owned_object(0);
      trash_it(eo->get_suif_env(), eo);
    }
    trash_it(node->get_suif_env(), node);
  }
  trash_it(an->get_suif_env(), an);
  delete _algos;
  _algos = 0;
  delete _graph;
  _graph = 0;
  delete _super_graph;
  _super_graph = 0;
  _ann = 0;
}

SGraph *CFGraphQuery::build_cfg(CFGraphAnnote *an) {
  SGraphList *l = new SGraphList();
  {for (size_t i = 0; i < an->get_node_count(); i++) {
    l->add_node(i);
}}
  
  {for (size_t i = 0; i < an->get_successor_count(); i++) {
    CFGraphNodeList *succs = an->get_successor(i);
    if (succs) {
      for (Iter<CFGraphNode*> iter = succs->get_node_iterator();
	   iter.is_valid(); iter.next()) {
      	CFGraphNode *n = iter.current();
	      l->add_edge(SGraphEdge(i, n->get_id()));
      }
    }
  }}
  return(l);
}

void
CFGraphQuery::build_stmt_node_mapping() {
  Iter<CFGraphNode* sf_owned> iter = _ann->get_node_iterator();
  while (iter.is_valid()) {
    CFGraphNode *cur = iter.current();
    AnnotableObject *base_obj = cur->get_base();
    SuifObjAnnote *ann = create_suif_obj_annote(_env, "obj_cfg_node", 
						cur->get_id());
    base_obj->append_annote(ann);
    iter.next();
  }
}

SGraphNode
CFGraphQuery::get_entry_node() const {
  assert(body_available());
  return _ann->get_entry_node();
}

SGraphNode
CFGraphQuery::get_exit_node() const {
  assert(body_available());
  return _ann->get_exit_node();
}

SNodeIter 
CFGraphQuery::get_node_iterator() const {
  assert(body_available());
  return _graph->get_node_iterator();
}

SNodeIter 
CFGraphQuery::get_successors(SGraphNode n) {
  assert(body_available());
  return _graph->get_node_successor_iterator(n);
}

SNodeIter 
CFGraphQuery::get_predecessors(SGraphNode n) {
  assert(body_available());
  return _graph->get_node_predecessor_iterator(n);
}

SNodeIter 
CFGraphQuery::get_dominators(SGraphNode n) {
  return get_dominator_graph()->
	  get_node_successor_iterator(n);
}

SGraph*
CFGraphQuery::get_dominator_graph(){
  assert(body_available());
  _algos->do_build_dominators();
  return _algos->get_dominators();
}

SNodeIter 
CFGraphQuery::get_immediate_dominators(SGraphNode n) {
  return get_immediate_dominator_graph()->
	  get_node_successor_iterator(n);
}

SGraph*
CFGraphQuery::get_immediate_dominator_graph(){
  assert(body_available());
  _algos->do_build_immediate_dominators();
  return _algos->get_immediate_dominators();
}



SNodeIter 
CFGraphQuery::get_post_dominators(SGraphNode n) {
  return get_post_dominator_graph()->
	  get_node_successor_iterator(n);
}

SGraph*
CFGraphQuery::get_post_dominator_graph(){
  assert(body_available());
  _reverse_algos->do_build_dominators();
  return _algos->get_dominators();
}

SNodeIter 
CFGraphQuery::get_immediate_post_dominators(SGraphNode n) {
  return get_immediate_dominator_graph()->
	  get_node_successor_iterator(n);
}

SGraph*
CFGraphQuery::get_immediate_post_dominator_graph(){
  assert(body_available());
  _reverse_algos->do_build_immediate_dominators();
  return _algos->get_immediate_dominators();
}

SNodeIter 
CFGraphQuery::get_dominance_frontier(SGraphNode n) {
  assert(body_available());
  _algos->do_build_dominance_frontier();
  return _algos->get_dominance_frontier()->
	  get_node_successor_iterator(n);
}

SNodeIter 
CFGraphQuery::get_iterated_dominance_frontier(SGraphNode n) {
  assert(body_available());
  _algos->do_build_iterated_dominance_frontier();
  return _algos->get_iterated_dominance_frontier()->
	  get_node_successor_iterator(n);
}

SNodeIter 
CFGraphQuery::get_control_dependence(SGraphNode n) {
  assert(body_available());
  _reverse_algos->do_build_dominance_frontier();
  return _reverse_algos->get_dominance_frontier()->
	  get_node_successor_iterator(n);
}

SNodeIter 
CFGraphQuery::get_iterated_control_dependence(SGraphNode n) {
  assert(body_available());
  _reverse_algos->do_build_iterated_dominance_frontier();
  return _reverse_algos->get_iterated_dominance_frontier()->
	  get_node_successor_iterator(n);
}

SGraphNode 
CFGraphQuery::suifobj_to_node(AnnotableObject *s) {
  SuifObjAnnote *ann = to<SuifObjAnnote>(s->peek_annote("obj_cfg_node"));
  assert(ann);
  return ann->get_cfg_node_num();
}

AnnotableObject *
CFGraphQuery::node_to_suifobj(SGraphNode n) {
  return _ann->get_node(n)->get_base();
}

int
CFGraphQuery::dominates(SGraphNode n, SGraphNode m) {
  assert(body_available());
  SGraph *dominators = _algos->get_dominators();
  SNodeIter dom_iter = dominators->get_node_successor_iterator(m, 1);
  while (dom_iter.is_valid()) {
    if (dom_iter.current() == n) return 1;
    dom_iter.next();
  }
  return 0;
}

int
CFGraphQuery::post_dominates(SGraphNode n, SGraphNode m) {
  assert(body_available());
  SGraph *post_dominators = _reverse_algos->get_dominators();
  SNodeIter post_dom_iter = 
	  post_dominators->get_node_successor_iterator(m, 1);
  while (post_dom_iter.is_valid()) {
    if (post_dom_iter.current() == n) return 1;
    post_dom_iter.next();
  }
  return 0;
}

SGraphNodeList *
CFGraphQuery::get_nodes() const {
  return _algos->get_reverse_postorder_list();
}

CFGraphNode *
CFGraphQuery::get_node(SGraphNode n) const {
  return _ann->get_node(n);
}



void CFGraphQuery::print_graph(ion *the_ion)
{
  CFGraphAnnote *cfg_an = _ann;
  SGraph *the_super_graph = _algos->get_reachable_graph();

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

bool CFGraphQuery::is_executable(CFGraphNode *node) const {
  return(node->get_is_executable());
}

bool CFGraphQuery::is_fake_executable(CFGraphNode *node) const {
  return (is_executable(node) &&
	  node->get_owned_object() != NULL);
}

ExecutionObject *CFGraphQuery::get_executable(CFGraphNode *node) const {
  if (!is_executable(node)) return(NULL);
  ExecutionObject *eo = node->get_owned_object();
  if (eo == NULL)
    eo = to<ExecutionObject>(node->get_base());
  return(eo);
}

ExecutionObject *CFGraphQuery::get_real_executable(CFGraphNode *node) const {
  if (!is_executable(node)) return(NULL);
  ExecutionObject *eo = node->get_owned_object();
  if (eo == NULL ||
      node->get_is_one_to_one())
    return(to<ExecutionObject>(node->get_base()));
  return(NULL);
}


void CFGraphQuery::print_dot(ion *the_ion)
{
  CFGraphAnnote *cfg_an = _ann;
  SGraph *the_graph = _algos->get_reachable_graph();
  
  PrintSubSystem *psub = cfg_an->get_suif_env()->get_print_subsystem();
  suif_vector<String> cfg_names;
  for (SNodeIter bv_iter(the_graph->get_node_iterator());
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
    cfg_names[node] = str + String(" : ")+String(node);
  }
  export_named_dot(the_graph, the_ion,
		   &cfg_names);
  
}
