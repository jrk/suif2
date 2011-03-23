/*--------------------------------------------------------------------
 * vwidget.h
 *
 * The "vwidget" is a base widget class. All other widgets are derived
 * from this class. A widget is a basic window component, it corresponds
 * roughly to a tcl/tk widget.
 *
 * The "win" method returns the current window that contains the widget.
 *
 */

#ifndef VWIDGET_H
#define VWIDGET_H

//#include <sty.h>
#include "vdefs.h"


class vnode;
class vprop;
class window;

enum wstates {
  W_NORMAL,
  W_DESTROYED
};

enum {
  WIDGET_NULL = 0,
  WIDGET_TOPLEVEL,
  WIDGET_FRAME,
  WIDGET_MENU,
  WIDGET_TTY,
  WIDGET_TEXT,
  WIDGET_MESSAGE,
  WIDGET_GRAPH,
  WIDGET_LISTBOX,
  WIDGET_FORM,
  WIDGET_HTML,
  WIDGET_BUTTONBAR
};

// this is a virtual base class

class vwidget {

 protected:
  char wpath[MAX_PATH_LEN];	// widget pathname
  vwidget *parent;

  int state;			// state of the widget

 public:
  vwidget(vwidget *par) { 
    parent = par;
    state = W_NORMAL;
    wpath[0] = '\0';
  }
  virtual ~vwidget(void) {}
  virtual void destroy(void) {}
  virtual int kind(void) { return WIDGET_NULL; }

  char *path(void) { return wpath; }
  window *win(void);
  bool is_alive(void) { return (state == W_NORMAL); }
  
  virtual vnode *get_selection(void) { return 0; }
};

#endif
