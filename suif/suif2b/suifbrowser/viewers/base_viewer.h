/*--------------------------------------------------------------------
 * base_viewer.h
 *
 */

#ifndef BASE_VIEWER_H
#define BASE_VIEWER_H

#include "visual/visual.h"
#include "common/suif_list.h"
#include "basicnodes/basic.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module.h"
#include "suifprinter/suifprinter.h"
extern SuifEnv* suif_env;
extern SuifPrinterModule* pr;

/*----------------------------------------------------------------------
 * base viewer
 */
class base_viewer : public window {
  typedef window inherited;

  static void do_close_command( event& e, base_viewer* viewer );

protected:
  binding* event_binding;

public:
  base_viewer();
  virtual ~base_viewer();

  virtual char* class_name() { return "Base Viewer"; }
  virtual void handle_event( event& ) {};

  virtual void add_close_command( vmenu* menu, char* parent_menu );
  virtual void add_close_button( vbuttonbar* button_bar );
};
/*----------------------------------------------------------------------
 * text viewer
 */
class text_base_viewer : public base_viewer {
  typedef base_viewer inherited;

protected:
  vframe* frame;
  vmenu*  menu;
  vtext*  text;

public:
  text_base_viewer();
  virtual ~text_base_viewer();

  virtual char* class_name() { return "Text Viewer"; }
  virtual void handle_event( event& e );

  virtual void create_window();
};
/*----------------------------------------------------------------------
 * graph base viewer
 */
class graph_base_viewer : public base_viewer {
  typedef base_viewer inherited;

protected:
  vframe* frame;
  vmenu*  menu;
  vgraph* graph_wdgt;

public:
  graph_base_viewer();
  virtual ~graph_base_viewer();

  virtual char* class_name() { return "Graph Viewer"; }

  virtual void create_window();
};

/*----------------------------------------------------------------------
 * list base viewer
 */
class list_base_viewer : public base_viewer {
  typedef base_viewer inherited;

protected:
  vframe*     listbox_frame;
  vlistbox*   listbox;
  vframe*     button_frame;
  vbuttonbar* button_bar;

  char* list_title;

public:
  list_base_viewer();
  virtual ~list_base_viewer();

  virtual void set_title( char* s );

  virtual void create_window();
  virtual char* class_name() { return "List Viewer"; }
};

/*----------------------------------------------------------------------
 * form base viewer
 */
typedef list<SuifBrick*> BrickList;
class form_base_viewer : public base_viewer {
  typedef base_viewer inherited;

protected:
  vmenu*      menu;
  vframe*     form_frame;
  vframe*     button_frame;
  vform*      form;
  vmessage*   info_bar;
  vbuttonbar* button_bar;

public:
  form_base_viewer();
  virtual ~form_base_viewer();

  virtual void create_window();
  virtual char* class_name() { return "Form Viewer"; }

  void set_info_bar( char* msg );

  virtual void add_bricks( SuifObject* obj );
  virtual void add_brick( SuifBrick *br, char* field_name );

  BrickList* get_brick_list( SuifObject* obj, int first_field_num = 0);
  SuifBrick *get_brick( SuifObject* obj, int field_num, char*& error_msg );
};

#endif  // BASE_VIEWER_H
