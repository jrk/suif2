/*  Super Graph Region Dataflow Solver */

/*  Copyright (c) 1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef FLAT_SOLVER_H
#define FLAT_SOLVER_H


#include "super_graph/super_graph_forwarders.h"
#include "sgraph/sgraph_forwarders.h"
#include "ion/ion_forwarders.h"
#include "region_solver.h"

  

/*
 * an FSolver is an SDSolver
 * that works on a flat graph
 * i.e. region entry and exit are the same
 * we might consider using a simpler work-list algorithm here
 * this MAY but does not need to 
 * work in conjunction with an FValue
 */
class FSolver : public SDSolver {
 public:
  
  FSolver(SuperGraph *graph);
  virtual ~FSolver();
  
  virtual bool apply_transfer(SGraphNode) = 0;

  virtual bool apply_region_exit_transfer(SGraphNode);
  virtual bool apply_subregion_entry_transfer(SuperGraphRegion *);
};


#endif /* FLAT_SOLVER_H */
