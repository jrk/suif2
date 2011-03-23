/*-------------------------------------------------------------------
 * vtagman
 *
 */


#include "vtagman.h"
#include "vtcl.h"
//#include <sty.h>
#include <tcl.h>
#include <stdio.h>


/*-------------------------------------------------------------------
 * tag_node::tag_node
 *
 */

tag_node::tag_node(void)
{
  next = son = last_son = parent = 0;
  object = 0;
  pr_fn = 0;
  state.expanded = false;
}

/*-------------------------------------------------------------------
 * tag_node::get_depth
 *
 */
int tag_node::get_depth(void)
{
  return (depth);
}

/*-------------------------------------------------------------------
 * tag_node::lookup
 *
 */
void tag_node::lookup(vnode *obj, tag_node_list *found)
{
  for (tag_node *child = son; child; child = child->next) {
    if (child->object == obj) {
      found->push_back(child);
    }
    child->lookup(obj, found);
  }
}

/*-------------------------------------------------------------------
 * tag_node::set_print_fn
 *
 */
void tag_node::set_print_fn(print_fn p_fn, int d, void *c_data) {
  assert(son == 0);	// this must be a leaf node!
  depth = d;
  pr_fn = p_fn;
  client_data = c_data;
}

/*-------------------------------------------------------------------
 * tag_node::add_son
 */
void tag_node::add_son(tag_node *node) {
  if (last_son) {
    last_son->next = node;
  } else {
    son = node;
  }
  last_son = node;
  node->parent = this;
}

/*-------------------------------------------------------------------
 * tag_node::remove_children
 */
void tag_node::remove_children(void)
{
  tag_node *next_node;
  for (tag_node *node = son; node; node = next_node) {
    next_node = node->next;
    delete (node);
  }
  son = last_son = 0;
}

/*-------------------------------------------------------------------
 * vtagman::vtagman
 */
vtagman::vtagman(void)
{
  root = new tag_node;		// dummy tag node
  root->begin = text_coord(1, 0);
  root->end = text_coord(1, 0);
}

/*-------------------------------------------------------------------
 * vtagman::~vtagman
 */
vtagman::~vtagman(void)
{
  clear();
  delete (root);
}

/*-------------------------------------------------------------------
 * vtagman::find_tag
 */
tag_node *vtagman::find_tag(const text_coord &coord, int scope)
{
  if (!root->son) {		// no tag?
    return 0;
  }

  tag_node *node = root;
  while (node->son) {
    tag_node *son;
    for (son = node->son; son; son = son->next) {
      if (son->begin <= coord && coord < son->end) {
	node = son;
	break;
      }
    }
    if (son == 0) {
      if (node == root)
	return (0);
      break;
    }
  }

  for (int i = 0; i < scope && node->parent != root ; i++) {
    node = node->parent;
  }

  assert(node != root);
  return (node);
}

/*-------------------------------------------------------------------
 * vtagman::lookup
 */
void vtagman::lookup(vnode *object, tag_node_list *found)
{
  root->lookup(object, found);
}

tag_node *vtagman::lookup(vnode *object)
{
  tag_node_list found;
  root->lookup(object, &found);
  if (!found.empty()) {
    return (found[0]);
  }
  return (0);
}

/*-------------------------------------------------------------------
 * vtagman::get_parent
 */
tag_node *vtagman::get_parent(tag_node *n)
{
  tag_node *par = n->parent;
  if (par == root) par = 0;
  return (par);
}

/*-------------------------------------------------------------------
 * vtagman::free_tag_node
 */
void vtagman::free_tag_node(tag_node *n)
{
  tag_node *node, *next_node;
  for (node = n->son; node; node = next_node) {
    if (node->son) {
      free_tag_node(node->son);
    }
    next_node = node->next;
    delete (node);
  }
}

/*-------------------------------------------------------------------
 * vtagman::clear
 */
void vtagman::clear(void)
{
  free_tag_node(root);
  root->son = root->last_son = 0;
  root->begin = text_coord(1, 0);
  root->end = text_coord(1, 0);
}

/*-------------------------------------------------------------------
 * vtagman::node_update
 */
void vtagman::node_update(tag_node *changed_node, text_coord &orig_end,
			  const text_coord &new_end)
{
  tag_node *node = changed_node;
  while (1) {
    /* next node */
    while (node->next) {
      node = node->next;
      node_update_helper(node, orig_end, new_end, true);
    }
    /* parent node */
    node = node->parent;
    if (!node) break;
    
    node_update_helper(node, orig_end, new_end, false);
  }
}

void vtagman::node_update_helper(tag_node *node, text_coord &orig_end,
				 const text_coord &new_end,
				 bool descend)
{
  /* update */

  int delta_rows = new_end.row - orig_end.row;
  int delta_cols = new_end.col - orig_end.col;
  if (node->begin >= orig_end) {
    node->begin.row += delta_rows;
    if (node->begin.row == orig_end.row) {
      node->begin.col += delta_cols;
    }
  }
  
  assert(node->end >= orig_end);
  node->end.row += delta_rows;
  if (node->begin.row == orig_end.row) {
    node->end.col += delta_cols;
  }

  if (descend) {
    for (tag_node *child = node->son; child; child = child->next) {
      node_update_helper(child, orig_end, new_end, true);
    }
  }
}

