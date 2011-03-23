// $Id: printing_maps.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFPRINT__PRINTING_MAPS_H
#define OSUIFPRINT__PRINTING_MAPS_H

#include <iostream.h>

#include "iokernel/iokernel_forwarders.h"
#include "suifkernel/visitor_map.h"


/**
 * The PrintingMaps class has two VisitorMaps: process map and children map.
 * The classes which want pretty printing on its objects should register
 * with the process map. A default method is registered for the
 * SuifObject, this method will be called if the class does not define its
 * own custom method.
 */
class PrintingMaps {
private:
  LString _printing_maps_name;
  bool _done;
  int indent;

  VisitorMap* _process_map;
  VisitorMap* _children_map;

public:
  PrintingMaps(SuifEnv *suif_env, const LString &printing_maps_name);

  /// Initializes the default implementation for the SuifObject.
  void init_suif_object();

  /// The printing of the tree is entered with this function
  void process_a_suif_object(SuifObject *so);

  // Accessors
  VisitorMap* get_children_map() const { return _children_map; }
  VisitorMap* get_process_map() { return _process_map; }
  LString get_printing_maps_name() const { return _printing_maps_name; }
  int getIndent() const { return indent; }

  //void set_done(bool b) { _done = b; }

  // Register methods
  void register_children_visit_method( Address state,
				       VisitMethod visitMethod, 
				       const LString &name );
  void register_process_visit_method( Address state,
				      VisitMethod visitMethod, 
				      const LString &name );

  void incrementIndent();
  void decrementIndent();

  /// Test for early exit.
  bool is_done() const { return _done; }

private:
  PrintingMaps(const PrintingMaps &);
  PrintingMaps& operator=(const PrintingMaps &);
  
};

#endif /* OSUIFPRINT__PRINTING_MAPS_H */
