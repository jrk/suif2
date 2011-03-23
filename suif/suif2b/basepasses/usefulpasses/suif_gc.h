#ifndef _SUIFLINK_SUIFGC_H_
#define _SUIFLINK_SUIFGC_H_

#include "basicnodes/basic.h"

/**
  * @file
  * Defines class SuifGC.
  */

/** This class helps collect unreferenced objects from symbol tables in
  * a FileSetBlock.
  *
  * The Collector will traverse from a root object, and mark
  * the symbol table object garbage if it is not reachable
  * from the root object.
  *
  * The Collector marks a symbol table object garbage by removing
  * it from the symbol table and add it to an internal garbage buffer.
  * The garbages are deleted in the destructor.
  */
class SuifGC {
 public:

  /** Collect and delete all unreferenced objects from all symbol tables
    * in a FileSetBlock.
    */
  static void collect(FileSetBlock*);

  /** Decide if there is a path (of referenced and owned links) between
    * two objects.
    * @return true iff there is a path from \a from to \a to.
    */
  static bool is_reachable(SuifObject* from, SuifObject* to);
};

#endif // _SUIFLINK_SUIFGC_H_
