/*  Call Graph */

/*  Copyright (c) 1995, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#ifndef CG_GRAPH_H
#define CG_GRAPH_H

#include <stdio.h>
#include "basicnodes/basic_forwarders.h"
#include "common/suif_indexed_list.h"
#include "suifkernel/suifkernel_forwarders.h"

class cg_node;

/*  This code builds a call graph for the current fileset.  The nodes
 *  of the call graph have pointers to the proc_sym's of the procedures 
 *  they represent.  As the call graph is built, each procedure is
 *  read in and then flushed from memory.  If "include_externs" is true
 *  then the graph will include nodes for external procedures 
 *  (these procedures will be leaves in the call graph since we can't
 *  scan them to see if they call any other procedures).  Recursive calls
 *  are represented in the graph, thus the graph may have cycles.
 */

typedef void (*cg_map_f)(cg_node *n, void *x);

class cg {
private:
    list< cg_node* > *nds;		// array of nodes 
    cg_node *main;			// main function, first node in graph
    indexed_list< ProcedureSymbol*, cg_node *>
                   _map_procedure_symbol_to_node; // maps procedures to nodes
    bool ext;

    // methods for building the graph 
    void build_call_graph( FileSetBlock *fileset );
    void process_procedure( ProcedureDefinition *ps );
    
public:
    cg( bool include_externs = false );
    virtual ~cg();

    s_count_t num_nodes();
  //    cg_node *node(unsigned i);
    void set_node( s_count_t i, cg_node *n );
    cg_node *get_node( s_count_t i );
    
    cg_node *main_node();
    bool externs_included();   

    void add_node( cg_node *n );
    void set_main_node( cg_node *n );

    void map( cg_map_f f, void *x,
	     bool bottom_up = true,     // otherwise top-down
	     cg_node *start_node = 0); // if NULL, start @ "main"


    virtual cg_node *get_cg_node( ProcedureSymbol *ps );	     
    void print( FILE *fp = stdout );
};


//  initialization and finalization functions 
//
void init_sbrowser_cg( int *argc, char *argv[] );
void exit_sbrowser_cg();


#endif /* CG_GRAPH_H */
