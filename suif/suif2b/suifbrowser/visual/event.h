#ifndef EVENT_H
#define EVENT_H

/*
 * The visual system is an event-driven environment. An "event" object
 * contains the current event context - the event kind, the vnode object,
 * the source widget.
 *
 * There is a global event binding list. Whenever an event occurs,
 * event bindings in this global list are invoked. The "add_event_binding"
 * function adds a binding to this list; the "remove_event_binding" removes
 * a binding from the list. The "post_event" method posts a global event,
 * and invokes the corresponding event bindings;
 */

//#include <sty.h>

enum event_kind {
  NULL_EVENT = 0,

  VNODE_CREATE = 1024,
  VNODE_DESTROY,

  WINDOW_CREATE = 2048,
  WINDOW_CLOSE,
  PROP_CREATE,			// New property
  PROP_CHANGE,			// Property changed
  PROP_DESTROY,			// Property removed

  SELECTION = 4096,
  INVOCATION,
  OK_BUTTON,
  CANCEL_BUTTON,

  VISUAL_USER_EVENT = 8192
};

/* event masks */
#define ALL_EVENTS     (-1L)
#define VNODE_EVENTS   1024L
#define VISUAL_EVENTS  2048L
#define X_EVENTS       4096L
#define VISUAL_USER_EVENTS    (VISUAL_USER_EVENT)


class event {
private:
  int e_kind;
  class vnode *vn;
  class vwidget *source_obj;
  void *event_param;

public:
  event() { e_kind = NULL_EVENT; }
  event(vnode *obj, int k, vwidget *source = 0, void *param = 0 ) {
    vn = obj;
    e_kind = k;
    source_obj = source;
    event_param = param;
  }
  ~event() {}

  int kind(void) const { return e_kind; }
  vnode *get_object(void) const { return vn; }
  vwidget *get_source(void) const { return source_obj; }
  void *get_param(void) const { return event_param; }
};


void init_eman(void);
void exit_eman(void);
void post_event(const event &e);
void add_event_binding(class binding *b, int event_mask);
void remove_event_binding(binding *b);
void set_event_mask(binding *b, int event_mask);

#endif
