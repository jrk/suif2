#ifndef __SUIF_CFGRAPH_QUERY__
#define __SUIF_CFGRAPH_QUERY__

#include <suifnodes/suif.h>
#ifdef BUILDING_LIBRARY
#include "suif_cfgraph.h"
#else
#include <suif_cfgraph/suif_cfgraph.h>
#endif
#include <sgraph/sgraph.h>
#include <sgraph/sgraph_iter.h>
#include <sgraph_algs/cfgraph_algs.h>
#include <super_graph/super_graph_forwarders.h>
#include <basicnodes/basic.h>

class CFGraphQuery {
  SuifEnv *_env;
  CFGraphAnnote *_ann;
  // two builders, one forwards, the other backwards
  DFBuild *_algos;
  DFBuild *_reverse_algos;
  // These are the forward graphs
  SGraph *_graph;
  // Reverse of the graph above
  SGraph *_reverse_graph;
  SuperGraph *_super_graph;
  void build_stmt_node_mapping();
public:
  CFGraphQuery(SuifEnv *env, ProcedureDefinition *pd);

  ~CFGraphQuery();

  int body_available() const { return _ann != NULL; };
  CFGraphAnnote *get_suif_cfgraph_annote() const { return _ann; }
  SGraph *get_cfg() const { return(_graph); }
  SGraph *get_reverse_cfg() const { return(_reverse_graph); }
  SuperGraph *get_super_graph() const { return(_super_graph); }
  void invalidate_annotes();

  static SGraph *build_cfg(CFGraphAnnote *an);
  
  /**
   * true if there is a possible effect from an
   * associated ExecutionObject
   */
  bool is_executable(CFGraphNode *node) const;
  /**
   * true if it is executable and the effect
   * comes from a virtual dismantling of a
   * complex statement
   */
  bool is_fake_executable(CFGraphNode *node) const;
  ExecutionObject *get_executable(CFGraphNode *node) const;

  // If not fake, return the executable.
  // If it is fake but has a one-to-one correspondence with
  // a real object, return the real object
  // Else return NULL
  ExecutionObject *get_real_executable(CFGraphNode *node) const;

  /**
	\warning The functions below may returns some nodes that
	you wouldn't expect to find in the CFG. Make sure you
	use of the functions above to find out whether they are
	fake.
  */

  // iterators and such
  SGraphNode get_entry_node() const;
  SGraphNode get_exit_node() const;

  SGraphNodeList *get_nodes() const;

  SNodeIter get_node_iterator() const;
  SNodeIter get_successors(SGraphNode n);
  SNodeIter get_predecessors(SGraphNode n);

  // dominance of various flavors
  
  /**
	This returns the dominator graph -- edge (y,x) is
	in the graph if if every path from Entry to y contains
	x. Note that this is not a tree.
  */
  SGraph* get_dominator_graph();

  /**
	Returns all dominators of node n. Equivalent to calling
	get_successors in the dominator graph.
  */
  SNodeIter get_dominators(SGraphNode n);

  /**
	Return the i-dominance tree. x=idom(y) iff
	x strictly dominates y and every other dominator
	of y (except for x) dominates x. This is a tree, 
	where Entry node is reachable to by all other nodes.
  */
  SGraph* get_immediate_dominator_graph();

  /**
	Return the immediate dominator of node \a n.
	My understanding is that in all cases except 
	when called on the Entry node, this iterator 
	will contain one element.
  */
  SNodeIter get_immediate_dominators(SGraphNode n);

  /**
	These ar similar relations on the reverse graph.
	Edge (x, y) is present if every edge from x to 
	Exit goes through y.
  */
  SGraph* get_post_dominator_graph();

  /**
	Same as above for a specific node
  */
  SNodeIter get_post_dominators(SGraphNode n);

  /**
    This is a tree version
  */
  SGraph* get_immediate_post_dominator_graph();

  /**
	And this is the same thing for a node
  */
  SNodeIter get_immediate_post_dominators(SGraphNode n);

  /**
	Node m is in DF of n if m dominates some predecessor 
	of n but doesn't strictly dominate n.
  */

  SNodeIter get_dominance_frontier(SGraphNode n);

  /**
      IDF is the transitive closure of the DF relation.
  */
  SNodeIter get_iterated_dominance_frontier(SGraphNode n);

  /**
	Returns all nodes n is control dependent upon.
	This is the same relation as above, but in the reverse 
	graph. Intuitively, n is control dependent on m if
	n postdominates successors of m, but doesn't strictly 
	postdominate m.
  */

  SNodeIter get_control_dependence(SGraphNode n);

  /**
	Checks if node \a n is control dependent on anything.

	This is equivalent to checking if the iterator above is
	empty.
  */
  bool is_control_dependent(SGraphNode n){
	return get_control_dependence(n).is_valid();
  }

  SGraphNode get_only_control_dependence(SGraphNode n){
	SNodeIter iter = get_control_dependence(n);
	suif_assert(iter.is_valid());
	SGraphNode result = iter.current();
	iter.next();
	suif_assert(!iter.is_valid());
	return result;
  }

  /**
    This is the iterated version.
  */
  SNodeIter get_iterated_control_dependence(SGraphNode n);

  // converters
  SGraphNode suifobj_to_node(AnnotableObject *s);
  AnnotableObject *node_to_suifobj(SGraphNode n);
  CFGraphNode *get_node(SGraphNode n) const;
  /* Does n dominate m */
  int dominates(SGraphNode n, SGraphNode m);
  /* Does n post-dominate m */
  int post_dominates(SGraphNode n, SGraphNode m);

  //  void export_dot(ion *out, suif_vector<String> *node_name_map);
  void print_graph(ion *out);
  void print_dot(ion *out);
};

#endif
