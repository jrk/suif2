/*-------------------------------------------------------------------
 * vmessage
 *
 */

#include "vmessage.h"
//#include <sty.h>
#include "vtcl.h"
#include "vcommands.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------------------
 * vmessage::vmessage
 *
 */
vmessage::vmessage(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());

  /* create frame widget */
  tcl << "vmessage_create" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vmessage::~vmessage
 *
 */
vmessage::~vmessage(void)
{
}

/*-------------------------------------------------------------------
 * vmessage::destroy
 *
 */
void vmessage::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vmessage_destroy" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vmessage::set_message
 *
 */
void vmessage::set_message(char *mesg)
{
  char *tmp = mesg;

  char unset_com[] = "catch {global v_tmp; unset v_tmp}";
  tcl.eval(unset_com);

  if (tcl.link_var("v_tmp", (char *) &tmp,
		   TCL_LINK_STRING | TCL_LINK_READ_ONLY) != TCL_OK) {
    v_warning("Cannot link variable (%s)", tcl.result());
  }
  char com[100];
  sprintf(com, "global v_tmp; vmessage_set %s $v_tmp", wpath);
  tcl.eval(com);

  tcl.unlink_var("v_tmp");
  tcl.eval(unset_com);
}

/*--------------------------------------------------------------------*/
/* tk command
 */
int TCLTK_CALLING_CONVENTION vmessage::vmessage_cmd(ClientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
  static char *firstargs[] = {"destroy", 0};
  enum { DESTROY = 0};

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }
  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }

  vmessage *message;
  sscanf(argv[2], "%p", &message);

  switch (a) {
  case DESTROY:
    message->destroy();
    break;
  }
  return (TCL_OK);
}
