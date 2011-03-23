/*--------------------------------------------------------------------
 * info_viewer.h
 *
 * The "info_viewer" class displays the currently selected object, and
 * various information about that object.
 *
 */

#ifndef INFO_VIEWER_H
#define INFO_VIEWER_H

#include "base_viewer.h"

class info_viewer : public text_base_viewer {
  typedef text_base_viewer inherited;

  virtual void create_obj_menu();
  virtual void create_edit_menu();

  static void do_show_obj_cmd(event& e, info_viewer* viewer, vnode* vn );

public:
  info_viewer();
  ~info_viewer();

  virtual void create_window();

  virtual char* class_name() { return "Info Viewer"; }
  virtual void handle_event( event& e );

  virtual void clear();
  virtual void view( vnode* vn );
  virtual void refresh();

  inline static window* constructor() {
    return new info_viewer;
  }
};

#endif
