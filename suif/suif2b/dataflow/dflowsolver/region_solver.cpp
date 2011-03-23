#include "region_solver.h"
#include "super_graph/super_graph.h"
#include "sgraph/sgraph.h"
#include "sgraph/sgraph_iter.h"
#include "common/suif_vector.h"
#include "ion/ion.h"
#include "bit_vector/bit_vector.h"
#include "region_value.h"
#include "suifkernel/suif_env.h"

SDWork::SDWork(SuperGraphRegion *region) :
  _is_changed(false),
  _region(region),
  _worklist(new SGraphNodeList()),
  _subregion_work(new list<SuperGraphRegion*>()),
  _backedge_work(new SGraphNodeList()),
  _region_exit_work(new SGraphNodeList())
{}

SDWork::~SDWork() {
  // everything SHOULD be empty.
  assert(_worklist->empty());
  assert(_subregion_work->empty());
  assert(_backedge_work->empty());
  assert(_region_exit_work->empty());

  delete _worklist;
  delete _subregion_work;
  delete _backedge_work;
  delete _region_exit_work;

}


bool SDWork::empty_worklist() const { return(_worklist->empty()); }
void SDWork::add_work(SGraphNode node, bool is_back) {
  for (list<SGraphNode>::iterator iter = _worklist->begin();
       iter != _worklist->end(); iter++) {
    if ((*iter) == node) return;
  }
  if (is_back)
    _worklist->push_back(node);
  else
    _worklist->push_front(node);
}
SGraphNode SDWork::pop_work(bool is_back) {
  if (is_back) {
    SGraphNode node = _worklist->back();
    _worklist->pop_back();
    return(node);
  } 
  SGraphNode node = _worklist->front();
  _worklist->pop_front();
  return(node);
}

  // Add all of the successors to the worklist
void SDWork::add_successor_work(SGraphNode from_node, bool is_back) {
  SGraph *gr = _region->get_super_graph();
  for (SNodeIter iter(gr->get_node_successor_iterator(from_node));
       !iter.done(); iter.increment()) {
    SGraphNode to_node = iter.get();
    // Cases:
    // 1) subregion node
    // 2) backedges in a natural loop.
    // 3) backedge/crossedge in an irregular loop.
    // 4) exit
    // 5) normal
    SuperGraphRegion *to_region =
      _region->get_super_graph()->find_region(from_node);
    if (to_region != _region) {
      assert(to_region->get_parent_region() == _region);
      add_subregion_work(to_region);
      continue;
    }
    if (_region->is_regular_loop() &&
	_region->is_backedge(to_node)) {
      add_backedge_work(to_node);
      continue;
    }
    if (_region->is_exit(from_node)) {
      add_exit_work(from_node);
      continue;
    }
    add_work(to_node, is_back);
  }
}
  
bool SDWork::empty_exit_work() const { return(_region_exit_work->empty()); }
void SDWork::add_exit_work(SGraphNode node) {
  for (list<SGraphNode>::iterator iter = _region_exit_work->begin();
       iter != _region_exit_work->end(); iter++) {
    if ((*iter) == node) return;
  }
  _region_exit_work->push_back(node);
}
SGraphNode SDWork::pop_exit_work() {
  SGraphNode node = _region_exit_work->back();
  _region_exit_work->pop_back();
  return(node);
}

bool SDWork::empty_subregion_work() const { return(_subregion_work->empty()); }
void SDWork::add_subregion_work(SuperGraphRegion *region){
  for (list<SuperGraphRegion*>::iterator iter = _subregion_work->begin();
       iter != _subregion_work->end(); iter++) {
    if ((*iter) == region) return;
  }
  _subregion_work->push_back(region);
}
SuperGraphRegion *SDWork::pop_subregion_work() {
  SuperGraphRegion *node = _subregion_work->back();
  _subregion_work->pop_back();
  return(node);
}

