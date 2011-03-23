/* vcommands */

#include "tcltk_calling_convention.h"
#include "vcommands.h"
#include "vtcl.h"
#include "vgraph.h"
#include "vtext.h"
#include "vmenu.h"
#include "vframe.h"
#include "vtoplevel.h"
#include "vmessage.h"
#include "vlistbox.h"
#include "vform.h"
#include "vbuttonbar.h"
//#include <sty.h>

#include <tcl.h>
#include <stdarg.h>

/*--------------------------------------------------------------------*/
Tcl_CmdProc v_handle_error;
Tcl_CmdProc v_exit;

/*--------------------------------------------------------------------*/

command_entry v_commands[] =
{
  { "v_handle_error", &v_handle_error, 0, 0},
  { "v_exit", &v_exit, 0, 0},
  { "vframe", &vframe::vframe_cmd, 0, 0},
  { "vmenu", &vmenu::vmenu_cmd, 0, 0},
  { "vtext", &vtext::vtext_cmd, 0, 0},
  { "vgraph", &vgraph::vgraph_cmd, 0, 0},
  { "vtoplevel", &vtoplevel::vtoplevel_cmd, 0, 0},
  { "vmessage", &vmessage::vmessage_cmd, 0, 0},
  { "vlistbox", &vlistbox::vlistbox_cmd, 0, 0},
  { "vform", &vform::vform_cmd, 0, 0},
  { "vbuttonbar", &vbuttonbar::vbuttonbar_cmd, 0, 0},
  { 0, 0, 0, 0}
};

/*--------------------------------------------------------------------*/
/*
 * create tcl/tk commands
 *
 */

void v_create_commands(command_entry commands[])
{

  for (int i = 0;; i++) {
    if (commands[i].name == 0) {
      break;
    }
    tcl.create_command(commands[i].name,
		       commands[i].proc,
		       commands[i].clientData,
		       commands[i].delProc);
  }
}

/*--------------------------------------------------------------------*/
/*
 * parse first argument
 *
 */
int v_parse_firstarg(Tcl_Interp *interp, int argc, char *argv[],
		      char *firstargs[])
{
  if (argc >= 2) {
    for (int i = 0; firstargs[i]; i++) {
      if (strcmp(argv[1], firstargs[i]) == 0) {
	return (i);
      }
    }
  }

  Tcl_AppendResult(interp, "Syntax error. First argument must be:", 0);
  for (int i = 0; firstargs[i]; i++) {
    Tcl_AppendResult(interp, " ", firstargs[i], 0);
  }
  return (-1);
}

/*--------------------------------------------------------------------*/
/*
 * v_wrong_argc
 *
 */

void v_wrong_argc(Tcl_Interp *interp)
{
  Tcl_SetResult(interp, "wrong # args", TCL_STATIC);
}

/*--------------------------------------------------------------------*/
/*
 * v_warning
 *
 */

void v_warning(Tcl_Interp *interp)
{
  fprintf(stderr, "Warning: %s\n", interp->result);
}

void v_warning(char *msg ...)
{
  va_list ap;
  va_start(ap, msg);

  vfprintf(stderr, msg, ap);
  va_end(ap);
}

/*--------------------------------------------------------------------*/
/*
 * v_error
 *
 */

void v_error(int return_code, char *msg)
{
  fprintf(stderr, "Error(%d): %s\n", return_code, msg);
  exit(1);
}

/*--------------------------------------------------------------------*/
/*
 * v_handle_error
 *
 */

int TCLTK_CALLING_CONVENTION v_handle_error(ClientData, Tcl_Interp *interp, int /* argc */, /* unused */
		    char * /* argv */ []) /* unused */
{
  char buffer[200];
  sprintf(buffer, "Tcl error: result was: %s\n", interp->result);
  v_error(1, buffer);
  return (TCL_OK);
}

/*--------------------------------------------------------------------*/
/*
 * v_exit
 *
 */

int TCLTK_CALLING_CONVENTION v_exit(ClientData, Tcl_Interp * /* interp */ /* unused */, int argc,
	  char *argv[])
{
  int exit_code = 0;
  if (argc > 1) {
    exit_code = atoi(argv[1]);
  }
  exit(exit_code);
  return (TCL_OK);
}

