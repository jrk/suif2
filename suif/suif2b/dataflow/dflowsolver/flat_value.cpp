#include "flat_value.h"

FValue::FValue() {}
FValue::~FValue() {}


bool FValue::irregular_widen(const SDValue &src) { 
  return(lub_meet(src)); 
}
bool FValue::kleene(const SDValue &src) { 
  return(lub_meet(src));
}

bool FValue::enter_sub_region(SuperGraphRegion *parent_region,
			      SuperGraphRegion *sub_region,
			      const SDValue &region_src) {
  return(lub_meet(region_src));
}
bool FValue::exit_sub_region(SuperGraphRegion *sub_region,
			     SuperGraphRegion *parent_region,
			     const SDValue &sub_region_src) {
  return(lub_meet(sub_region_src));
}


