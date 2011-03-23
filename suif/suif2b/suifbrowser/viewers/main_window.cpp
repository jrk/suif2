/*----------------------------------------------------------------------
 * main_window.cc
 *
 *
 */

#include "sbrowser_cg/sbrowser_cg.h"
#include "main_window.h"
#include "viewers.h"
#include "visual/visual.h"
#include "suif_utils.h"
#include "text_viewer.h"
#include "suif_event.h"
#include "suif_vnode.h"
#include "suifkernel/suif_env.h"

char *application_help_text = "No help available.";
main_window* main_window::mainwin = 0;

/*----------------------------------------------------------------------
 * main_window
 *
 */
main_window::main_window()
{
  // ** instance members **
  menu             = 0;
  text_frame       = 0;
  text             = 0;
  button_frame     = 0;
  button_bar       = 0;

  current_argc     = 0;
  current_argv     = 0;

  fileset_modified = false;

  // ** static member **
  //suif_assert_message( mainwin == 0, ("main_window is already initialized" ));
  assert(mainwin == 0);
  mainwin = this;
}

/*----------------------------------------------------------------------
 * ~main_window
 *
 */
main_window::~main_window()
{
  delete text;
  delete button_bar;
  delete text_frame;
  delete button_frame;
  delete menu;

  mainwin = 0;
}

/*----------------------------------------------------------------------
 * main_window::destroy
 *
 */
void
main_window::destroy()
{
  save_and_delete_fileset(); //@@@ move to >destroyed< ??

  inherited::destroy();
}

typedef list<window*> window_list;
/*----------------------------------------------------------------------
 * main_window::destroyed
 *
 */
void
main_window::destroyed()
{
  inherited::destroyed();

  // delete all other windows that are still open
  window_list windows;
  // window* win;
  vman->get_current_windows( windows );
  while ( !windows.empty() ) {
    windows.pop_front();
  }

  exit_viewers();
  exit_visual();
  exit_sbrowser_cg();
  exit(0);
}

/*----------------------------------------------------------------------
 * main_window::create_window
 *
 */
void
main_window::create_window(int argc, char *argv[])
{
  inherited::create_window();

  menu         = new vmenu(  toplevel );
  button_frame = new vframe( toplevel );
  text_frame   = new vframe( toplevel );
  button_bar   = new vbuttonbar( button_frame );
  text         = new vtext( text_frame );

  text->fout() << "No file set entry\n";
  text->update();

  /*
   * File menu
   */
  binding *b = new binding((bfun) &do_open_cmd, this);
  menu->add_command(b, "File", "Open File Set...");

  b = new binding((bfun) &do_reload_cmd, this);
  menu->add_command(b, "File", "Reload File Set");

  b = new binding((bfun) &do_save_cmd, this);
  menu->add_command(b, "File", "Save File Set");

  menu->add_separator("File");

  b = new binding((bfun) &do_exit_cmd, this);
  menu->add_command(b, "File", "Exit");

#ifdef CURRENTLY_NOT_SUPPORTED //@@@
  /*
   * Create list of modules in the "module" menu
   */
  module_list_iter mod_iter (vman->get_module_list());
  while (!mod_iter.is_empty()) {
    module *mod = mod_iter.step();

    char buffer[100];
    sprintf(buffer, "Module/%s", mod->class_name());
    mod->attach_menu(menu, buffer);
    mod->create_menu();
  }
#endif

  /*
   * Create list of viewers in the "window" menu
   */
  window_class_list* window_classes =  vman->get_window_classes();
  for ( s_count_t i=0; i<window_classes->size(); i++ ) {
    window_class *wclass = (*window_classes)[i];

    b = new binding((bfun2) &do_show_window_cmd, this, wclass);
    menu->add_command(b, "Windows", wclass->name());
  }

  /*
   * Help menu
   */
  b = new binding( (bfun) &do_help_cmd, this );
  menu->add_command( b, "Help", "Help!" );

  /*
   * Button bar
   */

  window_class* wclass = vman->find_window_class( "Callgraph Viewer" );
  if ( wclass ) {
    b = new binding( (bfun2) &do_show_window_cmd, this, wclass );
    button_bar->add_button( b, "Call Graph" );
  }

  wclass = vman->find_window_class( "Source Viewer" );
  if ( wclass ) {
    b = new binding( (bfun2) &do_show_window_cmd, this, wclass );
    button_bar->add_button( b, "Source" );
  }

  wclass = vman->find_window_class( "Suif Viewer" );
  if ( wclass ) {
    b = new binding( (bfun2) &do_show_window_cmd, this, wclass );
    button_bar->add_button( b, "Suif");
  }

  wclass = vman->find_window_class( "Output Viewer" );
  if ( wclass ) {
    b = new binding( (bfun2) &do_show_window_cmd, this, wclass );
    button_bar->add_button( b, "C Output" );
  }

  wclass = vman->find_window_class( "Info Viewer" );
  if (wclass) {
    b = new binding( (bfun2) &do_show_window_cmd, this, wclass );
    button_bar->add_button( b, "Info" );
  }
  if(argc == 1)
    load_fileset();
  else 
    load_fileset(argv[1]);
}

/*----------------------------------------------------------------------
 * open fileset
 *
 */
void main_window::do_open_cmd( event&, main_window* win ) {
  //@@@ old menu:  char* new_fs = select_fileset(win, "Load SUIF file(s):", 0);
  char new_fs[1000]; // @@@ problem with Tcl/Tk transfer. solution ??
  select_file( win, new_fs, "Specify Browser Input File:" );
  if ( new_fs ) {
    char **argv = new char*[100];
    argv[0] = 0;
    int argc = 1;
    for (char *p = strtok(new_fs, " "); p; p = strtok(0, " ")) {
      int len = strlen( p );
      char* name = new char[len+1];
      strcpy( name, p );
      argv[argc] = name;
      argc++;
      assert(argc < 100);
      //@@@ for old menu: delete [] new_fs;
    }
    argv[argc] = 0;
    if (argc > 1) 
      win->load_fileset(argv[1]);
    else
      win->load_fileset();
  }
}