bool SDWork::empty_backedge_work() const { return(_backedge_work->empty()); }
void SDWork::add_backedge_work(SGraphNode node) {
  for (list<SGraphNode>::iterator iter = _backedge_work->begin();
       iter != _backedge_work->end(); iter++) {
    if ((*iter) == node) return;
  }
  _backedge_work->push_back(node);
}
SGraphNode SDWork::pop_backedge_work() {
  SGraphNode node = _backedge_work->back();
  _backedge_work->pop_back();
  return(node);
}


/*
 * class SDSolver 
 * 
 * an abstract class that implements
 * a recursive solution algorithm to dataflow
 * equations over a region tree.
 */
SDSolver::SDSolver(SuperGraph *graph) :
  _graph(graph),
  _node_in_value(new sd_value_vector()),
  _node_out_value(new sd_value_vector()),
  _is_changed(false),
  _workstack(new suif_vector<SDWork*>)
{}

SDSolver::~SDSolver() {
  // remove the stuff first?  Does this own the values?
  delete _node_in_value;
  delete _node_out_value;
  delete _workstack;
}
SuperGraph *SDSolver::get_graph() const {
  return(_graph);
}

//
// This is the top-level driver.
//
bool SDSolver::solve_region(SuperGraphRegion *region) {
  // Here is the algorithm for solve region
  //
  // solve_region
  //   _worklist->push(new work_list)
  //   _kleeneworklist->push(new work_list)
  //   _workstack->push(this);
  //   current_worklist = _worklist->back()
  //   current_kleenelist = _kleeneworklist->back()
  //   current_worklist->push(region->entry());
  //   while (!current_worklist.empty()) {
  //       while (!current_worklist.empty()) {
  //          while (!current_worklist.empty()) {
  //              if (apply_to_node(current_worklist->pop())) {
  //                 current_worklist->append(node_successors(node));
  //              }
  //          }
  //          if (!subregionlist.empty()) {
  //             enterlist.pop()->enter_sub_region(region, region->parent());
  //             solve_region(subregionlist.pop());
  //          }
  //       }
  //       if (!kleeneworklist.empty()) {
  //          may add to worklist
  //          apply_kleene(kleeneworklist->pop());
  //       }
  //   }
  //   while (!_exitlist.empty()) {
  //      exitlist.pop()->exit_sub_region(region, region->parent());
  //   }

  // first propagate any work from the outer region

  SDWork *work = new SDWork(region);
  _workstack->push_back(work);
  //  SGraphNode entry_node = region->get_entry();
  //  work->add_work(entry_node, true);

  // we really need just a special case for the first
  //  time in
  // should add all reachable here in reverse postorder
  SGraphNodeList list;
  // TBD @@@ change this to a reverse postorder traversal
  // instead of numerical order.
  // Build the reverse postorder on the whole graph and
  // only add the ones that are in this region.
  // It's a little wasteful, but at least the iteration is
  // in the right order.

  
  SGraphNodeList *rpl = region->create_reverse_postorder_list();
  for (SGraphNodeList::iterator iter = rpl->begin();
       iter != rpl->end(); iter++) {
    SGraphNode node = *iter;
    work->add_work(node, false);
  }
  //delete rpl;

  //  for (SNodeIter iter(region->get_proper_member_iterator());
  //       !iter.done(); iter.increment()) {
  //    work->add_work(iter.get());
  //  }

  {for (SNodeIter iter(region->get_proper_member_iterator());
       !iter.done(); iter.increment()) {
    work->add_work(iter.get(), true);
  }}

  BitVector touched;

  while (!work->empty_worklist()) { // backedges
    while (!work->empty_worklist()) { // inners
      while (!work->empty_worklist()) { // worklist
	SGraphNode node = work->pop_work(true);
	// first we need to merge the incoming values into the
	// input value.
	if (merge_input_values(node) ||
	    !touched.get_bit(node)) {
	  touched.set_bit(node, true);
	  if (apply_transfer(node)) {
	    work->add_successor_work(node, false);
	  }
	}
      }
      if (!work->empty_subregion_work()) {
	SuperGraphRegion *region = work->pop_subregion_work();
	if (apply_subregion_entry_transfer(region)) {
	  solve_region(region); 
	  // to add some work to this region if a value changes
	}
      }
    }
    if (!work->empty_backedge_work()) {
      SGraphNode node = work->pop_backedge_work();
      if (apply_transfer(node)) {
	SNodeIter iter(region->get_super_graph()->get_node_successor_iterator(node));
	// this MUST have only 1 successor -- the loop header.
	assert(!iter.done());
	SGraphNode header_node = iter.get();
	assert(iter.done());
	assert(region->is_loop_header(header_node));
      }
    }
  }
  bool changed = false;
  while (!work->empty_exit_work()) {
    SGraphNode node = work->pop_exit_work();
    if (apply_region_exit_transfer(node)) {
      changed = true;
    }
  }
  // This value is pretty much ignored anyway.
  delete _workstack->back();
  _workstack->pop_back();
  return(changed);
}

