/* vcommands.h */

#ifndef VCOMMANDS_H
#define VCOMMANDS_H

#include <tcl.h>
#include "tk.h"

struct command_entry {
  char *name;
  Tcl_CmdProc *proc;
  ClientData clientData;
  Tcl_CmdDeleteProc *delProc;
};

extern command_entry v_commands[];

/*
 * create tcl/tk commands 
 */

void v_create_commands(command_entry commands[]);

/*
 * helper functions for implementing tcl/tk commands
 */

int v_parse_firstarg(Tcl_Interp *interp, int argc, char *argv[],
		      char *firstargs[]);
void v_wrong_argc(Tcl_Interp *interp);
void v_warning(Tcl_Interp *interp);
void v_warning(char *msg ...);
void v_error(int return_code, char *msg);

#endif
