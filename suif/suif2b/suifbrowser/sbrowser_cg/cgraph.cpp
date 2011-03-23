/*  Call Graph Implementation */



/*  Copyright (c) 1995, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#define _MODULE_ "libcg2.a"

#include <iostream.h>
#include "iokernel/cast.h"
#include "sbrowser_cg.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifnodes/suif.h"
//#include "viewers/suif_utils.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/group_walker.h"
#include "utils/expression_utils.h"
#include "cfeutils/cexpr_utils.h"
#include "cfenodes/cfe.h"
// For things like getting procedure symbol from CallExpression.


extern SuifEnv *suif_env;

/* ------------------------------------------------------------------------
       Class Implementations
   ------------------------------------------------------------------------ */

//===========
//
// Class cg
//
//===========

    /* Public methods */

cg::cg(bool include_externs)
{
    //nds = new array_tos< cg_node* >;
    nds = new list< cg_node* >;
    set_main_node(0);
    ext = include_externs;
    build_call_graph( suif_env->get_file_set_block() );
}


cg::~cg()
{
    for ( s_count_t i = 0; i < nds->size() ; i++ ) {
      //cg_node *n =  nds->elem_by_num( i );
      cg_node *n =  (*nds)[i];
      delete n;
    }
    delete nds;
}


s_count_t cg::num_nodes()
{
    return nds->size();
}


//cg_node *cg::node( unsigned i)		
//{
 //return *(nds)[i];
//}


void cg::set_node( s_count_t i, cg_node *n)
{
    //nds->set_elem_by_num( i, n );
    (*nds)[i] = n;
}


cg_node *cg::get_node( s_count_t i )	
{
  //return nds->elem_by_num( i );
  return (*nds)[i];
}


cg_node *cg::main_node()		
{
    return main;
}


bool cg::externs_included()
{
    return ext;
}



void cg::add_node(cg_node *n)
{
    s_count_t num = nds->size();
    n->set_number( num );
    n->set_parent( this );
    nds->push_back( n );
}


void cg::set_main_node(cg_node *n)
{
    main = n;
}


void cg::print(FILE *fp)
{
    fprintf(fp, "** Call graph **: %d nodes\n", num_nodes());

    for ( s_count_t i = 0; i < num_nodes() ; i++) {
       get_node( i )->print( fp );
       fputs("\n", fp);
    }
}

/*****************************************************************************/
//
// Iterate over the nodes in the call graph, in either pre-order or post-order
//

void cg::map(cg_map_f f, void *x, bool bottom_up, cg_node *start_node)
{
    cg_node *first_node = start_node;

    if ( !first_node ) first_node = main_node();
      suif_assert_message(first_node != 0,
                          ("No starting node found in cg::map"));

    // Put nodes into topological (top-down) or
    // reverse-topological (bottom-up) list
    //
    cg_node_list *topolist = new cg_node_list;
    first_node->toposort( topolist, bottom_up );

    // Now call "f" on nodes in sorted list
    list<cg_node*>::iterator begin = topolist->begin();
    list<cg_node*>::iterator end = topolist->end();
    for (list<cg_node*>::iterator cg_iter = begin;
           cg_iter != end; cg_iter++)
    {
      f(*cg_iter, x );
    }
    delete topolist;
}

/*****************************************************************************/

    /* Private methods */

//
// Build a new call graph.
//
class FindCalleesWalker : public SelectiveWalker
{
  cg_node *_current_procedure;
  cg * _cg;
public:

  FindCalleesWalker(SuifEnv *suif_env, cg *cg_context):
    SelectiveWalker(suif_env, ScopedObject::get_class_name()),
    _current_procedure(0), _cg(cg_context)  {} 

  virtual ApplyStatus operator () (SuifObject *x) {
    //if (!x)
       //return Truncate;
    //if (is_kind_of<BasicSymbolTable>(x))
       //return Truncate;
    if (is_kind_of<CallExpression>(x)) {
       if ( !_current_procedure ) return Continue;
       CallExpression *cal = to<CallExpression>(x);
       ProcedureSymbol *target = get_procedure_target_from_call_expression(cal);
       if ( target ) {
          //cout << "Calls - " << target->get_name().c_str() << "\n";
          if ( (!(target->get_definition()== 0)) || _cg->externs_included() ) {
	     cg_node *callee_node = _cg->get_cg_node( target );
	     _current_procedure->add_succ( callee_node );
          }
       } else {
         _current_procedure->inc_unknowns( 1 );
       }
       //inherited::handle_call_instruction( cal );
    }
    else if (is_kind_of<ProcedureDefinition>(x)) {
       ProcedureDefinition *pb = to<ProcedureDefinition>(x);
       // cg_node *old_current_procedure = _current_procedure;
       _current_procedure = _cg->get_cg_node( pb->get_procedure_symbol() );

       // Check if this is the main procedure
       //
       ProcedureSymbol *ps = pb->get_procedure_symbol();
       //cout << "ProcedureDefinition - " << ps->get_name().c_str() << endl;
       if ( ( strcmp(ps->get_name().c_str(), "main")   == 0 ) ||
	      strcmp(ps->get_name().c_str(), "MAIN__") == 0 ) {
         suif_assert_message( _cg->main_node() == 0,
                 ("Already found main procedure before scanning %s",
		 ps->get_name().c_str() ));
         _cg->set_main_node( _current_procedure );
       }

       //_current_procedure = old_current_procedure;
    }
    return Continue;
  }
};

void
cg::build_call_graph(FileSetBlock* fileset)
{
  assert( fileset );
  // Traverse the fileset looking for calls
  FindCalleesWalker walker(suif_env, this);
  fileset->walk(walker);
}
//
// Get the cg_node associated with "ps".  Check if a cg_node has already
// been created by looking in the map. Otherwise,
// create the node and add it to "call_graph".
//
cg_node *cg::get_cg_node( ProcedureSymbol *ps )
{
  cg_node *curr_node;

  if ( _map_procedure_symbol_to_node.is_member( ps ) ) {
    curr_node = (*(_map_procedure_symbol_to_node.find(ps))).second;
  } else {
    curr_node = new cg_node( ps );
    add_node( curr_node );
    _map_procedure_symbol_to_node.push_back(ps, curr_node);
  }
  return curr_node;
}
/* ------------------------------------------------------------------------
       External functions implementation
   ------------------------------------------------------------------------ */

void init_sbrowser_cg(int * /* argc */, char * /* argv */ []) {
}

void exit_sbrowser_cg() { }
