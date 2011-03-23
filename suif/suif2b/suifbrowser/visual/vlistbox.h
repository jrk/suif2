/*--------------------------------------------------------------------
 * vlistbox.h
 *
 */

#ifndef VLISTBOX_H
#define VLISTBOX_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"

class vnode;
class binding;

struct listbox_entry {
  vnode *object;
  binding *bd;

  listbox_entry(vnode *vn, binding *b) { object = vn; bd = b; }
};

class vlistbox : public vwidget {

private:

  //array_tos<listbox_entry*> entries;
  list<listbox_entry*> entries;

public:
  vlistbox(vwidget *par, char *title, bool horizontal_slider = false );
  ~vlistbox(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_LISTBOX; }

  void add(binding *b, vnode *object, char *text);
  void clear(void);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vlistbox_cmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[]);

};

#endif
