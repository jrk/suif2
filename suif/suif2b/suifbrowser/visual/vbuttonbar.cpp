/*-------------------------------------------------------------------
 * vbuttonbar
 *
 */

#include "vtcl.h"
#include "vbuttonbar.h"
#include "vcommands.h"
#include "vmisc.h"
#include "event.h"
#include "binding.h"
#include "iokernel/helper.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>

struct button_info {
  binding *bd;

  button_info(binding *b) { bd = b; }
  ~button_info() { delete bd; }
};


static void erase_button_list(button_info_list *list)
{
  int total_size = list->size();
  for (int i = 0; i < total_size; ++i)
     list->pop_front();
}

/*-------------------------------------------------------------------
 * vbuttonbar::vbuttonbar
 *
 */
vbuttonbar::vbuttonbar(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());
  tcl << "vbuttonbar_create" << wpath << this << tcl_end;

  button_list = new button_info_list;
}

/*-------------------------------------------------------------------
 * vbuttonbar::~vbuttonbar
 *
 */
vbuttonbar::~vbuttonbar(void)
{
  /* delete the bindings */
  erase_button_list(button_list);
  delete (button_list);
}

/*-------------------------------------------------------------------
 * vbuttonbar::destroy
 *
 */
void vbuttonbar::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vbuttonbar_destroy" << wpath << tcl_end;
  }
}

/*--------------------------------------------------------------------
 * Clear the button bar
 */
void vbuttonbar::clear(void)
{
  tcl << "vbuttonbar_clear" << wpath << tcl_end;

  /* delete the bindings */
  erase_button_list(button_list);
}

/*--------------------------------------------------------------------
 * Add a button
 */
void vbuttonbar::add_button(binding *b, char *text)
{
  tcl << "vbuttonbar_add_button" << wpath << this << text << tcl_end;

  button_info *f = new button_info(b);
  //button_list->append(f);
  button_list->push_back(f);
}

/*--------------------------------------------------------------------*/
/* tk command: vbuttonbar
 *
 */
int TCLTK_CALLING_CONVENTION vbuttonbar::vbuttonbar_cmd(ClientData, Tcl_Interp *interp, int argc,
			       char *argv[])
{
  static char *firstargs[] = {"invoke", "destroy", 0};
  enum { INVOKE = 0, DESTROY };

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }
  vbuttonbar *but;
  if (sscanf(argv[2], "%p", &but) != 1) {
    return (TCL_ERROR);
  }

  switch (a) {
  case INVOKE:
    {
      int but_num;
      if (sscanf(argv[3], "%d", &but_num) != 1) {
	return (TCL_ERROR);
      }

      button_info *button = (*(but->button_list))[ (s_count_t)but_num ];
      button->bd->invoke(event(0, INVOCATION, but));
    }
    break;

  case DESTROY:
    {
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      but->destroy();
    }
    break;

  default:
    return (TCL_ERROR);
  }

  return (TCL_OK);
}
