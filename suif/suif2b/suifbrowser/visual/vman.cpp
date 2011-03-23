/*-------------------------------------------------------------------
 * vman.cc
 *
 */

#include "vman.h"
#include "vmisc.h"
#include "binding.h"
#include "event.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "iokernel/helper.h"

visual_manager *vman;

#define MAX_HISTORY_LENGTH 50
/*-----------------------------------------------------------------
 * is_member function that finds out if an element is present in a
 * list.
 */
template <class T1, class T2>
bool is_member(T1 *l, T2 *key)
{
  typename T1::iterator current = l->begin();
  typename T1::iterator last = l->end();
  int count = 0;

  while(current != last)
  {
   if(*current == key)
     return true;
   current++;
   count++;
  }
  return false;
}
/* ---------------------------------------------------------------
 * get_member_pos() which returns the position given a value.
 */
template <class T1, class T2>
int get_member_pos(T1 *l, T2 *key)
{
  typename T1::iterator current = l->begin();
  typename T1::iterator last = l->end();
  int count = 0;

  while(current != last)
  {
   if(*current == key)
     return count;
   current++;
   count++;
  }
  return -1;
}

/*-------------------------------------------------------------------*/
inline int hashfn(const void *ptr) {
  int hash = (((int)ptr) >> 2) ^ (((int)ptr) >> 10);
  return (hash & VN_HASH_MASK);
}

/*-------------------------------------------------------------------
 * Init and clean up functions 
 */
void init_vman(void)
{
  vman = new visual_manager;
  vman->init();
}

void exit_vman(void)
{
  delete vman;
}

/*-------------------------------------------------------------------
 * visual_manager
 */
visual_manager::visual_manager(void)
{
  toplevel_num = 0;		// window number
  wclasses = new window_class_list;
  current_windows = new window_instance_list;
  event_binding = 0;
  props = new vprop_list;
  modules = new module_list;
  vn_history = new vnode_list;
}

/*-------------------------------------------------------------------
 * visual_manager
 */
visual_manager::~visual_manager(void)
{
  if (event_binding) {
    remove_event_binding(event_binding);
    delete (event_binding);
  }
  delete_list_and_elements(wclasses);
  delete_list_and_elements(current_windows);
  delete (props);
  delete (modules);
}

/*-------------------------------------------------------------------
 * visual_manager::init
 */
void
visual_manager::init(void)
{
  if (!event_binding) {
    event_binding = new binding((bfun) &event_handler, this);
    add_event_binding(event_binding, ALL_EVENTS);
  }
}

/*-------------------------------------------------------------------
 * visual_manager::find_vnode
 */
vnode *visual_manager::find_vnode(const void *obj)
{
  vnode_list *vn_list = &hashed_list[hashfn(obj)];
  for ( s_count_t _cnt=0; _cnt<vn_list->size(); _cnt++ ) {
    vnode *vn = (*vn_list)[_cnt];
    if ( vn->get_object() == obj) {
      return vn;
    }
  }
  return (0);
}

/*-------------------------------------------------------------------
 * visual_manager::add_vnode
 */
void visual_manager::add_vnode(vnode *vn)
{
  vnode_list *vn_list = &hashed_list[hashfn(vn->get_object())];
  vn_list->push_back(vn);
}

/*-------------------------------------------------------------------
 * remove_vnode
 */
void visual_manager::remove_vnode(vnode *vn)
{
  vnode_list *vn_list = &hashed_list[hashfn(vn->get_object())];
  int pos = get_member_pos(vn_list, vn); 
  if (pos != -1) {
    //vn_list->remove_elem( vn );
    vn_list->erase(pos);
    //delete vn;
  }
}

/*-------------------------------------------------------------------
 * visual_manager::remove_all_vnodes
 */
void visual_manager::remove_all_vnodes(void)
{
 // vn_history->clear();
  int total_size = vn_history->size();
  for (int i = 0; i < total_size; ++i)
    vn_history->pop_front();
  for (int i = 0; i < VN_HASH_SIZE; i++) {
    vnode_list *vlist = &hashed_list[i];
    total_size = vlist->size();
    for (int j = 0; j < total_size; ++j)
      vlist->pop_front();
  }
}

/*-------------------------------------------------------------------
 * visual_manager::get_selection
 */
vnode *visual_manager::get_selection(void) const
{
  if (vn_history->empty()) {
    return (0);
  } else {
    //return vn_history->head();
    return (*vn_history->begin());
  }
}
/*-------------------------------------------------------------------
 * visual_manager::new_toplevel_path
 *
 * assign a path to a new top level window
 */
void visual_manager::new_toplevel_path(char *path)
{
  sprintf(path, ".vwin%d", toplevel_num);
  toplevel_num++;
  assert (!widget_exists(path));
}

/*-------------------------------------------------------------------
 * visual_manager::register_window_class
 *
 */
void visual_manager::register_window_class(char *class_name, window_cons_fn fn)
{
  wclasses->push_back(new window_class(class_name, fn));
}

/*-------------------------------------------------------------------
 * visual_manager::find_window_class
 *
 */
window_class *visual_manager::find_window_class(char *name)
{
  for ( s_count_t i=0; i< wclasses->size(); i++ ) {
    window_class *wclass = (*wclasses)[i];
    if (strcmp(wclass->name(), name) == 0) {
      return (wclass);
    }
  }
  return (0);
}

/*----------------------------------------------------------------------
 * get_current_windows
 */
void visual_manager::get_current_windows(window_list &win_list)
{
  //win_list.clear();
  int total_size = win_list.size();
  for (int i = 0; i < total_size; ++i)
    win_list.pop_front();
  for (s_count_t i=0; i<current_windows->size(); i++) {
      win_list.push_back( (*current_windows)[i]->instance );
  }
}

