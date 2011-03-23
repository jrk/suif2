/*  Super Graph Region Dataflow Solver */

/*  Copyright (c) 1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef REGION_SOLVER_H
#define REGION_SOLVER_H


#include "super_graph/super_graph_forwarders.h"
#include "sgraph/sgraph_forwarders.h"
#include "ion/ion_forwarders.h"
#include "dflowsolver_forwarders.h"

  

class SDWork {
  bool _is_changed; // only dependant on the exits.
  SuperGraphRegion *_region;
  SGraphNodeList *_worklist; // work list of non-entry, non exit
  list<SuperGraphRegion*> *_subregion_work; // sub-regions to do
  SGraphNodeList *_backedge_work;   // list of backedges that need work.
  SGraphNodeList *_region_exit_work;   // list of this regions exits that 
  // need work
 public:
  SDWork(SuperGraphRegion *region); 
  ~SDWork();

  bool empty_worklist() const;
  void add_work(SGraphNode node, bool is_back);
  SGraphNode pop_work(bool is_back);

  // Add all of the successors to the worklist
  void add_successor_work(SGraphNode node, bool is_back);
  
  bool empty_exit_work() const;
  void add_exit_work(SGraphNode node);
  SGraphNode pop_exit_work();

  bool empty_subregion_work() const;
  void add_subregion_work(SuperGraphRegion *region);
  SuperGraphRegion *pop_subregion_work();

  bool empty_backedge_work() const;
  void add_backedge_work(SGraphNode node);
  SGraphNode pop_backedge_work();

private:
    /* override stupid defaults, don't implement these */
    SDWork &operator=(const SDWork &);
    SDWork(const SDWork &);
};  
  

class SDSolver {
  SuperGraph *_graph;
  typedef suif_vector<SDValue *> sd_value_vector;

  sd_value_vector *_node_in_value; // value at node input
  sd_value_vector *_node_out_value; // value at node output

  // stuff for the workflow algorithm
  bool  _is_changed;
  suif_vector<SDWork*> *_workstack;

 public:
  
  SDSolver(SuperGraph *graph);
  virtual ~SDSolver();
  
  SuperGraph *get_graph() const;
  // initialize the in and out values to the TOP value.
  // Frequently we use NULL for TOP. Saves space
  // but requires more complex solver functions.

  void init_solver_to_top(); // all nodes set to top.
  void init_solver(const SDValue &top_value);

  // initialize the entries to the specified region
  // to the init_value
  void init_region_entries(SuperGraphRegion *region,
			   const SDValue &init_value); 

  // region with the given value.
  void init_entries(const SDValue &init_value); // Initialize entry to the top region

  // iterate region and sub-regions until solved.
  // After iteration, we may need to iterate on the outer
  // Return true if the exit values have changed.
  virtual bool solve_region(SuperGraphRegion *region);

  /**
   * for all predecessors except back-edges,
   * merge the inputs to generate the in value of this node
   * return bool if the value changed.
   * If the previous node is the loop backedge in a nartural
   * loop, then
   *  1) merge the non-backedge inputs
   *  2) apply kleene
   */
  virtual bool merge_input_values(SGraphNode node);
  
  virtual bool apply_transfer(SGraphNode) = 0;
  virtual bool apply_region_exit_transfer(SGraphNode) = 0;
  virtual bool apply_subregion_entry_transfer(SuperGraphRegion *) = 0;

  virtual void print(ion *the_ion);

  // Access to the data
  const SDValue &get_out_val(SGraphNode node) const;
  const SDValue &get_in_val(SGraphNode node) const;

  // perform a lub meet on the current value and the new value
  bool lub_meet_out_val(SGraphNode node, const SDValue &val);
  bool lub_meet_in_val(SGraphNode node, const SDValue &val);

  SDValue &get_mod_out_val(SGraphNode node);
  SDValue &get_mod_in_val(SGraphNode node);

  bool is_in_val_top(SGraphNode node) const;
  bool is_out_val_top(SGraphNode node) const;

  void set_in_val_top(SGraphNode node);
  void set_out_val_top(SGraphNode node);

  // copy 
  void set_in_val(SGraphNode node, const SDValue &val);
  void set_out_val(SGraphNode node, const SDValue &val);

private:
    // override stupid defaults, don't implement these
    SDSolver &operator=(const SDSolver &);
    SDSolver(const SDSolver &);
};


#endif /* REGION_SOLVER_H */
