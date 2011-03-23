
#ifndef _SUIFLINK_SUIFCOUNTER_H_
#define _SUIFLINK_SUIFCOUNTER_H_

/** @file
  * Defines class SuifCounter and NodeCounter.
  */

#include "common/suif_map.h"
#include "common/suif_vector.h"
#include "iokernel/meta_class.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/all_walker.h"
#include <iostream.h>

class NodeCounter;

/** This class helps SuifCounter in counting instances for each subclass.
  * @internal
  */
class NodeCounter {
private:
  const MetaClass * const _meta_class;
  NodeCounter *_parent;
  suif_vector<NodeCounter*> _children;
  unsigned _direct_count;
  void set_parent(NodeCounter*);
public:
  NodeCounter(const MetaClass*);
  ~NodeCounter(void);
  MetaClass* get_meta_class(void) const;
  unsigned get_direct_count(void) const;
  unsigned get_indirect_count(void) const;
  void add_child(NodeCounter*);
  unsigned get_child_count(void) const;
  NodeCounter* get_nth_child(unsigned) const;
  unsigned add_direct_count(void);
};



/** This class help count the number of object instances for each subclass.
  *
  * This class works by creating a tree of NodeCounter, mirroring the class
  * tree of the SuifObject class family.
  * Each NodeCounter store the number of instances of a particular subclass.
  */
class SuifCounter : public AllWalker {
 private:
  NodeCounter *_root_counter;
  suif_map<const MetaClass*, NodeCounter*> _NodeCounter_map;
  bool make_ancestors(NodeCounter*);
  NodeCounter *add_node_counter(const MetaClass*);

 public:
  /** Constructor. */
  SuifCounter(SuifEnv*);

  /** This method does the actual work.
    * @see Walker
    */
  Walker::ApplyStatus operator()(SuifObject*);

  /** Count the number of instances reachable from the object \a obj.
    */
  void count(SuifObject* obj);

  /** @return the NodeCounter for the subclass in \a mc.
    */
  NodeCounter* get_node_counter(const MetaClass* mc);

  /** Print the result to \a out.
    */
  ostream& print_result(ostream&) const;

};


#endif // _SUIFLINK_SUIFCOUNTER_H_

