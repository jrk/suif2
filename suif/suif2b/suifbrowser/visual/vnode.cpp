/*-------------------------------------------------------------------
 * vnode
 *
 */

#include "vnode.h"
#include <stdio.h>
#include <stdlib.h>
#include "vman.h"
#include "event.h"


/*is_member function that finds out if an element is present in a
 * list.
 */

bool is_member(list<vprop*> *l, vprop* key)
{
  list<vprop*>::iterator current = l->begin();
  list<vprop*>::iterator last = l->end();
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
/*------------------------------------------------------------------
 * vnode::vnode
 */
vnode::vnode(void)
{
  object = 0;
  tag = (char*)LString("").c_str();
  props = 0;
  aux_data = 0;

  post_event(event(this, VNODE_CREATE));
}

vnode::vnode(void *const obj, const char *const obj_tag)
{
  object = obj;
  assert(obj_tag && obj);	// this must not be 0
  
  tag = (char *)LString(obj_tag).c_str();
  props = 0;
  aux_data = 0;

  post_event(event(this, VNODE_CREATE));
}

vnode::vnode(void *const obj, const char *const obj_tag, void *data)
{
  object = obj;
  assert(obj_tag && obj);	// this must not be 0
 
  tag = (char *)LString( obj_tag ).c_str();
  props = 0;
  aux_data = data;

  post_event(event(this, VNODE_CREATE));
}

/*-------------------------------------------------------------------
 * vnode::~vnode
 */
vnode::~vnode(void)
{
  delete (props);
  post_event(event(this, VNODE_DESTROY));
}

/*-------------------------------------------------------------------
 * vnode::add_prop
 */
void vnode::add_prop(vprop *p)
{
  if (!props) {
    //props = new slist_tos<vprop*>;
    props = new list<vprop*>;
  }

  if ( !is_member(props, p) ) {
    props->push_back(p);
  }
}

/*-------------------------------------------------------------------
 * vnode::get_prop
 */
vprop *vnode::get_prop(char *name)
{
  if (!props) return 0;
  
  for ( s_count_t i=0; i<props->size(); i++ ) {
	  vprop *prop = (*props)[i];
    if ( prop->name() == name ) {
      return prop;
    }
  }
  return 0;
}

/*-------------------------------------------------------------------
 * vnode::get_prop
 */

/*tos_ref<vprop*> vnode::get_prop_list(void) { 
  static slist_tos<vprop*> *empty_tos = new slist_tos<vprop*>;
  if (props) {
    return tos_ref<vprop*>(props);
  } else {
    return tos_ref<vprop*>(empty_tos);
  }
*/

list<vprop*>* vnode::get_prop_list(void) { 
  if (props) {
    return props;
  } else {
    return 0; 
  }
}

/*-------------------------------------------------------------------
 * vnode::remove_prop
 */
void vnode::remove_prop(char *name)
{
  if (!props) return;

  for ( s_count_t i=0; i<props->size(); i++ ) {
    if ( (*props)[i]->name() == name) {
      vprop *prop = (*props)[i];
      props->erase( props->get_nth(i) );
      delete prop;
      return;
    }
  }

  /*list<prop*>::iterator current = props->begin();
  list<prop*>::iterator last = props->end();
  list<prop*>::iterator next = current;

  while(current != last)
  {
    next = current++;
    if(*current == name)
      props->erase(current);
    current = next;
  }
  */
   
}

/*-------------------------------------------------------------------
 * vnode::remove_prop
 */
void vnode::remove_prop(vprop *p)
{
  if (!props) return;

/*  if (is_member( props, p ) ) {
    //props->remove_elem( p );
    props->erase( p );
    delete p;*/


  list<vprop*>::iterator current = props->begin();
  list<vprop*>::iterator last = props->end();
  int count = 0;

  while(current != last)
  {
    if(*current == p)
    {
      props->erase(current); 
      break;
    }
    current++;
    count++;
  }
}
