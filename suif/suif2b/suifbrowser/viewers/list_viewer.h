/*--------------------------------------------------------------------
 * list_viewer.h
 *
 */

#ifndef LIST_VIEWER_H
#define LIST_VIEWER_H

#include "base_viewer.h"

/*
 * Procedure list window
 */
class proc_list : public list_base_viewer {
  typedef list_base_viewer inherited;

protected:
  virtual void init();

public:
  proc_list( char* text );

  virtual void create_window();

  virtual char* class_name();
  virtual void handle_event( event& e );

  static window* constructor();
};

#endif


