/*--------------------------------------------------------------------
 * src_viewer.h
 *
 * The "src_viewer" displays source code of the currently selected object.
 */

#ifndef SRC_VIEWER_H
#define SRC_VIEWER_H

#include "base_viewer.h"
#include "code_tree.h"

class src_viewer : public text_base_viewer {
  typedef text_base_viewer inherited;

  FileBlock* current_file_block;
  String current_file_name;
  vmessage* infobar;
  column_id annote_column;
  code_tree* stree;
  list<code_tree*>* stree_cache;

  static const String no_source_file;

  virtual void create_file_menu();
  virtual void update_infobar();

  virtual void clear();
  virtual void refresh();

  virtual void clear_cache();
  virtual void build_stree();

  virtual bool print_source();
  virtual void show( vnode *vn );

  virtual void annotate_src();
  void annotate_src_helper(code_fragment* f );

  static void create_src_links(SuifObject* z, src_viewer* viewer );
  static code_range map_tree_node(SuifObject* z, src_viewer* viewer );
  static void do_close( event& e, src_viewer* viewer );

public:
  src_viewer();
  virtual ~src_viewer();

  virtual void create_window();
  virtual char* class_name() { return "Source Viewer"; }
  virtual void handle_event( event& e );

  virtual String get_source_file_name() {
    return current_file_name;
  }

  virtual void view(FileBlock* file, String = String());
  virtual void view(SuifObject* tn, bool select = true);

  static window* constructor() {
    return new src_viewer;
  }

};

#endif // SRC_VIEWER_H
