// $Id: search_utils.h,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#ifndef OSUIFUTILITIES__SEARCH_UTILS_H
#define OSUIFUTILITIES__SEARCH_UTILS_H

#include "iokernel/cast.h"
#include "suifkernel/suif_env.h"
#include "common/suif_list.h"
#include "basicnodes/basic.h"
#include "osuifnodes/osuif.h"

/**
 * Iterates over all entries in BasicSymbolTable and
 * collects the SymbolTableObject of kind 'SearchT'
 * into a list of kind 'RetT'.
 */
template<class RetT, class SearchT>
list<RetT* > collect_stos( BasicSymbolTable* symtab ) {
  list<RetT* > hit_list;
  Iter<SymbolTableObject*> iter =
    symtab->get_symbol_table_object_iterator();

  while( iter.is_valid() ) {
    SymbolTableObject* sto = iter.current();

    if ( sto->isKindOf( SearchT::get_class_name() ) ) {
      hit_list.push_back( to<RetT>(sto) );
    }
    iter.next();
  }

  return hit_list;
}


template<class SearchT>
list<SymbolTableObject* > collect_stos( BasicSymbolTable* symtab ) {
  return collect_stos<SymbolTableObject, SearchT>( symtab );
}


/**
 * Look up the arguments parent chain until a parent of
 * type 'SearchT' is found.
 * If no such parent exists NULL is returned.
 */
template<class SearchT>
SearchT* find_ancestor( SuifObject* obj ) {
  if ( obj == NULL) return NULL;

  SuifObject* hit = obj->get_parent();
  while ( hit != NULL &&
	  !hit->isKindOf( SearchT::get_class_name() ) ) {
    hit = hit->get_parent();
  }
	  
  return to<SearchT>( hit );
}


/**
 * Gurantees to returns a FileBlock.
 * If 'obj' is (transitively) owned by a FileBlock,
 * then this block is returned. Otherwise a random
 * FileBlock is returned. If no FileBlock exits, a new
 * one is created.
 */
extern FileBlock* get_associated_file_block( SuifObject* obj );


/**
 * Returns the ClassType that conceptually owns 'obj.
 * If no such ClassType can be infered, NULL is returned.
 */
extern ClassType* get_owning_class( SuifObject* obj );

#endif /* OSUIFUTILITIES__SEARCH_UTILS_H */
