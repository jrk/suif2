#include "super_graph.h"
#include "common/suif_vector.h"
#include "bit_vector/bit_vector.h"
#include "sgraph_algs/cfgraph_algs.h"
#include "suifkernel/suifkernel_messages.h"
#include "sgraph/sgraph_iter_impl.h"
#include "sgraph/sgraph_bit_iter.h"
#include "suifkernel/suif_env.h"
#include "sgraph_algs/sgraph_algs.h"

SuperGraph::SuperGraph(SGraph *the_graph,
		       SGraphNode entry,
		       SGraphNode the_exit) :
  _graph(the_graph),
  _reachable_nodes(new BitVector),
  _regions(new suif_vector<SuperGraphRegion*>),
  _region_map(new suif_vector<SuperGraphRegion*>),
  _top_region(NULL)
{
  SGraphNodeList *entries = new SGraphNodeList;
  SGraphNodeList *exits = new SGraphNodeList;

  entries->push_back(entry);
  exits->push_back(the_exit);

  DFBuild build(the_graph, entry, the_exit);
  build.do_build_dominators();
  SGraph *dom = build.get_dominators();

  BitVector *members = dom->new_node_set();
  SuperGraphRegion *sgr = new SuperGraphRegion(0,
					       this, 
					       0,
					       *members,
					       entries,
					       exits);
  _top_region = sgr;
  // for the time being all members are in this one.
  for (size_t i = 0; i < the_graph->max_num_nodes(); i++) {
    _region_map->push_back(_top_region);
  }
}

SuperGraphRegion *SuperGraph::top_region() const {
  return(_top_region);
}

SuperGraphRegion *SuperGraph::find_region(SGraphNode node) const {
  return((*_region_map)[node]);
}


SGraphNode SuperGraph::max_num_nodes() const {
  return _graph->max_num_nodes();
}
bool SuperGraph::is_node_member(SGraphNode node) const {
  return(_graph->is_node_member(node));
}
void SuperGraph::add_node(SGraphNode node) {
  _graph->add_node(node);
}
void SuperGraph::remove_node(SGraphNode node) {
  _graph->remove_node(node);
}

bool SuperGraph::is_edge_member(const SGraphEdge &edge) const {
  return (_graph->is_edge_member(edge));
}
void SuperGraph::add_edge(const SGraphEdge &edge) {
  _graph->add_edge(edge);
}
void SuperGraph::remove_edge(const SGraphEdge &edge) {
  _graph->remove_edge(edge);
}

SNodeIter SuperGraph::get_node_iterator() const {
  return(_graph->get_node_iterator());
}
SNodeIter SuperGraph::get_node_successor_iterator(SGraphNode node) const {
  return(_graph->get_node_successor_iterator(node));
}
SNodeIter SuperGraph::get_node_predecessor_iterator(SGraphNode node) const {
  return(_graph->get_node_predecessor_iterator(node));
}





SuperGraphRegion::SuperGraphRegion(int region_id,
				   SuperGraph *parent, 
				   SuperGraphRegion *parent_region,
				   const BitVector &members,
				   SGraphNodeList *entries,
				   SGraphNodeList *exits) :
  _parent(parent),
  _regular_loop(false),
  _irregular_loop(false),
  _parent_region(parent_region),
  _children_regions(new list<SuperGraphRegion*>),
  _members(new BitVector(members)),
  _child_members(new BitVector),
  _backedge_nodes(new SGraphNodeList),
  _entries(entries),
  _exits(exits),
  _region_id(region_id)
{
  
}

SGraphNode SuperGraphRegion::get_entry() const {
  suif_assert(_entries->size() == 1);
  return(_entries->front());
}

SGraphNode SuperGraphRegion::get_exit() const {
  suif_assert(_exits->size() == 1);
  return(_exits->front());
}

SNodeIter SuperGraphRegion::get_exit_iterator() const {
  return(new SGraphNodeListIter(_exits, false));
}
SNodeIter SuperGraphRegion::get_entry_iterator() const {
  return(new SGraphNodeListIter(_entries, false));
}

SNodeIter SuperGraphRegion::get_proper_member_iterator() const {
  BitVector *bits = new BitVector(*_members);
  //  BitVector not_child(*_child_members);
  BitVector not_child = _child_members->invert();
  
  (*bits) &= not_child;
  return(SNodeIter(new SGraphBitIter(bits, true)));
}


bool SuperGraphRegion::is_regular_loop() const {
  return(_regular_loop);
}
bool SuperGraphRegion::is_irregular_loop() const {
  return(_irregular_loop);
}
bool SuperGraphRegion::is_loop() const {
  return(is_irregular_loop() || is_regular_loop());
}

bool SuperGraphRegion::is_loop_header(SGraphNode node) const {
  return(is_regular_loop() && _loop_header == node);
}

bool SuperGraphRegion::is_backedge(SGraphNode node) const {
  for (SGraphNodeList::iterator iter = _backedge_nodes->begin();
       iter != _backedge_nodes->end(); iter++) {
    if ((*iter) == node) return(true);
  }
  return(false);
}

bool SuperGraphRegion::is_exit(SGraphNode node) const {
  for (SGraphNodeList::iterator iter = _exits->begin();
       iter != _exits->end(); iter++) {
    if ((*iter) == node) return(true);
  }
  return(false);
}

bool SuperGraphRegion::is_member(SGraphNode node) const {
  return(_members->get_bit(node));
}
bool SuperGraphRegion::is_child_member(SGraphNode node) const {
  return(_child_members->get_bit(node));
}
bool SuperGraphRegion::is_proper_member(SGraphNode node) const {
  return(is_member(node) && ! is_child_member(node));
}

SuperGraphRegion *SuperGraphRegion::get_parent_region() const {
  return(_parent_region);
}

SuperGraph *SuperGraphRegion::get_super_graph() const {
  return(_parent);
}

SGraphNodeList *SuperGraphRegion::create_reverse_postorder_list() const {
  SGraphNodeList *rpl =
    build_reverse_postorder_list(get_super_graph(),
				 _entries, true);
  SGraphNodeList::iterator iter = rpl->begin();
  while (iter != rpl->end()) {
    SGraphNode node = *iter;
    if (!is_proper_member(node)) {
      iter = rpl->erase(iter);
      continue;
    }
    iter++;
  }
  return(rpl);
}

extern "C" void init_super_graph(SuifEnv *s) {
  s->require_module("sgraph");
} 

