/*--------------------------------------------------------------------
 * vnode.h
 *
 * A "vnode" object is a visual object that represents any client object.
 * It is used as a handle in the visual system. It contains a pointer
 * a pointer to the actual object, a tag that is a string identifying the
 * type of the object, a list of visual properties (vprops) that the
 * object has.
 *
 * When the object is freed, its dual vnode should also be deleted.
 * The visual_manager keeps track of all the vnodes, and makes sure that
 * there is only one vnode per client object.
 *
 */

#ifndef VNODE_H
#define VNODE_H

#include "common/suif_list.h"


class vprop;

class vnode {
private:
  friend vprop;

  char *tag;
  void *object;
  void *aux_data;

//  slist_tos<vprop*> *props;
  list<vprop*> *props;

  void add_prop(vprop *prop);
  void remove_prop(char *prop_name);
  void remove_prop(vprop *prop);

public:
  vnode(void);
  vnode(void *const obj, const char *const tag);
  vnode(void *const obj, const char *const tag, void *data);
  ~vnode(void);

  char *get_tag(void) const { return tag; }
  void *get_object(void) const { return object; }
  void *get_data(void) const { return aux_data; }
  void set_object(void *const obj) { object = obj; }

  /* properties */
  vprop *get_prop(char *prop_name);
  //tos_ref<vprop*> get_prop_list(void);
  list<vprop*>* get_prop_list(void);
};

//typedef slist_tos<vnode*> vnode_list;
typedef list<vnode*> vnode_list;
typedef list<vnode *>::iterator vlist_iter; 

#endif
