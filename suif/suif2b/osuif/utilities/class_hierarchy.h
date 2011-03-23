// $Id: class_hierarchy.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFUTILITIES__CLASS_HIERARCHY_H
#define OSUIFUTILITIES__CLASS_HIERARCHY_H

#include "common/suif_list.h"
#include "suifnodes/suif.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"


extern void print_subclass_hierarchy( ClassType* ctype,
				      int delta = 2,
				      int offset = 0 );


extern void print_superclasses( SingleInheritanceClassType* ctype );



/* This class builds and manages alist containing the class hierarchy for a
 * given list of stos.
 *
 *  void build_type_hierarchy( ro_tos_ref<sto* > input_stos)
 *  
 *      Builds the type hierarchy from the input list of stos. The input list
 *      can contain any kind of stos. For efficiency reasons the builder works
 *      as follows :
 *        - considering the inheritance graph as a collection of dags, the
 *          roots are enumerated first
 *        - next we have a parallel breadth first walk through all the dags,
 *          all levels at the same depth being enumerated in the same step.
 *       
 *
 *      Complexity is O(n)
 *
 *      OUTPUT : a list of types linked in the inheritance order, i.e if the
 *               input contained   A->B->C, D, F (let's say in order B,D,A,F,C)
 *               the output is D, A, F, B, C.
 *
 *  virtual ro_tos_ref<class_type *> type_hierarchy_list()
 *     
 *     Returns the entire type hierarchy list
 *
 * virtual ro_tos_ref<class_type *> type_hierarchy_list( class_type
 * the_member)
 *
 *  Given the input type, returns a list with the inheritance path, root
 *  being the first element.
 */
class SingleInheritanceClassHierarchy {
private:
  list<SingleInheritanceClassType *> _type_list;
  
public:
  SingleInheritanceClassHierarchy() { }
  virtual ~SingleInheritanceClassHierarchy() { }

  virtual void build_type_hierarchy( Iter<SymbolTableObject* > input_stos);
  virtual list<SingleInheritanceClassType *> * type_hierarchy_list();

  virtual list<SingleInheritanceClassType *> *
  type_hierarchy_list( SingleInheritanceClassType* the_member );
  
  void add_type( ClassType* the_type );
};


#endif
