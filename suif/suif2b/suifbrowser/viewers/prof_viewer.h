/*--------------------------------------------------------------------
 * prof_viewer.h
 *
 */

#ifndef PROF_VIEWER_H
#define PROF_VIEWER_H

//@@@ #include "profile.h"
#include "base_viewer.h"

/*
 * profile viewer
 */
class prof_viewer : public text_base_viewer {
  typedef text_base_viewer inherited;

  //@@@ profile_table *p_table;

  virtual void init();
  virtual void print_profile();
  virtual void show( vnode* vn );
  virtual void clear();

  static void do_open_runtime_file( event& e, prof_viewer* viewer );

public:
  prof_viewer();
  virtual ~prof_viewer();

  virtual void create_window();

  virtual char* class_name() { return "Profile Viewer"; }
  virtual void handle_event(event &e);

  static window *constructor() {
    return new prof_viewer;
  }
};

#endif