/*----------------------------------------------------------------------
 * reload fileset
 *
 */
void
main_window::do_reload_cmd(event&, main_window* win)
{
  win->load_fileset();
}

/*----------------------------------------------------------------------
 * save fileset
 *
 */
void
main_window::do_save_cmd( event&, main_window* win )
{
  win->save_and_delete_fileset();
  win->load_fileset();

  /* broadcast fileset changed */
  post_event(event(0, NEW_FILESET));
}

/*----------------------------------------------------------------------
 * do_exit_cmd
 *
 */
void
main_window::do_exit_cmd( event&, main_window* win )
{
  win->destroy();
}

/*----------------------------------------------------------------------
 * do_help_cmd
 *
 */
void
main_window::do_help_cmd(event &, main_window* )
{
  text_viewer *help = new text_viewer;
  help->create_window();
  help->insert_text(application_help_text);
}

/*----------------------------------------------------------------------
 * do_show_window_cmd
 *
 */
void
main_window::do_show_window_cmd(event &, main_window *, window_class *wclass)
{
  vman->show_window( wclass );
}


/*----------------------------------------------------------------------
 * main_window::load_fileset
 */
void
main_window::load_fileset( const String file_name)
{
      suif_env->read(file_name);
      unpost_progress(this);
      update_display();
      post_event(event(0, NEW_FILESET));
}

/*----------------------------------------------------------------------
 * main_window::save_and_delete_fileset
 */
main_window::fileset_state main_window::save_and_delete_fileset(bool cancel_button ) {
  fileset_state result = main_window::no;
  FileSetBlock* fileset = suif_env->get_file_set_block();
  if ( !fileset ) return main_window::yes; // it's like deleted
  s_count_t number_of_files = 1;
  // s_count_t num;

  if ( fileset_modified && ( number_of_files > 0 ) ) {
    result = (fileset_state)display_dialog(this,
		"Do you want to save the current file set?",
		(char*)(cancel_button ? "Yes No Cancel" : "Yes No"), 0);
  }

  if ( result == main_window::cancel ) return result;

  if ( result == main_window::yes ) {
#if TO_BE_SETTLED
    fileset->set_output_suffix(".modified.spd");
    fileset->get_zio_fileset()->root()->write_all();
#endif /* TO_BE_SETTLED */
  }

  suif_env->set_file_set_block(0);
  delete fileset;
  fileset_modified = false;

  if ( result == main_window::yes ) {
    display_message( this, "File set saved" );
  }
  vman->remove_all_vnodes();
  update_display();

  post_event(event(0, CLOSE_FILESET));

  return result;
}


/*----------------------------------------------------------------------
 * main_window::update_display
 */
void
main_window::update_display()
{
  FileSetBlock* fb = suif_env->get_file_set_block();
  text->clear();
  fstream& fout = text->fout();

  if ( fb ) {
    vnode* vn;

    fout << "Visible in Info Viewer:\n";

    vn = create_vnode( fb );
    text->tag_begin(vn);
    fout << "              [Annotes on file_set_block]\n";
    text->tag_end( vn );

    // External Symbol Table
    BasicSymbolTable *tb = fb->get_external_symbol_table();
    if (tb) {
       vn = create_vnode(tb);
       text->tag_begin(vn);
       fout << "              [External Symbol Table]\n";
       text->tag_end( vn );
    } else
       fout << "              [External Symbol Table = NULL]\n";

    // File Set Symbol Table
    tb = fb->get_file_set_symbol_table();
    if (tb) {
       vn = create_vnode(tb);
       text->tag_begin(vn);
       fout << "              [Global Symbol Table]\n";
       text->tag_end( vn );
    } else
       fout << "              [Global Symbol Table = NULL]\n";

    fout << "File Blocks:\n";
    s_count_t count = fb->get_file_block_count();
    for ( s_count_t i = 0; i < count; i++ ) {
      FileBlock* block = fb->get_file_block(i);
      assert(block);
      vn = create_vnode( block );
      text->tag_begin(vn);
      const char *name =  block->get_source_file_name().c_str();
      if ( !name ) name = "<this file_block has no name>";
      fout << "  FILE BLOCK: [`" << name << "']\n";
      text->tag_end(vn);
    }

    count = fb->get_information_block_count();
    for ( s_count_t i = 0; i < count; i++ ) {
      GlobalInformationBlock* global_block = fb->get_information_block(i);
      vn = create_vnode( global_block );
      text->tag_begin( vn );
      fout << "              [" <<  global_block->get_class_name().c_str()
           << "]\n";
      text->tag_end( vn );
    }
  }else {
     fout << "No File Set Block specified.\n";
  }
  text->update();
}


/*----------------------------------------------------------------------
 * main_window::handle_event
 */
void
main_window::handle_event(event& e)
{
  switch (e.kind()) {
  case PROC_MODIFIED:
  case FSE_MODIFIED:
    {
      fileset_modified = true;
    }
    break;
  case SELECTION:
    {
      if (e.get_source() == text) {
	      vnode *vn = e.get_object();
	      if ( (vn->get_tag() == tag_suif_object) /* @@@ &&
	           (is_file_block( (zot*)vn->get_object() ) ) */ ) {
	        vman->show_window("Info Viewer");
	      }
      }
    }
    break;
  default:
    break;
  }
}
