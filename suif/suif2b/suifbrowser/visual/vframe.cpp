/*-------------------------------------------------------------------
 * vframe
 *
 */

#include "vframe.h"
#include "vtcl.h"
//#include <sty.h>
#include "vcommands.h"
#include "vmisc.h"
#include "window.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------------------
 * vframe::vframe
 *
 */
vframe::vframe(vwidget *par, bool expand ) : vwidget(par)
{
  for (int i = 0;; i++) {
    sprintf(wpath, "%s.frame%d", par->path(), i);
    if (!widget_exists(wpath)) {
      break;
    }
  }

  char *class_name = "";
  window *w = win();
  if (w) {
    class_name = w->class_name();
  }

  /* create frame widget */
  tcl << "vframe_create" << wpath << this << class_name <<(int)expand<< tcl_end;
}

/*-------------------------------------------------------------------
 * vframe::~vframe
 *
 */
vframe::~vframe(void)
{
}

/*-------------------------------------------------------------------
 * vframe::destroy
 *
 */
void vframe::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vframe_destroy" << wpath << tcl_end;
  }
}

/*--------------------------------------------------------------------*/
/* tk command:
 * vframe destroy <vframe>
 */
int TCLTK_CALLING_CONVENTION vframe::vframe_cmd(ClientData, Tcl_Interp *interp,
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
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vframe *frame;
      sscanf(argv[2], "%p", &frame);
      frame->destroy();
    }
    break;

  }

  return (TCL_OK);
}
