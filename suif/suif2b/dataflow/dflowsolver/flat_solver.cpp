#include "flat_solver.h"
#include "common/suif_vector.h"
#include "ion/ion.h"
#include "bit_vector/bit_vector.h"
#include "super_graph/super_graph.h"
#include "sgraph/sgraph.h"
#include "sgraph/sgraph_iter.h"

FSolver::FSolver(SuperGraph *graph) :
  SDSolver(graph)
{}
FSolver::~FSolver() {}

bool FSolver::apply_region_exit_transfer(SGraphNode node) {
  return(apply_transfer(node));
}

bool FSolver::
apply_subregion_entry_transfer(SuperGraphRegion *sub_region) {
  bool changed = false;
  for (SNodeIter iter(sub_region->get_entry_iterator());
       !iter.done(); iter.increment()) {
    SGraphNode node = iter.get();
    if (merge_input_values(node)) {
      if (apply_transfer(node)) {
	changed = true;
      }
    }
  }
  return(changed);
}


 
