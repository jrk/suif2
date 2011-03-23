/*--------------------------------------------------------------------
 * text_viewer.h
 *
 */

#ifndef TEXT_VIEWER_H
#define TEXT_VIEWER_H

#include "base_viewer.h"

/*
 * text viewer
 */
class text_viewer : public text_base_viewer {
  typedef text_base_viewer inherited;

private:
  static void do_open(event& e, text_viewer* viewer );
  static void do_close(event& e, text_viewer* viewer );

public:
  text_viewer();
  virtual ~text_viewer();

  virtual void create_window();
  virtual char *class_name() { return "Text Viewer"; }

  void open( char* filename );
  void clear();
  void insert_text( char* str );

  static window *constructor() {
    return new text_viewer;
  }
};

#endif
