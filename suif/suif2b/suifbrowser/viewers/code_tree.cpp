/*-------------------------------------------------------------------
 * code_tree.cc
 *
 */

#include "code_tree.h"
#include <stdlib.h>
#include <limits.h>
#include "suif_utils.h"
#include "suif_vnode.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"


/*--------------------------------------------------------------------
 * code_fragment::code_fragment
 */
code_fragment::code_fragment(SuifObject* n)
{
  son = last_son = next = 0;
  f_line = l_line = 0;
  tn = n;
}

/*--------------------------------------------------------------------
 * code_fragment::~code_fragment
 */
code_fragment::~code_fragment() {
  free_subtree();
}

/*--------------------------------------------------------------------
 * code_fragment::add_son
 */
void code_fragment::add_son( code_fragment* f ) {
  if ( !f ) return;
  if (last_son) {
    last_son->next = f;
  } else {
    son = f;
  }
  last_son = f;
}

/*--------------------------------------------------------------------
 * code_fragment::lookup
 *
 * look for a tree node in code_fragment subtree
 */

code_fragment *
code_fragment::lookup(SuifObject *n)
{
  // helper function for lookup !! @@@ ??
  if (tn == n) return this;

  for (code_fragment *child = son; child; child = child->next) {
    code_fragment *found = child->lookup(n);
    if ( found ) return found;
  }
  return 0;
}

/*--------------------------------------------------------------------
 * code_fragment::free_subtree()
 *
 */
void
code_fragment::free_subtree()
{
  code_fragment *next_child;
  for (code_fragment *child = son; child; child = next_child) {
    next_child = child->next;
    delete child;
  }
  son = last_son = 0;
}

/*--------------------------------------------------------------------
 * code_tree::code_tree
 */
code_tree::code_tree() {
  root = new code_fragment(0);
  map_fn = 0;
}

/*--------------------------------------------------------------------
 * code_tree::~code_tree
 *
 */
code_tree::~code_tree() {
  delete root;
}

/*--------------------------------------------------------------------
 * code_tree::clear
 *
 */
//@@@void code_tree::clear() {
//@@@}

/*--------------------------------------------------------------------
 * code_tree::build
 *
 */
#define XXX__
#ifdef XXX__
void code_tree::build( SuifObject *proc ) {
  root->add_son(  map_to_source( proc, 0 ) );
}

/*--------------------------------------------------------------------
 * code_tree::map_to_source
 *
 * Create the map between the source code and suif nodes
 *
 */
code_fragment *
code_tree::map_to_source(SuifObject *tn, code_fragment*)
{
  if (tn->isKindOf(SymbolTable::get_class_name())) return 0;
  // ====== map the input SuifObject =====
  code_range r = (*map_fn)( tn, client_data );

  int min_line, max_line;

  code_fragment* current_f = new code_fragment( tn );
  min_line = r.first_line;
  max_line = r.last_line;

#if TO_BE_SETTLED
  // ====== map the child zots =====
  for ( s_count_t _cnt=0; _cnt<tn->num_components(); _cnt++ ) { //
    SuifBrick br=tn->component(_cnt);     //  go over all owned zots
    if ( !br.is_owned_zot() ) continue;                         //
    SuifObject *child = br.get_zot();                                  //
    if ( !child ) continue;

    code_fragment* child_fragment = map_to_source( child, 0 );

    //  propagate line number info
    if ( child_fragment ) {
      min_line =
         min_line < child_fragment->f_line ? min_line : child_fragment->f_line;
      max_line =
         max_line > child_fragment->l_line ? max_line : child_fragment->l_line;

      // check for overlapping
      for ( code_fragment* son = current_f->child(); son; son = son->next_sib())
      {
        int a = child_fragment->f_line,
	    b = child_fragment->l_line,
	    c = son->f_line, d = son->l_line;
       if (( a==c) && ( b == d))
	 //	if ( ( (a<=c) && (b<=d) && (c<=b) ) ||
	 //     (c<=a) && (d<=b) && (a<=d) )
	  // the two code_fragments overlap
       {
	 son->f_line = a < c ? a : c;
	 son->l_line = b > d ? b : d;

          son->add_son( child_fragment );

          child_fragment = 0;
          break;
        }
      }
      if ( child_fragment ) {
        // no overlapping
        current_f->add_son( child_fragment );
      }
    }
  }
#endif // TO_BE_SETTLED
  if ( max_line ) {
    current_f->f_line = min_line;
    current_f->l_line = max_line;
  } else {
    delete current_f;
    current_f = 0;
  }

  return current_f;
}

#else

void code_tree::build( SuifObject *proc ) {
  map_to_source(proc, root);
}

