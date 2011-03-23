/*--------------------------------------------------------------------
 * vbuttonbar.h
 *
 */

#ifndef VBUTTONBAR_H
#define VBUTTONBAR_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"

class binding;
class button_info;

//typedef slist_tos<button_info*> button_info_list;
typedef list<button_info*> button_info_list;

class vbuttonbar : public vwidget {
protected:
  button_info_list *button_list;

public:

  vbuttonbar(vwidget *par);
  ~vbuttonbar(void);
  void destroy(void);

  void add_button(binding *b, char *text);
  void clear(void);

  virtual int kind(void) { return WIDGET_BUTTONBAR; }

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vbuttonbar_cmd(ClientData, Tcl_Interp *interp, int argc,
			    char *argv[]);
};

#endif
