/*-------------------------------------------------------------------
 * vtoplevel
 *
 */

#include "vtoplevel.h"
#include "vtcl.h"
#include "vman.h"
#include "vcommands.h"
#include "event.h"
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------------------
 * vtoplevel::toplevel
 *
 */
vtoplevel::vtoplevel(char *title, window *win)
: vwidget(0)
{
  vman->new_toplevel_path(wpath);
  tcl << "vtoplevel_create" << wpath << this << title << tcl_end;

  owner = win;

  tcl << "vtoplevel_place" << wpath << title << tcl_end;
  set_title(title);
  deiconify();
}

/*-------------------------------------------------------------------
 * vtoplevel::~vtoplevel
 *
 */
vtoplevel::~vtoplevel(void)
{
}

/*-------------------------------------------------------------------
 * vtoplevel::destroy
 *
 */
void vtoplevel::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vtoplevel_destroy" << wpath << tcl_end;
    if (owner) {
      owner->destroyed();
    }
  }
}

/*-------------------------------------------------------------------
 * vtoplevel::set_title
 *
 */

void vtoplevel::set_title(char *title)
{
  tcl << "vtoplevel_set_title" << wpath << title << tcl_end;
}

/*-------------------------------------------------------------------
 * vtoplevel::iconify
 *
 */
void vtoplevel::iconify(void)
{
  tcl << "vtoplevel_iconify" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vtoplevel::deiconify
 *
 */
void vtoplevel::deiconify(void)
{
  tcl << "vtoplevel_deiconify" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vtoplevel::raise
 *
 */
void vtoplevel::raise(void)
{
  tcl << "vtoplevel_raise" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vtoplevel::lower
 *
 */
void vtoplevel::lower(void)
{
  tcl << "vtoplevel_lower" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vtoplevel::withdraw
 *
 */
void vtoplevel::withdraw(void)
{
  tcl << "vtoplevel_withdraw" << wpath << tcl_end;
}

/*--------------------------------------------------------------------*/
/* tk command
 *
 */
int TCLTK_CALLING_CONVENTION vtoplevel::vtoplevel_cmd(ClientData, Tcl_Interp *interp,
			     int argc, char *argv[])
{
  static char *firstargs[] = {"destroy", 0};
  enum { DESTROY = 0};

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  switch (a) {
  case DESTROY:
    {
      if (argc != 4) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vtoplevel *toplevel;
      sscanf(argv[2], "%p", &toplevel);
      // check whether we have a toplevel widget
      if ( !strcmp( toplevel->path(), argv[3] ) ) {
        toplevel->destroy();
      }
    }
    break;

  default:
    return (TCL_ERROR);
  }

  return (TCL_OK);
}
