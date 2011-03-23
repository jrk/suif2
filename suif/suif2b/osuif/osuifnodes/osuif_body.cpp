// $Id: osuif_body.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#include <string.h>

#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/union_meta_class.h"
#include "iokernel/clone_stream.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "iokernel/object_factory.h"
#include "iokernel/virtual_iterator.h"
#include "suifkernel/real_object_factory.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/module.h"
#include "basicnodes/basic_factory.h"


void init_osuif_cloning(CloneSubSystem * css) {
  // nothing to be done.
};


/*
 * Generic method to check if two arbitrary types are
 * equivalent.
 */
bool MethodType::is_equivalent(Type* type1, Type* type2) {
  return type1==type2;
}


bool MethodType::is_equivalent( MethodType* mType ) {
  suif_assert( false );
  return false; // never reached
}

bool StaticMethodType::is_equivalent(Type* type1, Type* type2) {
  return MethodType::is_equivalent( type1, type2 );
}


/* 
 * Checks if the two static method types are equivalent.
 *
 * Two methods are equivalent if all the formal parameter types
 * are type equivalent. All return types must be equivalent.
 */
bool StaticMethodType::is_equivalent( MethodType* mType ) {
  suif_assert( is_kind_of<StaticMethodType>(mType) );

  int argument_count1 = get_argument_count();
  int argument_count2 = mType->get_argument_count();

  if(argument_count1 != argument_count2)
    return false;

  //check if the argument lists are the same
  Iter<QualifiedType*> argumentIter1 = get_argument_iterator();
  Iter<QualifiedType*> argumentIter2 = get_argument_iterator();
  
  while(argumentIter1.is_valid()){
    if(! argumentIter2.is_valid())
      return false;
    QualifiedType* argument1 = argumentIter1.current();
    QualifiedType* argument2 = argumentIter2.current();
    
    if ( ! is_equivalent( argument1, argument2 ) )
      return false;

    argumentIter1.next();
    argumentIter2.next();
  }

  DataType *result1 = get_result_type();
  DataType *result2 = get_result_type();

  return is_equivalent( result1, result2 );
}


/*
 * Get the  receiver type which is the first argument in the list.
 */
QualifiedType* InstanceMethodType::get_receiver_type(){
  Iter<QualifiedType*> argumentIter = get_argument_iterator();

  return argumentIter.current();
}


/*
 * Sets the first argument to the receiver type argument.
 */
void InstanceMethodType::set_receiver_type(QualifiedType* receiver){
  //insert the receiver to argument list at position zero
  insert_argument(0, receiver);
}


bool InstanceMethodType::is_equivalent(Type* type1, Type* type2) {
  return MethodType::is_equivalent( type1, type2 );
}

/* 
 * Checks if the two instance method types are equivalent.
 *
 * Two methods are equivalent if all the formal parameter types except
 * the first one are type equivalent. All return types must be
 * equivalent
 */
bool InstanceMethodType::is_equivalent( MethodType* mType ) {
  suif_assert( is_kind_of<InstanceMethodType>(mType) );

  int argument_count1 = get_argument_count();
  int argument_count2 = mType->get_argument_count();

  if(argument_count1 != argument_count2)
    return false;

  //check if the argument lists are the same
  Iter<QualifiedType*> argumentIter1 = get_argument_iterator();
  Iter<QualifiedType*> argumentIter2 = get_argument_iterator();

  //go past the first the argument which is the receiver
  argumentIter1.next();
  argumentIter2.next();

  while(argumentIter1.is_valid()){
    if(! argumentIter2.is_valid())
      return false;
    Type* argument1 = argumentIter1.current();
    Type* argument2 = argumentIter2.current();

    if ( ! is_equivalent( argument1, argument2 ) )
      return false;

    argumentIter1.next();
    argumentIter2.next();
 }

  Type *result1 = get_result_type();
  Type *result2 = get_result_type();

  return is_equivalent( result1, result2 );
}


/*
 * The equivalence of an instance method symbols is defined as
 * - same name
 * - method type must be equivalent
 */
bool InstanceMethodSymbol::is_equivalent(InstanceMethodSymbol* msym) {
  if(get_name() != msym->get_name())
    return false;

  InstanceMethodType* mtype1 =
    to<InstanceMethodType>( get_type() );
  InstanceMethodType* mtype2 =
    to<InstanceMethodType>( msym->get_type() );

  return mtype1->is_equivalent( mtype2 );
}


