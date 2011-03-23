/*--------------------------------------------------------------------
 * code_tree.h
 *
 */

#ifndef CODE_TREE_H
#define CODE_TREE_H

#include "visual/visual.h"

#include "suif_vnode.h"
#include "suifkernel/suifkernel_forwarders.h"

class code_fragment {
  friend class code_tree;

protected:
  int f_line;
  int l_line;
  SuifObject *tn;

  code_fragment *son;
  code_fragment *last_son;
  code_fragment *next;

  void free_subtree();

public:
  code_fragment( SuifObject *node );
  ~code_fragment();

  code_fragment *child() { return son; }
  code_fragment *last_child() { return last_son; }
  code_fragment *next_sib() { return next; }


  void select( vtext* text, bool add = false ) {
    vnode* vn = create_vnode( tn );
    text->select( vn, add );

    for ( code_fragment* son = this->son; son; son = son->next ) {
      son->select( text, true );
    }
  }


  void select_code_fragments( vtext* text, bool sub_nodes = true ) {
    vnode* vn = vman->find_vnode( this );
    if ( vn ) {
      text->select( vn, sub_nodes );
    }

    if ( sub_nodes ) {
      for ( code_fragment* son = this->son; son; son = son->next ) {
        son->select( text, sub_nodes );
      }
    }
  }


  void set_node( SuifObject * node ) { tn = node; }
  SuifObject *node() { return tn; }
  int first_line() { return f_line; }
  int last_line() { return l_line; }
  void set_first_line( int line ) { f_line = line;}
  void set_last_line( int line ) { l_line = line; }

  void add_son(code_fragment *f);
  code_fragment *lookup(SuifObject *tn);
};

struct code_range {
  int first_line;
  int last_line;

  code_range(int first, int last) {
    first_line = first;
    last_line = last;
  }
};

typedef code_range (*map_tn_fn)(SuifObject *tn, void *client_data);

class code_tree {
  code_fragment *root;

  map_tn_fn map_fn;
  void *client_data;

  code_fragment *map_to_source( SuifObject* tn, code_fragment* );
  static void create_tags(code_fragment* f, tag_node *parent_tag);

  static void print_helper(FILE *fd, code_fragment *f, int depth);

public:
  code_tree();
  virtual ~code_tree();

  void *id;   // for attaching additional identifying info

  code_fragment *get_root() { return root; }
  void set_map_fn( map_tn_fn fn, void* data ) {
    map_fn = fn; client_data = data;
  }

  void build( SuifObject* z );
  virtual code_fragment* lookup( SuifObject* tn );

  void create_tags( vtext* text );
//@@@  void suif_to_code_prop(vprop *suif_prop, vprop *code_prop);

  void print( FILE* fd );

};

#endif // CODE_TREE_H
