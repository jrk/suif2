/*--------------------------------------------------------------------
 * vman.h
 *
 * The visual_manager class manages all the internals of the visual system.
 * it is responsible for starting and exiting the visual system,
 * keep track of the vnodes, window classes, window instances,
 * external modules, visual properties.
 *
 * There should only be one instance (vman) of the visual_manager class.
 *
 */

#ifndef VMAN_H
#define VMAN_H

#include "vnode.h"
#include "vprop.h"
#include "window.h"
#include "vmodule.h"

class binding;

void init_vman(void);
void exit_vman(void);

/* If you change the hash table parameters, you should change the hash
 * function also.
 */
#define VN_HASH_SIZE 256
#define VN_HASH_MASK 255


/*----------------------------------------------------------------------
 * window
 */
struct window_instance {
  window *instance;
  window_class *wclass;
  
  window_instance(void) {
    instance = 0;
    wclass = 0;
  }
};

typedef list<window_instance*> window_instance_list;

/*----------------------------------------------------------------------
 * Visual Manager
 *
 * There should only be one instance of this class
 */
class visual_manager {
private:
  binding *event_binding;

  /* toplevel management */
  int toplevel_num;

  /* vnodes management */
  vnode_list hashed_list[VN_HASH_SIZE];
  vnode_list *vn_history;

  void add_vnode(vnode *vn);
  void remove_vnode(vnode *vn);

  /* window class management */
  window_class_list *wclasses;

  /* window management */
  window_instance_list *current_windows;

  /* properties */
  vprop_list *props;

  /* modules */
  module_list *modules;


  static void event_handler(event &e, visual_manager *v);

public:
  visual_manager(void);
  ~visual_manager(void);
  void init(void);

  /* toplevel path management */
  void new_toplevel_path(char *path);

  /* vnodes management */
  vnode *find_vnode(const void *obj);
  void remove_all_vnodes(void);

  /* selection */
  vnode *get_selection(void) const;
  vnode_list *get_selection_history(void) const { return vn_history; }
  void go_back(void);

  /* window class management */
  window_class_list *get_window_classes(void) const { return wclasses; }
  void register_window_class(char *class_name, window_cons_fn fn);
  window_class *find_window_class(char *name);

  /* window management */
  window *create_window_instance(window_class *);
  void new_window_instance(window *win, char *class_name);
  void remove_window_instance(window *win);
  void get_current_windows(window_list &win_list);

  window *find_window_instance(window_class *wclass);
  bool find_window_instance(window *win);
  window *show_window(window_class *wclass);
  window *show_window(char *class_name);

  /* modules */
  void register_module(char *name, module *(*constructor_fn)(void));
  void add_module(module *m);
  module_list *get_module_list(void) { return modules; }

  /* props */
  vprop_list *get_prop_list(void) { return props; }
};

extern visual_manager *vman;

/*----------------------------------------------------------------------
 * Macros
 *
 */

#define REGISTER_MODULE(name, constructor_function) \
{ \
  vman->register_module(name, constructor_function); \
}

#define REGISTER_WINDOW_CLASS(name, constructor_function) \
{ \
  vman->register_window_class(name, constructor_function); \
}

#endif
