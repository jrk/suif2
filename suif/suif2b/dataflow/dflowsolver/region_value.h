/*  Super Graph Region Dataflow Solver */

/*  Copyright (c) 1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef REGION_VALUE_H
#define REGION_VALUE_H


#include "super_graph/super_graph_forwarders.h"
#include "sgraph/sgraph_forwarders.h"
#include "ion/ion_forwarders.h"


//
// NULL is DEFINED as the TOP value.
//

//
// The graph we are given has special features:
//   1) there is an entry and (possibly multiple) exit node(s) for each region.
//      The entry has one predecesssor in the parent region,
//      The exits each have one successor in the parent region.
//      Of course the top region has no parent region..
// 
//   2) there is a backedge node at loop backedges:
//      It has one successor - a loop header.
//      all natural loop backedges will go through this node.
//
// A loop dataflow lattice has a number of operators:
//   1) value = meet(value, value);
//   2) bool is_less_than(value, value)
//   4) value = kleene(value, value) 
//   5) a BOTTOM element. less than any other value.
// 
//  The following are also defined.  Beware: the lattice
//   can change for these
//   6) value = enter_sub_region(region, sub_region, value)
//   7) value = exit_sub_region(sub_region, region, value)
//
// For a standard dataflow problem kleene(), enter(), and exit() will
//    just be meet.
//
// To make everyone's life easier, however, we implement these
//    in the "super_dataflow_value" class instead of in a lattice.
//
// Because of this choice, for any value that changes its lattice
//    we probably need to keep the owning region in it.
//   

// These are garbage collected.
class SDValue { // for super_dataflow_value
 public:
  
  SDValue();
  virtual ~SDValue();
  virtual SDValue *clone() const = 0; // make a pure copy of this value.
  virtual void print(ion *the_ion) const = 0;
  virtual SDValue &assign(const SDValue &val) = 0;
  
  SDValue &operator=(const SDValue &val);

  
  
  // least upper bound meet in place.
  // If possibly changed, return true
  // this value will be in the same lattice
  virtual bool lub_meet(const SDValue &src) = 0;

  // Widening required to break up 
  // irregular loops. 
  // If possibly changed, return true
  virtual bool irregular_widen(const SDValue &src) = 0;

   // this value will be at the head of a loop
  virtual bool kleene(const SDValue &src) = 0;

  // The value will be in the sub_region lattice
  virtual bool enter_sub_region(SuperGraphRegion *parent_region,
			SuperGraphRegion *sub_region,
			const SDValue &sub_region_src) = 0;  
  // The value will be in the parent region lattice
  virtual bool exit_sub_region(SuperGraphRegion *sub_region,
		       SuperGraphRegion *parent_region,
		       const SDValue &sub_region_src) = 0; 


  // less that or equal to
  // They MUST be in comparable lattices.
  virtual bool lt_eq_defined() const = 0;
  virtual bool lt_eq(const SDValue &src) const = 0;

  // equal to
  // This is harder.  They must be in comparable
  // lattices with equivalaence defined.
  // Not all lattices will have this property.
  // Using a solver that requires it on a value
  // that does not provide it is a no-no.
  virtual bool eq_defined() const = 0;
  virtual bool eq(const SDValue &src) const = 0;


 private:
  // This should have NO implementation
  SDValue(const SDValue &val); 
};
  


#endif /* REGION_VALUE_H */
