/* window.h */

#ifndef WINDOW_H
#define WINDOW_H

/*
 * A "window" class is a basic top-level window in the visual system.
 * The "create_window" method is used to create the window; it is a
 * virtual function and subclasses should override this with any
 * additional code.
 * 
 * When the window is destroyed, the "destroyed" method is called by
 * tcl/tk, and the window object is then deleted.
 */

#include "vtoplevel.h"
#include "binding.h"
#include "suifkernel/suifkernel_forwarders.h"

class event;


/*
 * window
 */

class window {
protected:
  binding_list *bindings;
  vtoplevel *toplevel;

public:
  window(void);
  virtual ~window(void);
  virtual void destroy(void);
  virtual void create_window(void);
  virtual void destroyed(void);

  /* must override this method to define the class name */
  virtual char *class_name(void) { return "no-name"; }

  /* misc */
  vtoplevel *toplevel_window(void) const { return toplevel; }

  /* display methods */
  void raise(void) { if (toplevel) toplevel->raise(); }
  void lower(void) { if (toplevel) toplevel->lower(); }
  void iconify(void) { if (toplevel) toplevel->iconify(); }
  void deiconify(void) { if (toplevel) toplevel->deiconify(); }
  void withdraw(void) { if (toplevel) toplevel->withdraw(); }
private:
  /* override stupid defaults, no implementation needed */
  window &operator=(const window &);
  window(const window &);
};


typedef list<window*> window_list;

/*
 * window_class
 */
 
typedef window *(*window_cons_fn)(void);

class window_class {
private:
  window_cons_fn cons_fn;	// constructor fn
  LString nm;			// name

public:
  window_class(char *n, window_cons_fn fn) : nm(n) {
    //nm = LString(n).c_str();
    cons_fn = fn;
  }

  const char *name(void) { return nm.c_str(); }
  window_cons_fn constructor(void) { return cons_fn; }
};

typedef list<window_class*> window_class_list;

#endif