/*----------------------------------------------------------------------
 * show_window
 */
window *visual_manager::show_window(window_class *wclass)
{
  window *instance = find_window_instance(wclass);
 
  if (instance) {
    instance->raise();
  } else {      
    /* create new instance */
    instance = create_window_instance(wclass);
    instance->create_window();
  }
  return (instance);
}

window *visual_manager::show_window(char *class_name)
{
  window_class *wclass = find_window_class(class_name);
  if (wclass) {
    return show_window(wclass);
  } else {
    return 0;
  }
}

/*----------------------------------------------------------------------
 * craete_window_instance
 */
window *visual_manager::create_window_instance(window_class *wclass)
{
  window *win = wclass->constructor()();
  return (win);
}

/*----------------------------------------------------------------------
 * find_window_instance
 */
window *visual_manager::find_window_instance(window_class *wclass)
{
  for ( s_count_t i=0; i<current_windows->size(); i++ ) {
    window_instance *win = (*current_windows)[i];
    if (win->wclass == wclass) {
      return (win->instance);
    }
  }
  return (0);
}

bool visual_manager::find_window_instance(window *win)
{
  for ( s_count_t i=0; i<current_windows->size(); i++ ) {
    window_instance *inst = (*current_windows)[i];  
    if (inst->instance == win) {
      return (true);
    }
  }
  return (false);
}

/*----------------------------------------------------------------------
 * new_window_instance
 */
void visual_manager::new_window_instance(window *win, char *class_name)
{
  window_class *wclass = find_window_class(class_name);

  window_instance *inst = new window_instance;
  inst->wclass = wclass;
  inst->instance = win;
  current_windows->push_back(inst);
}

/*----------------------------------------------------------------------
 * remove_window_instance
 *
 */
void visual_manager::remove_window_instance(window *the_win)
{

 for ( s_count_t i=0; i<current_windows->size(); i++ ) {
    window_instance *win = (*current_windows)[i];  
    if (win->instance == the_win) {
      current_windows->erase( current_windows->get_nth(i) );
      delete win;
      break;
    }
  }
}

/*----------------------------------------------------------------------
 * visual_manager::register_module
 *
 */
void visual_manager::register_module(char * /* name */, /* unused */
				     module * (*constructor_fn)(void))
{
  module *m = constructor_fn();
  add_module(m);
}

/*----------------------------------------------------------------------
 * visual_manager::add_module
 *
 */
void visual_manager::add_module(module *m)
{
  modules->push_back(m);
}

/*----------------------------------------------------------------------
 * visual_manager::go_back
 *
 */
void visual_manager::go_back(void)
{
  if ( vn_history->size()>1 ) {	// make sure there are two objects
    vn_history->front();
    vnode *vn = vn_history->front();

    /* post selection event */
    post_event(event(vn, SELECTION, 0));
  }
}

/*----------------------------------------------------------------------
 * event handler
 *
 */
void visual_manager::event_handler(event &e, visual_manager *visual_man)
{
  switch (e.kind()) {
  case VNODE_CREATE:
    {

#if 0
      assert_msg(visual_man->find_vnode(e.get_object()) == 0,
		 ("Internal error. Vnode duplicated."));
#endif

      visual_man->add_vnode(e.get_object());
    }
    break;

  case VNODE_DESTROY:
    {
      vnode *vn = e.get_object();

      // remove vnode from list
      visual_man->remove_vnode(e.get_object());

      // remove vnode from the history list
      for ( s_count_t _cnt=0; _cnt<visual_man->vn_history->size(); _cnt++ ) {
        vnode *curr = (*(visual_man->vn_history))[_cnt];
	if ( curr == vn ) {
          //visual_man->vn_history->remove( _cnt );
          visual_man->vn_history->
		     erase(visual_man->vn_history->get_nth(_cnt));
          _cnt--; // AG @@@ is this necessary?
	}
      }


    }
    break;

  case WINDOW_CREATE:
    {
      window *win = (window *) e.get_param();
      visual_man->new_window_instance(win, win->class_name());
    }
    break;

  case WINDOW_CLOSE:
    {
      window *win = (window *) e.get_param();
      visual_man->remove_window_instance(win);
    }
    break;

  case SELECTION:
    {
      vnode *sel = e.get_object();
      int n = visual_man->vn_history->size();
      if (n == 0 ||
	  //visual_man->vn_history->head() != sel) {
	  visual_man->vn_history->front() != sel) {

	int pos = get_member_pos(visual_man->vn_history, sel);
        // remove from history list
	if( pos != -1)
          visual_man->vn_history->erase( pos );
	// add to top of history list
	visual_man->vn_history->push_back( sel );

	if (n > MAX_HISTORY_LENGTH) {
	  pos = get_member_pos(visual_man->vn_history, 
	                      visual_man->vn_history->back());
	  //visual_man->vn_history->remove_elem(visual_man->vn_history->
	  //tail());
          visual_man->vn_history->erase(pos);
	}
      }
    }
    break;

  case PROP_CREATE:
    {
      vprop *p = (vprop *) e.get_param();
      if (!is_member(visual_man->props, p)) {
	visual_man->props->push_back(p);
      }
    }
    break;

  case PROP_DESTROY:
    {
      vprop *p = (vprop *) e.get_param();
      //tos_handle<vprop*> handle;
      //handle = visual_man->props->lookup_handle(p);
      //if ( !handle.is_null() ) {
        //visual_man->props->remove( handle );
      //}
      int pos = get_member_pos(visual_man->props, p);
      // Check if remove does not delete the node, but erase does!
      if (pos != -1)
	visual_man->props->erase(pos);
    }
    break;

  default:
    break;
  }
}
