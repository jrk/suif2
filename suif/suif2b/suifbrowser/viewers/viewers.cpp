/*--------------------------------------------------------------------
 * viewers.cc
 */
#include "output_viewer.h"
#include "suif_viewer.h"
#include "src_viewer.h"
#include "text_viewer.h"
#include "info_viewer.h"
#include "cg_viewer.h"
#include "prof_viewer.h"
#include "list_viewer.h"
#include "suif_vnode.h"
#include "suif_print.h"

#include "main_window.h"


/*--------------------------------------------------------------------
 * main initialization routine
 */
void enter_viewers( int *, char *[] ) {
   static bool init_flag = false;
   if (init_flag) return;
   init_flag = true;

   init_vnode_tags();

#ifdef CURRENTLY_NOT_IMPLEMENTED
    /* Pointer analysis initialization */
    wilbyr_init_annotes();

    /* Register dynamic types */
    register_suif_types();  //AG do we really need that??
#endif

    /* Register viewers */
    vman->register_window_class("Output Viewer", &output_viewer::constructor);
    vman->register_window_class("Suif Viewer", &suif_viewer::constructor);
    vman->register_window_class("Source Viewer", &src_viewer::constructor);
    vman->register_window_class("Text Viewer", &text_viewer::constructor);
    vman->register_window_class("Info Viewer", &info_viewer::constructor);
    vman->register_window_class("Callgraph Viewer", &cg_viewer::constructor);
    vman->register_window_class("Profile Viewer", &prof_viewer::constructor);
    vman->register_window_class("Procedure List", &proc_list::constructor);
}

/*----------------------------------------------------------------------
 * exit
 */
main_window* main_window::get_main_window() { return main_window::mainwin; }

void exit_viewers(void) {
  main_window* mainwin = main_window::get_main_window();
  if ( mainwin ) {
    mainwin->destroy();
  }
}

/*----------------------------------------------------------------------
 * start_viewers
 */
void start_viewers( int argc, char *argv[] ) {
  main_window* main =  new main_window;

  // duplicate command line
  char **ptr = new char*[argc];
  for ( int i = 0; i < argc; i++ ) {
    char* value = 0;
    if ( argv[i] ) {
      int len = strlen( argv[i] );
      value = new char[len + 1 ];
      strcpy( value, argv[i] );
    }
    ptr[i] =  value;
  }
  main->create_window( argc, ptr );
}
