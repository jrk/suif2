/*
 * main_window.h
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "base_viewer.h"

extern char *application_help_text;

/*----------------------------------------------------------------------
 * main window
 *
 */
class main_window : public base_viewer {
  typedef base_viewer inherited;
  
  vmenu*      menu;
  vframe*     text_frame;
  vtext*      text;
  vframe*     button_frame;
  vbuttonbar* button_bar;

  int         current_argc;
  char**      current_argv;

  bool        fileset_modified;

  static main_window* mainwin;

  static void do_open_cmd(        event& e, main_window* win );
  static void do_save_cmd(        event& e, main_window* win );
  static void do_reload_cmd(      event& e, main_window* win );
  static void do_exit_cmd(        event& e, main_window* win );
  static void do_help_cmd(        event& e, main_window* win );
  static void do_show_window_cmd( event& e, main_window* win,
				    window_class* wclass );

  enum fileset_state { yes=0, no=1, cancel=2 };
public:
  main_window();
  virtual ~main_window();

  virtual char* class_name() { return "Main Window"; }
  virtual void create_window( int argc = 0, char *argv[] = 0 );
  virtual void load_fileset( const String file_name );
  virtual void load_fileset() {}
  virtual fileset_state save_and_delete_fileset( bool cancel_button = false );
  virtual void update_display();
  virtual void handle_event( event& e);
  virtual void destroy();
  virtual void destroyed();
  static main_window* get_main_window();
};

#endif

