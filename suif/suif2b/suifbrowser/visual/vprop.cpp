/*-------------------------------------------------------------------
 * vprop.cc
 *
 */

#include "vprop.h"
#include "vtcl.h"
#include "vnode.h"
#include "event.h"
#include <stdio.h>
#include "iokernel/helper.h"

bool is_member(vnode_list *l, vnode* key)
{
  list<vnode*>::iterator current = l->begin();
  list<vnode*>::iterator last = l->end();
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

/*-------------------------------------------------------------------
 * vprop::vprop
 */
vprop::vprop(char *n, void *data)
{
  client_data = data;
  nodes = new vnode_list;
  nm = (char *)LString(n).c_str();
  desc = 0;

  foreground = DEFAULT_FOREGROUND;
  background = DEFAULT_BACKGROUND;

  post_event(event(0, PROP_CREATE, 0, this));
}

/*-------------------------------------------------------------------
 * vprop::~vprop
 */
vprop::~vprop(void)
{
  delete nodes;
  post_event(event(0, PROP_DESTROY, 0, this));
}

/*-------------------------------------------------------------------
 * vprop::add_node
 */
void vprop::add_node(vnode *vn)
{
  nodes->push_back(vn);
  vn->add_prop(this);
}

/*-------------------------------------------------------------------
 * vprop::remove_node
 */
void vprop::remove_node(vnode *vn)
{
  if ( is_member( nodes, vn ) ) {
  int pos = get_member_pos(nodes, vn); 
    //nodes->remove_elem( vn );
    nodes->erase(pos);  
    vn->remove_prop(this);
  }
}

/*-------------------------------------------------------------------
 * vprop::erase
 */
void vprop::erase(void)
{
  for ( s_count_t _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    (*nodes)[_cnt]->remove_prop(this);
  }
  //nodes->clear();
  delete_list_and_elements(nodes);
}

/*-------------------------------------------------------------------
 * vprop::update
 */
void vprop::update(void)
{
  post_event(event(0, PROP_CHANGE, 0, this));
}
