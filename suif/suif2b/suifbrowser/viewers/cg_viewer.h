/*--------------------------------------------------------------------
 * cg_viewer.h
 *
 * The "cg_viewer" class displays the call graph of the current file set.
 *
 */

#ifndef CG_VIEWER_H
#define CG_VIEWER_H

#include "base_viewer.h"
#include "sbrowser_cg/sbrowser_cg.h"

class cg_viewer : public graph_base_viewer {
  typedef graph_base_viewer inherited;

  int     num_nodes;
  gnode** nodes;
  bool    show_external_procs;
  cg*     current_cg;

  void show_cg( cg* callgraph );

  virtual void show( vnode* vn );
  virtual void create_proc_menu();
  virtual void create_view_menu();

  static void do_show_external_procs( event& e, cg_viewer* viewer );

public:
  cg_viewer();
  ~cg_viewer();

  virtual char* class_name(void) { return "Callgraph Viewer"; }
  virtual void handle_event(event &e);

  virtual void create_window();
 
  virtual void clear();
  virtual void init();

  virtual void select(ProcedureDefinition* proc);

  virtual void set_binding( binding* b );

  static window* constructor() {
    return new cg_viewer;
  }

};

#endif
