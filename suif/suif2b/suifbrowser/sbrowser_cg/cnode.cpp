/*  Call Graph Node Implementation */

/*  Copyright (c) 1995, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"
#define _MODULE_ "libcg2.a"

#include "cnode.h"
#include "sbrowser_cg.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "common/suif_vector.h"
#include "basicnodes/basic.h"


/* ------------------------------------------------------------------------ 
       Class Implementations
   ------------------------------------------------------------------------ */

//================
//
// Class cg_node
//
//================

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


    /* Public Methods */
cg_node::cg_node( ProcedureSymbol *ps )
{
    set_number( (unsigned)-1 );
    set_parent(0);

    this->psym = ps;
    unknown = 0;
}

cg_node::~cg_node()
{
}


void cg_node::add_pred(cg_node *n)
{
    // avoid duplicate edges
    if ( !is_member(preds(),  n ) ) {
      // add the forward and backward edges
      preds()->push_back( n );
      n->succs()->push_back( this );
    }
}


void cg_node::add_succ( cg_node *n )
{
    // avoid duplicate edges 
    if ( !is_member(succs(), n ) ) {
      // add the forward and backward edges 
      succs()->push_back(n);
      n->preds()->push_back(this);
    }
}


void cg_node::remove_pred( cg_node *n )
{
  int pos = get_member_pos(preds(), n); 
  if (pos != -1) {
    preds()->erase(pos);
  }
  pos = get_member_pos(n->succs(), this); 
  if (pos != -1) {
    n->succs()->erase(pos);
  }
}


void cg_node::remove_succ( cg_node *n )
{
  int pos = get_member_pos(succs(), n); 
  if (pos != -1) {
    succs()->erase(pos);
  }
  pos = get_member_pos(n->preds(), this); 
  if (pos != -1) {
    n->preds()->erase(pos);
  }
}

void
cg_node::toposort(cg_node_list *l, bool reverse /* = false */)
{
    bit_vector mark(100, false);
    
    toposort_helper(l, &mark, reverse );
}

void cg_node::print( FILE *fp )
{
    fprintf( fp, "%s %10s, Node %5u: ", 
                  ( ( parent()->main_node() == this ) ? "-->" : "   " ),
	                get_procedure_symbol()->get_class_name().c_str(), 
	                number() );
    fputs( "  Succs:", fp );
    succs()->print( fp );
    fputs( "  Preds:", fp );
    preds()->print( fp );
}

/* Private methods */

void cg_node::toposort_helper( cg_node_list *l, bit_vector *mark,
			      bool reverse /* = false */ )
{
    (*mark)[number()] = true;
    succs()->toposort_helper( l, mark, reverse );

    if ( reverse ) l->push_back( this );
    else         l->push_back( this );
}


//====================
//
// Class cg_node_list
//
//====================

    /* Public methods */
void cg_node_list::remove_node( cg_node *n )
{
   list<cg_node*>::iterator begin = this->begin(),
                            end   = this->end();
   bool found = false;
   for (list<cg_node*>::iterator it = begin; it != end; it++) {
      if (*it == n) {
         found = true;
	 erase(it);
	 break;
      }
   }
   if (!found)
      suif_assert_message( false,
           ("remove_node - node '%u' not found", n->number() ));
}

void cg_node_list::print( FILE *fp )
{
    list<cg_node *>::iterator begin = this->begin(),
                             end   = this->end();
    for (list<cg_node*>::iterator it = begin; it != end; it++) {
   	cg_node *n = *it;
	fprintf( fp, " %u", n->number() );
    }
}


/* Private methods */

void cg_node_list::toposort_helper(cg_node_list *l, bit_vector *mark, 
				   bool reverse /* = false */ )
{
    list<cg_node *>::iterator begin = this->begin(),
                             end   = this->end();
    for (list<cg_node*>::iterator it = begin; it != end; it++) {
   	cg_node *n = *it;
	if (!(*mark)[n->number()]) { 
	   n->toposort_helper( l, mark, reverse );
        }
    }
}