/*
 * Checks if the current InstanceMethodSymbol redefines 'msym'.  The
 * overridden_method link is followed until 'msym' is found or the
 * link is NULL (in which case false is returned).
 */
bool InstanceMethodSymbol::overrides(InstanceMethodSymbol* msym) {
  suif_assert( msym );

  InstanceMethodSymbol* overridden_msym= this;

  while( overridden_msym==NULL || overridden_msym==msym ) {
    overridden_msym= overridden_msym->get_overridden_method();
  }

  return !(msym == NULL);
}


bool ClassType::is_ancestor_class(ClassType* ancestor_class){
  Iter<InheritanceLink *> parent_class_iter = get_parent_classe_iterator();
    
  while( parent_class_iter.is_valid() ) {
    InheritanceLink* parentLink = parent_class_iter.current();

    if(parentLink->is_ancestor_class(ancestor_class))
      return true;

    parent_class_iter.next();
  }

  return false;
}


bool ClassType::is_descendant_class(ClassType* descendant_class){
  Iter<InheritanceLink *> child_class_iter = get_child_classe_iterator();

  while(child_class_iter.is_valid()){
    InheritanceLink* childLink = child_class_iter.current();

    if(childLink->is_descendant_class(descendant_class))
      return true;

    child_class_iter.next();
  }

  return false;
}

/*
 *The trivial test of checking with parent is done here. To further perform
 *the transitve closure, the call is forwarded with the parent_class_type
 *instance variable
*/
bool InheritanceLink::is_ancestor_class(ClassType* the_ancestor_class) const{

  if(the_ancestor_class == _parent_class_type);
    return true;

  //forward the call using the parent_class_type reference
  return _parent_class_type->is_ancestor_class(the_ancestor_class);

}/*end is_ancestor_class*/

/*
 *The trivial test of checking with the child class is performed here. To
 *further check the transitive closure the call is forwarded with the
 *child_class_type variable
 */
bool InheritanceLink::is_descendant_class(ClassType* the_descendant_class) 
                                                const{

  if(the_descendant_class == _child_class_type)
    return true;

  return _child_class_type->is_descendant_class(the_descendant_class);
}/*end is_descendant_class*/


void ClassType::add_static_field_symbol( StaticFieldSymbol* sym )
{
  _per_class_symbol_table->add_symbol( sym );
}

bool ClassType::has_static_field_symbol(const LString& name) {
  return _per_class_symbol_table->has_lookup_table_member(name);
}


StaticFieldSymbol*
ClassType::lookup_static_field_symbol( const LString& name ) {
  return
    to<StaticFieldSymbol>( _per_class_symbol_table->lookup_lookup_table(name) );
}

void ClassType::add_static_method_symbol( StaticMethodSymbol* sym )
{
  _per_class_symbol_table->add_symbol( sym );
}

bool ClassType::has_static_method_symbol (const LString& name) {
  return _per_class_symbol_table->has_lookup_table_member(name);
}


StaticMethodSymbol*
ClassType::lookup_static_method_symbol( const LString& name ) {
  return
    to<StaticMethodSymbol>( _per_class_symbol_table->lookup_lookup_table(name) );
}

void ClassType::add_instance_field_symbol( InstanceFieldSymbol* fsym )
{
  get_group_symbol_table()->add_symbol( fsym );
}

bool ClassType::has_instance_field_symbol( const LString& name ) {
  return get_group_symbol_table()->has_lookup_table_member(name);
}


InstanceFieldSymbol*
ClassType::lookup_instance_field_symbol( const LString& name ) {
  return
    to<InstanceFieldSymbol>( get_group_symbol_table()->lookup_lookup_table(name) );
}


void ClassType::add_instance_method_symbol( InstanceMethodSymbol* msym )
{
  _instance_method_symbol_table->add_symbol( msym );
}

bool ClassType::has_instance_method_symbol(const LString& name) {
  return _instance_method_symbol_table->has_lookup_table_member(name);
}


InstanceMethodSymbol*
ClassType::lookup_instance_method_symbol(const LString& name ) {
  return to<InstanceMethodSymbol>( _instance_method_symbol_table->lookup_lookup_table(name) );
}






