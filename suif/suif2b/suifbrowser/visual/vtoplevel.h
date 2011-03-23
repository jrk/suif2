/*--------------------------------------------------------------------
 * vtoplevel.h
 *
 */

#ifndef VTOPLEVEL_H
#define VTOPLEVEL_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"
#include "vtcl.h"

class window;

class vtoplevel : public vwidget {

protected:
  window *owner;

public:
  vtoplevel(char *title, window *win);
  ~vtoplevel(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_TOPLEVEL; }

  /* misc */
  void set_title(char *title);
  window *win(void) { return owner; }

  /* display */
  void raise(void);
  void lower(void);
  void iconify(void);
  void deiconify(void);
  void withdraw(void);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vtoplevel_cmd(ClientData clientData, Tcl_Interp *interp,
			   int argc, char *argv[]);
};

#endif
