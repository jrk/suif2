/*  Call Graph Nodes */

/*  Copyright (c) 1995, 1997 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include "common/suif_copyright.h"

#include <stdio.h>
#include "common/suif_list.h"
#include "basicnodes/basic_forwarders.h"
#include "cgraph.h"

#ifndef CG_NODE_H
#define CG_NODE_H

/*
 *  Each call graph node contains only an ID number and information
 *  related to the call graph.  Other information can be associated with
 *  a node by storing it in a separate array where the node ID number
 *  is used to index the array.
 */

class cg_node;

class cg_node_list : public list<cg_node*> 
{
public:
    void toposort_helper( cg_node_list *l, bit_vector *mark, bool reverse = false );

    void remove_node( cg_node *n );
    void print( FILE *fp = stdout );
};


class cg_node {
private:
    unsigned nnum;		// node number 
    cg *par;			// parent call graph
    cg_node_list prs;		// predecessors
    cg_node_list scs;		// successors 
    ProcedureSymbol *psym;     // procedure_symbol this node represents 
    int unknown;                // number of calls to unknown procs

   public:
    void toposort_helper( cg_node_list *l, bit_vector *mark, bool reverse = false );
  
    cg_node(ProcedureSymbol *ps);
    ~cg_node();
    
    unsigned number()			      { return nnum;    }
    cg *parent()			      { return par;     }
    ProcedureSymbol *get_procedure_symbol()  { return psym;    }
    int unknown_callees()                     { return unknown; }
  
    cg_node_list *preds()		      { return &prs;    }
    cg_node_list *succs()		      { return &scs;    }
    void inc_unknowns(int val = 1)            { unknown += val; }

    void add_pred(cg_node *n);
    void add_succ(cg_node *n);
    void remove_pred(cg_node *n);
    void remove_succ(cg_node *n);

    void toposort(cg_node_list *l, bool reverse = false);

    void print(FILE *fp=stdout);
    
    void set_number(unsigned n)		{ nnum = n; }
    void set_parent(cg *p)		{ par = p; }
    
  };

#endif /* CG_NODE_H */
