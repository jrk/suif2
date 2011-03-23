// $Id: class_hierarchy.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "common/suif_list.h"
#include "suifnodes/suif.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifutilities/problems.h"

#include "osuifutilities/class_hierarchy.h"


extern void print_subclass_hierarchy( ClassType* ctype,
				      int delta,
				      int offset )
{
  suif_assert( ctype );

  for( int i=0; i<offset ; i++ )
    cout << " ";
  cout << ctype->get_name().c_str() << endl;
  
  Iter<InheritanceLink *> iter = ctype->get_child_classe_iterator();
  while( iter.is_valid() ){
    InheritanceLink* ilink = iter.current();
    suif_assert( ilink->get_parent_class_type() == ctype );
    print_subclass_hierarchy( ilink->get_child_class_type(),
			      delta,
			      offset+delta );    
    iter.next();
  }
}


extern void print_superclasses( SingleInheritanceClassType* ctype ) {
  if( ctype == NULL ) return;

  cout << ctype->get_name().c_str() << endl;
  print_superclasses( ctype->parent_class() );
}



void SingleInheritanceClassHierarchy::
build_type_hierarchy( Iter<SymbolTableObject* > input_stos) {
  list<ClassType *> waiting_types_list;
  
  /* add all roots to the final list */
  while( input_stos.is_valid() ) {
    SymbolTableObject* cur_sto = input_stos.current();
    input_stos.next();
        
    if( cur_sto->isKindOf(ClassType::get_class_name()) ) {
      SingleInheritanceClassType* current_type =
	to<SingleInheritanceClassType>(cur_sto);
            
      if( current_type->parent_class() == NULL ) {
	/* this is a root so add it to the result */
	add_type( current_type );
              
	/* now add descendants to the wait_list */
	Iter<InheritanceLink *> child_class_iter =
	  current_type->get_child_classe_iterator();

	while( child_class_iter.is_valid() ){
	  InheritanceLink* childLink = child_class_iter.current();
	  child_class_iter.next();

	  waiting_types_list.push_back(childLink->get_child_class_type());
	}	
      }
    }
  }

  if( waiting_types_list.empty() )
    return;
  
  /* now do a bf walk on the graph */
  list<ClassType* >::iterator iter =
    waiting_types_list.begin();
  while( iter != waiting_types_list.end() ) {
    ClassType* const & current_type = (*iter);

    Iter<InheritanceLink *> child_class_iter = 
      current_type->get_child_classe_iterator();
    while( child_class_iter.is_valid() ){
      InheritanceLink *child_link = child_class_iter.current();
      child_class_iter.next();

      waiting_types_list.push_back( child_link->get_child_class_type() );
    }
    
    iter ++;
  }

  /* at this point waiting_types_list contains the rest of the bfwalk list*/
  /* just append this list to the result list */  
  iter = waiting_types_list.begin();
  while( iter != waiting_types_list.end() ) {
    add_type( (*iter) );
    iter ++;
  } 
}


void SingleInheritanceClassHierarchy::
add_type( ClassType* the_type ) {
  _type_list.push_back( to<SingleInheritanceClassType>(the_type) );
}


list<SingleInheritanceClassType *> *
SingleInheritanceClassHierarchy::type_hierarchy_list() {
  return & _type_list;
}


list<SingleInheritanceClassType *> *SingleInheritanceClassHierarchy::
type_hierarchy_list( SingleInheritanceClassType* the_member )
{
  OsuifProblems::not_implemented();
  return NULL;
}