/*--------------------------------------------------------------------
 * code_tree::map_to_source
 *
 * Create the map between the source code and suif nodes
 *
 */
code_fragment *code_tree::map_to_source( SuifObject *tn,
					 code_fragment *parent ) {
  // ====== map the input zot =====
  code_range r = (*map_fn)( tn, client_data );

  code_fragment* current_f = new code_fragment( tn );
  current_f->f_line = r.first_line;
  current_f->l_line = r.last_line;

  // ====== map the child zots =====
  for ( s_count_t _cnt=0; _cnt<tn->num_components(); _cnt++ ) { //
    brick br=tn->component(_cnt);                               //  go over all owned zots
    if ( !br.is_owned_zot() ) continue;                         //
    SuifObject *child = br.get_zot();                                  //
    if ( !child ) continue;

    code_fragment *child_fragment = map_to_source( child, current_f );

    //  propagate line number info
    if ( child_fragment ) {
      current_f->f_line = current_f->f_line < child_fragment->f_line ? current_f->f_line : child_fragment->f_line;
      current_f->l_line = current_f->l_line > child_fragment->l_line ? current_f->l_line : child_fragment->l_line;
    }
  }

  // if it has line number info
  if ( current_f->l_line
) {
    parent->add_son( current_f );
  } else {
    delete current_f;
    current_f = 0;
   }

  return current_f;
}

#endif


/*--------------------------------------------------------------------
 * code_tree::lookup
 *
 * lookup the code_fragment that corresponds with a tree node
 */

code_fragment *
code_tree::lookup(SuifObject *tn)
{
  ProcedureDefinition *proc = get_procedure_definition(tn);
  for ( code_fragment *proc_f = root->son; proc_f; proc_f = proc_f->next ) {
    if (proc_f->tn == proc) {
      /* this is the procedure, now, look for the code_fragment! */
      return (proc_f->lookup(tn));
    }
  }
  return 0;
}

/*--------------------------------------------------------------------
 * create_tags
 *
 */
void code_tree::create_tags( vtext *text ) {
  for (code_fragment *f = get_root()->son; f; f = f->next) {
    create_tags(f, text->root_tag());
  }
}


void code_tree::create_tags( code_fragment *f, tag_node *parent_tag ) {
  assert( f->tn != 0 );
  vnode *vn = vman->find_vnode(f);
  if (!vn) vn = new vnode(f, tag_code_fragment);

  tag_node *tag = new tag_node;
  tag->set_object(vn);
  tag->set_begin_coord(text_coord(f->f_line, 0));
  tag->set_end_coord(text_coord(f->l_line + 1, 0));
  parent_tag->add_son(tag);

  /* children */
  for (code_fragment *child = f->son; child; child = child->next) {
      create_tags(child, tag);
  }
}

/*--------------------------------------------------------------------
 * print
 *
 */
void code_tree::print( FILE *fd ) {
  for (code_fragment *f = get_root()->son; f; f = f->next) {
    fprintf( fd, "Proc '%s'\n",
    get_procedure_definition(f->tn)->
         get_procedure_symbol()->get_name().c_str());
    print_helper( fd, f, 0);
  }
}

void
code_tree::print_helper(FILE *fd, code_fragment *f, int depth)
{
  for ( int i = 0; i<depth; i++ ) fprintf( fd, " ");
  fprintf( fd, "- (first: %d, last: %d, f = %p tn = %p %s)\n",
	  f->f_line, f->l_line, f, f->tn, f->tn->get_class_name().c_str() );

  for (code_fragment *child = f->son; child; child = child->next) {
    print_helper(fd, child, depth + 1);
  }
}



#ifdef AG
/*--------------------------------------------------------------------
 * create properties on code fragments
 *
 */
void code_tree::suif_to_code_prop(vprop *suif_prop, vprop *code_prop)
{
  code_prop->erase();

  vnode_list_iter iter (suif_prop->get_node_list());
  while (!iter.empty()) {

    /* find the corresponding code_fragment */
    vnode *vn = iter.step();
    char *tag = vn->get_tag();
    if (tag == tag_suif_object) {
      suif_object *obj = (suif_object *) vn->get_object();
      tree_node *tn = 0;

      switch (obj->object_kind()) {
      case TREE_OBJ:
	{
	  tn = (tree_node *) obj;
	}
	break;
      case INSTR_OBJ:
	{
	  tn = ((instruction *) obj)->parent();
	}
      default:
	break;
      }

      if (tn) {
	code_fragment *f = lookup(tn);
	if (f) {
	  vnode *vn = vman->find_vnode(f);
	  if (vn) code_prop->add_node(vn);
	}
      }
    }
  } /* end while */
}

#endif
