bool SDSolver::merge_input_values(SGraphNode node) {
  // default implementation find all of the previous nodes
  // that are NOT
  SDValue *new_val = NULL;
  
  SGraphNodeList backedge_list;
  SuperGraphRegion *region = _graph->find_region(node);

  for (SNodeIter iter(_graph->get_node_predecessor_iterator(node));
       !iter.done(); iter.increment()) {
    SGraphNode pred = iter.get();
    SuperGraphRegion *pred_region = _graph->find_region(pred);
    assert(pred_region == region);

    if (pred_region->is_regular_loop() &&
	pred_region->is_backedge(pred)) {
      backedge_list.push_back(pred);
    }
    
    if (new_val == NULL) {
      if (!is_out_val_top(pred)) {
	new_val = get_out_val(pred).clone();
      }
    } else {
      if (!is_out_val_top(pred)) {
	new_val->lub_meet(get_out_val(pred));
      }
    }
  }

  assert(backedge_list.size() <= 1);

  if (backedge_list.size() == 1) {
    SGraphNode backedge_node = backedge_list.front();
    if (!is_out_val_top(backedge_node)) {
      assert(new_val != NULL); // top?  needs explicit one
      new_val->kleene(get_out_val(backedge_node));
    }
  }

  if (new_val == NULL) { return(false); }
  // assert
  if (new_val->lt_eq_defined() &&
      new_val->eq_defined()) {
    if (is_in_val_top(node)) {
      set_in_val(node, *new_val);
      return(true);
    }
    assert(new_val->lt_eq(get_in_val(node)));
    if (new_val->eq(get_in_val(node))) {
      delete new_val;
      return(false);
    }
    set_in_val(node, *new_val);
    delete new_val;
    return(true);
  }
  // No choice but to meet with the current value
  if (is_in_val_top(node)) {
    set_in_val(node, *new_val);
    delete new_val;
    return(true);
  }
  bool res = get_mod_in_val(node).lub_meet(*new_val);
  delete new_val;
  return(res);
}

bool SDSolver::is_in_val_top(SGraphNode node) const {
  if (_node_in_value->size() <= node) { return(true); }
  return((*_node_in_value)[node] == NULL);
}

bool SDSolver::is_out_val_top(SGraphNode node) const {
  if (_node_out_value->size() <= node) { return(true); }
  return((*_node_out_value)[node] == NULL);
}

const SDValue &SDSolver::get_out_val(SGraphNode node) const {
  assert(!is_out_val_top(node));
  return( *((*_node_out_value)[node]));
}

const SDValue &SDSolver::get_in_val(SGraphNode node) const {
  assert(!is_in_val_top(node));
  return( *((*_node_in_value)[node]));
}

bool SDSolver::lub_meet_out_val(SGraphNode node, const SDValue &val) {
  if (is_out_val_top(node)) {
    set_out_val(node, val);
    return(true);
  }
  return(get_mod_out_val(node).lub_meet(val));
}

