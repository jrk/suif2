#include "region_value.h"

/**
 *
 * SDValue 
 * 
 *  a class that encodes some portion of the 
 *  lattice of values for the region Solver.
 */
SDValue::SDValue() {}
SDValue::~SDValue() {}
SDValue &SDValue::operator=(const SDValue &other) {
  // dispatch
  return(assign(other));
}
