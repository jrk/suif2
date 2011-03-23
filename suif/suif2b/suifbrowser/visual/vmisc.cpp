/*-------------------------------------------------------------------
 * vmisc
 *
 */

#include "vtcl.h"
#include "vcommands.h"
#include "vmisc.h"
#include "window.h"
//#include <sty.h>
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern void visual_yield(int visual_yield);

static char update_command[] = "update";

/*--------------------------------------------------------------------
 * Duplicate a string  (Note: in some systems, strdup is missing) 
 */
char *strdup(char *s)
{
  char *new_s = (char *) malloc(strlen(s) + 1);
  strcpy(new_s, s);
  return (new_s);
}

/*-------------------------------------------------------------------
 */

static char *window_path(window *win)
{
  char *path = "";
  if (win && win->toplevel_window()) {
    path = win->toplevel_window()->path();
  }
  return (path);
}

/*-------------------------------------------------------------------
 * widget_exists
 *
 * returns 1 if widget exists, 0 otherwise
 */

bool widget_exists(char *path)
{
  tcl << "winfo" << "exists" << path << tcl_end;

  int result;
  tcl >> result;
  if (result) {
    return true;
  } else {
    return false; 
  }
}

/*--------------------------------------------------------------------
 * select_file
 *
 * display a select file dialog
 * returns the filename selected
 */
void select_file(window *parent, char *filename, char *text,
		 char *default_filename)
{
  char *parent_path = window_path(parent);

  int result = tcl << "select_file" << parent_path << text << 
    default_filename << tcl_end;
  if (result == TCL_OK) {
    strcpy(filename, tcl.result());
  } else {
    filename[0] = 0;
  }
}

/*--------------------------------------------------------------------
 * select_fileset
 *
 * display a select fileset dialog
 * returns the files selected, in the form of a list of filenames
 * separated by blank spaces.
 */
char *select_fileset(window *parent, char *text, char *fileset)
{
  char *parent_path = window_path(parent);

  int result = tcl << "select_fileset" << parent_path << text <<
    (fileset ? fileset : "") << tcl_end;
  if (result == TCL_OK) {

    char *r = tcl.result();
    if (r[0]) {
      char *new_list = new char[strlen(r) + 1];
      strcpy(new_list, r);
      return new_list;
    }
  }

  return (0);
}

/*--------------------------------------------------------------------
 * display_message
 *
 */
void display_message(window *parent, char *message ...)
{
  char *parent_path = window_path(parent);

  va_list ap;
  va_start(ap, message);

  static char buffer[200];
  vsprintf(buffer, message, ap);
  va_end(ap);

  tcl << "display_message" << parent_path << buffer << tcl_end;
}

/*--------------------------------------------------------------------
 * display_dialog
 *
 */
int display_dialog(window *parent, char *message,
		    char *options, int default_option)
{
  char *parent_path = window_path(parent);

  tcl << "display_dialog" << parent_path << message <<
    options << default_option << tcl_end;
  int result;
  tcl >> result;
  return (result);
}

/*--------------------------------------------------------------------
 * display_query
 *
 */
void display_query(window *parent, char *message, char *result)
{
  char *parent_path = window_path(parent);

  tcl << "display_query" << parent_path << message << tcl_end;
  if (result) {
    strcpy(result, tcl.result());
  }
}

/*--------------------------------------------------------------------
 * post_progress
 *
 */
void post_progress(window *parent, char *message, float percent_completed)
{
  char *parent_path = window_path(parent);

  tcl << "post_progress" << parent_path << message << percent_completed <<
    tcl_end;
  tcl.eval(update_command);
}

/*--------------------------------------------------------------------
 * unpost_progress
 *
 */
void unpost_progress(window *parent)
{
  char *parent_path = window_path(parent);

  tcl << "unpost_progress" << parent_path << tcl_end;
}
