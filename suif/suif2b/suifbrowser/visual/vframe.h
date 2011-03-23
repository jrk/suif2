/*--------------------------------------------------------------------
 * vframe.h
 *
 */

#ifndef VFRAME_H
#define VFRAME_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"
#include "vtcl.h"

class vframe : public vwidget {

 public:
  vframe(vwidget *par, bool expand = true );
  ~vframe(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_FRAME; }

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vframe_cmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]);
};

#endif
