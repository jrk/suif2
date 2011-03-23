/*--------------------------------------------------------------------
 * vmessage.h
 *
 */

#ifndef VMESSAGE_H
#define VMESSAGE_H

#include "tcltk_calling_convention.h"
#include <tcl.h>
#include <tk.h>
#include "vwidget.h"

class vmessage : public vwidget {

 private:

 public:
  vmessage(vwidget *par);
  ~vmessage(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_MESSAGE; }

  void set_message(char *mesg);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vmessage_cmd(ClientData clientData, Tcl_Interp *interp, int argc,
			  char *argv[]);

};

#endif
