/*--------------------------------------------------------------------
 * vtagman.h
 *
 */

#ifndef VTAGMAN_H
#define VTAGMAN_H

#include "common/suif_list.h"
#include "vtty.h"

struct text_coord {
  int row;
  int col;

  text_coord() {}
  text_coord(int r, int c) {
    row = r;
    col = c;
  }
  bool operator<= (const text_coord &t) const {
    return (row < t.row || (row == t.row && col <= t.col));
  }
  bool operator< (const text_coord &t) const {
    return (row < t.row || (row == t.row && col < t.col));
  }
  bool operator>= (const text_coord &t) const {
    return (row > t.row || (row == t.row && col >= t.col));
  }
  bool operator> (const text_coord &t) const {
    return (row > t.row || (row == t.row && col > t.col));
  }
  bool operator== (const text_coord &t) const {
    return (row == t.row && col == t.col);
  }
};

struct tag_state {
  bool expanded;
};

class tag_node;
class vtext;
class vnode;
class tag_node;

typedef list<tag_node*> tag_node_list;


class tag_node {

 private:
  friend class vtagman;
  friend class vtext;

  vnode *object;
  text_coord begin;
  text_coord end;

  tag_node *son;
  tag_node *last_son;
  tag_node *next;
  tag_node *parent;		// its parent in the tag tree

  print_fn pr_fn;		// print fn to print more/less detail
  void *client_data;		// used in the print fn
  int depth;

  tag_state state;

 public:
  tag_node(void);
  vnode *get_object(void) { return object; }
  void set_object(vnode *obj) {object = obj; }

  /* text coordinates */
  text_coord &get_begin_coord(void) { return begin; }
  text_coord &get_end_coord(void) { return end; }
  void set_begin_coord(text_coord coord) { begin = coord; }
  void set_end_coord(text_coord coord) { end = coord; }

  /* get misc info */
  int get_depth(void);

  /* expansion */
  void set_print_fn(print_fn pr_fn, int d, void *client_data);
  void expand(vtty *text) {
    state.expanded = true;
    (*pr_fn)(text, object, depth, PRINT_FULL, client_data);
  }
  void collapse(vtty *text) {
    state.expanded = false;
    remove_children();
    (*pr_fn)(text, object, depth, PRINT_BRIEF, client_data);
  }

  /* query state */
  bool is_expandable(void) { return (pr_fn != 0); }
  bool is_expanded(void) { return (!pr_fn || state.expanded); }

  /* look up object in this subtree */
  void lookup(vnode *object, tag_node_list *found);

  /* children */
  void add_son(tag_node *node);
  void remove_children(void);
};

class vtagman {
 private:
  tag_node *root;

  void free_tag_node(tag_node *node);

  static void node_update_helper(tag_node *changed_node, text_coord &begin,
				 const text_coord &end, bool descend);

 public:
  vtagman(void);
  ~vtagman(void);

  void clear(void);

  tag_node *get_root(void) { return root; }

  tag_node *find_tag(const text_coord &coord, int scope = 0);
  void lookup(vnode *obj, tag_node_list *found);
  tag_node *lookup(vnode *obj);
  tag_node *get_parent(tag_node *node);

  void node_update(tag_node *changed_node,
		     text_coord &begin, const text_coord &end);
};



#endif
