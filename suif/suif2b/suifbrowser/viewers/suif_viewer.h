/*--------------------------------------------------------------------
 * suif_viewer.h
 *
 * The "suif_viewer" displays SUIF codes of a procedure.
 *
 */

#ifndef SUIF_VIEWER_H
#define SUIF_VIEWER_H

//#include <pyg.h>
#include "base_viewer.h"
#include "code_tree.h"

bool tag_prefix_match(LString tag, char *prefix);
typedef list<LString> string_list;

/*----------------------------------------------------------------------
 * suif_viewer
 */
class suif_viewer : public text_base_viewer {
  typedef text_base_viewer inherited;

  bool show_mark_instructions;
  string_list* annotation_names;

  vmessage* info_bar;
  ProcedureDefinition* current_proc;

  struct {
    char* tag;
  } find_info;

  /* menu */
  virtual void create_proc_menu();
  virtual void create_edit_menu();
  virtual void create_view_menu();
  virtual void create_find_menu();

  /* display */
  virtual void clear();
  virtual void update_info_bar();


  /* find */
  void find_helper( void *client_data, bool search_forward );

  virtual void show_node( SuifObject* tn );

  virtual void select_node( vnode* );
 
  static void find_node_cmd( event& e, suif_viewer* viewer, char* tag );
  static void find_instr_cmd( event& e, suif_viewer* viewer );
  
  static void find_cmd( const event&, suif_viewer* viewer,
			    void* search_forward );

  static void collapse_all_cmd(event& e, suif_viewer* viewer);
  static void expand_all_cmd(event& e, suif_viewer* viewer);
  static void filter_mrk_cmd(event& e, suif_viewer* viewer);
public:
  suif_viewer();
  virtual ~suif_viewer();

  virtual void create_window();
  virtual char *class_name() { return "Suif Viewer"; }
  virtual void handle_event( event& e );

  virtual void select( code_fragment* f );
  virtual void select( SuifObject* );

  virtual void view( code_fragment* f );
  virtual void view( SuifObject* );

  virtual void refresh();
  virtual void refresh_menu();

  static window *constructor() {
    return new suif_viewer;
  }
};

#endif
