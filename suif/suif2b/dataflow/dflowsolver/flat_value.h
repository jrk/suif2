/*  Flat value */

/*  Copyright (c) 1999 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef FLAT_VALUE_H
#define FLAT_VALUE_H


#include "super_graph/super_graph_forwarders.h"
#include "sgraph/sgraph_forwarders.h"
#include "ion/ion_forwarders.h"
#include "region_value.h"

/*
 * this value implements all of the non-meet functions
 * as a meet.  Many dataflow problems like this.
 */

//
// NULL is DEFINED as the TOP value.
//

//

// These are garbage collected.
class FValue : public SDValue { // for super_dataflow_value
 public:
  
  FValue();
  virtual ~FValue();
  virtual SDValue *clone() const = 0; // make a pure copy of this value.
  virtual void print(ion *the_ion) const = 0;
  virtual SDValue &assign(const SDValue &val) = 0;
  
  // least upper bound meet in place.
  // If possibly changed, return true
  // this value will be in the same lattice
  virtual bool lub_meet(const SDValue &src) = 0;

  // The following are implemented as lub_meet
  virtual bool irregular_widen(const SDValue &src); 
  virtual bool kleene(const SDValue &src);
  virtual bool enter_sub_region(SuperGraphRegion *parent_region,
			SuperGraphRegion *sub_region,
			const SDValue &sub_region_src);
  virtual bool exit_sub_region(SuperGraphRegion *sub_region,
		       SuperGraphRegion *parent_region,
		       const SDValue &sub_region_src);


  virtual bool lt_eq_defined() const = 0;
  virtual bool lt_eq(const SDValue &src) const = 0;
  virtual bool eq_defined() const = 0;
  virtual bool eq(const SDValue &src) const = 0;


 private:
  // This should have NO implementation
  //  FValue(const FValue &val); 
};
  


#endif /* FLAT_VALUE_H */
