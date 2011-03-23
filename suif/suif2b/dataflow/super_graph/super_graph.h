#ifndef SUPER_GRAPH_H
#define SUPER_GRAPH_H

#include "bit_vector/bit_vector_forwarders.h"
#include "sgraph/sgraph.h"
// List implementation
#include "sgraph/sgraph_list.h"
#include "super_graph_forwarders.h"


class SuperGraphRegion {
  SuperGraph *_parent;

  // if neither regular or irregular loop, we have no backedges.
  bool _regular_loop; // If true, we have 1 backedge and 1 header.
  bool _irregular_loop; // If true, we MUST use a special non-loop widener
  // at any "backedge"
  
  // defines the tree.
  SuperGraphRegion *_parent_region;
  list<SuperGraphRegion*> *_children_regions;

  BitVector *_members;
  BitVector *_child_members;
  SGraphNodeList *_backedge_nodes; // probably only one

  SGraphNodeList *_entries;
  SGraphNodeList *_exits;
  SGraphNode _loop_header;

  int _region_id;
 private:
  SuperGraphRegion(const SuperGraphRegion &);
  SuperGraphRegion &operator=(const SuperGraphRegion &);
 public:
  // We can always get the super graph region
  // from the 
  SuperGraphRegion(int region_id,
		   SuperGraph *parent, 
		   SuperGraphRegion *parent_region,
		   const BitVector &members,
		   SGraphNodeList *entries,
		   SGraphNodeList *exits);
  
  // We can query the Proper Region of any node with a
  // method in the SuperGraph.
  SuperGraph *get_super_graph() const;

  SuperGraphRegion *get_parent_region() const; // null if top region.
  const list<SuperGraphRegion*> &get_children_regions() const;

  bool is_regular_loop() const;
  bool is_irregular_loop() const;
  bool is_loop() const; // true => is_regular_loop() || is_irregular_loop()

  // Every interface from one region to another has
  // unique entry/exit members.  the sub-region entry/exits 
  // are owned by the sub-region.
  //
  // The following directed graph (with entry N1 and exit N4)
  // 
  //        +-----+
  //        v     |
  //  N1 -> N2 -> N3 -> N4
  //
  // will  be transformed to look like Nodes will look like
  //
  //                   +-----+
  //                   v     |
  //  N1 -> ( entry -> N2 -> N3 -> exit ) -> N4
  //         
  
  
  bool is_member(SGraphNode node) const;
  bool is_child_member(SGraphNode node) const;
  bool is_proper_member(SGraphNode node) const; 
  // == is_member && !is_child_member


  SGraphNode get_entry() const; // assert if there is not exactly 1 exit
  SGraphNode get_exit() const; // assert if there is not exactly 1 exit
  SNodeIter get_exit_iterator() const;
  SNodeIter get_entry_iterator() const; 

  SNodeIter get_proper_member_iterator() const;

  bool is_entry(SGraphNode node) const;
  bool is_exit(SGraphNode node) const;
  bool is_backedge(SGraphNode node) const;
  bool is_loop_header(SGraphNode node) const; // only true for a regular loop

  SGraphNodeList *create_reverse_postorder_list() const;

 private:
  
  void add_member(SGraphNode node);
  void remove_member(SGraphNode node);
  
  void add_child_member(SGraphNode node);
  void remove_child_member(SGraphNode node);
};

// In addition to the interface by number of the sgraph,
// the SuperGraph also provides an interface to SuperGraphNodes.
class SuperGraph : public SGraph {
  // The underlying graph
  // may have unreachable nodes.

  SGraph *_graph;

  BitVector *_reachable_nodes;
  suif_vector<SuperGraphRegion *> *_regions;
  suif_vector<SuperGraphRegion *> *_region_map;
  SuperGraphRegion *_top_region;

 public:
  // The reachable portion of this graph will be 
  //    copied into the  _reachable_graph.
  // Every region is defined by the set of nodes in 
  SuperGraph(SGraph *the_graph,
	     SGraphNode entry,
	     SGraphNode exit);

  SuperGraph(SGraph *the_graph, 
	     list<SGraphNode> *entries,
	     list<SGraphNode> *exits);
  SuperGraph(SGraph *the_graph, 
	     const BitVector &entries,
	     const BitVector &exits);

  SuperGraphRegion *top_region() const;


  void recalc(); // After a change in the graph, we need to

  // recalculate the regions, etc.
  void renumber(); // Renumbers the nodes.
  
  // returns null if not reachable.
  SuperGraphRegion *find_region(SGraphNode) const;


  // Add in the stuff we need
  // because we implement the sgraph interface
  SGraphNode max_num_nodes() const;
  bool is_node_member(SGraphNode node) const;
  void add_node(SGraphNode node);
  void remove_node(SGraphNode node);

  bool is_edge_member(const SGraphEdge &edge) const;
  void add_edge(const SGraphEdge &edge);
  void remove_edge(const SGraphEdge &edge);
  
  SNodeIter get_node_iterator() const;
  SNodeIter get_node_successor_iterator(SGraphNode) const;
  SNodeIter get_node_predecessor_iterator(SGraphNode) const;
  
};
  



#endif /* SUPER_GRAPH_H */