bool SDSolver::lub_meet_in_val(SGraphNode node, const SDValue &val) {
  if (is_in_val_top(node)) {
    set_in_val(node, val);
    return(true);
  }
  return(get_mod_in_val(node).lub_meet(val));
}


SDValue &SDSolver::get_mod_out_val(SGraphNode node) {
  assert(!is_out_val_top(node));
  return( *((*_node_out_value)[node]));
}

SDValue &SDSolver::get_mod_in_val(SGraphNode node) {
  assert(!is_in_val_top(node));
  return(* ((*_node_in_value)[node]));
}

void  SDSolver::set_in_val(SGraphNode node, const SDValue &val) {
  while (_node_in_value->size() <= node) {
    _node_in_value->push_back(NULL);
  }
  if ((*_node_in_value)[node] == NULL) {
    (*_node_in_value)[node] = val.clone();
  } else { 
    (*_node_in_value)[node]->assign(val);
  }
}

void SDSolver::set_in_val_top(SGraphNode node) {
  if (is_in_val_top(node)) return;
  (*_node_in_value)[node] = 0;
}
void SDSolver::set_out_val_top(SGraphNode node) {
  if (is_out_val_top(node)) return;
  (*_node_out_value)[node] = 0;
}

void SDSolver::print(ion *the_ion) {
  the_ion->printf("Begin Output values:\n");
  int i = 0;
  for (sd_value_vector::iterator iter = _node_out_value->begin();
       iter != _node_out_value->end(); iter++, i++) {
    if ((*iter) == NULL) {
      the_ion->printf("out[%d] => NULL\n", i);
    } else {
      the_ion->printf("out[%d] => ", i);
      (*iter)->print(the_ion);
      the_ion->printf("\n");
    }
  }
  the_ion->printf("End Output values:\n");
  the_ion->printf("Begin Input values:\n");

  i = 0;
  {for (sd_value_vector::iterator iter = _node_in_value->begin();
       iter != _node_in_value->end(); iter++, i++) {
    if ((*iter) == NULL) {
      the_ion->printf("in[%d] => NULL\n", i);
    } else {
      the_ion->printf("in[%d] => ", i);
      (*iter)->print(the_ion);
      the_ion->printf("\n");
    }
  }}
  the_ion->printf("End Input values\n");
}

void  SDSolver::set_out_val(SGraphNode node, const SDValue &val) {
  while (_node_out_value->size() <= node) {
    _node_out_value->push_back(NULL);
  }
  if ((*_node_out_value)[node] == NULL) {
    (*_node_out_value)[node] = val.clone();
  } else { 
    (*_node_out_value)[node]->assign(val);
  }
}

void SDSolver::init_region_entries(SuperGraphRegion *region,
			 const SDValue &init_value) {
  for (SNodeIter iter(region->get_entry_iterator());
       !iter.done(); iter.increment()) {
    SGraphNode node = iter.get();
    if (&init_value == NULL) {
      set_in_val_top(node);
    } else {
      set_in_val(node, init_value);
    }
  }
}

// init the ins, the outs all stay NULL.
void SDSolver::init_solver_to_top() {
  while (!_node_in_value->empty()) {
    delete _node_in_value->back();
    _node_in_value->pop_back();
  }
  while (!_node_out_value->empty()) {
    delete _node_out_value->back();
    _node_out_value->pop_back();
  }
  while (!_workstack->empty()) {
    delete _workstack->back();
    _workstack->pop_back();
  }
}
  
void SDSolver::init_solver(const SDValue &init_value) {
  init_solver_to_top();
  for (size_t i = 0; i < _graph->max_num_nodes(); i++) {
    _node_in_value->push_back(init_value.clone());
    //_node_out_val->push_back(init_value.clone());
  }
}

void SDSolver::init_entries(const SDValue &init_value) {
  init_region_entries(_graph->top_region(), init_value);
}

extern "C" void init_dflowsolver(SuifEnv *s) {
  s->require_module("super_graph");
}
 
