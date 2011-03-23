/* file "build_single_vtable.cpp" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include <suif_copyright.h>


/*
      This is the implementation of dismantling passes of the porky
      library.
*/


#define _MODULE_ "libporky"

#pragma implementation "build_single_vtable.h"

#include <cast.h>
#include <utilities.h>
#include <error_macros.h>
#include <i_integer.h>
#include <basic_factory.h>
#include <suif.h>
#include <suif_factory.h>
#include <basic_constants.h>
#include <suifkernel_messages.h>

#include "build_single_vtable.h"
#ifdef HACK
#include <symbol_utils.h>
#include <execution_utils.h>
#endif
#include <osuif.h>
#include <osuif_factory.h>

#define BITS_PER_BYTE	8



build_single_vtable_pass::
build_single_vtable_pass(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {};


// Construct a list of the objects to be processed, before making any changes.
// At this point we're looking for the roots of inheritance trees: i.e. those
// classes that do not inherit from any other class.
list<ClassType*>* collect_root_class_symbol_objects( SuifObject* start_object ) {
  list<ClassType*>* l = new list<ClassType*>;

  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( ClassType::get_class_name() );

    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      if ( ((ClassType*)it->current())->get_parent_classe_count() == 0 ) { // root?
        l->push_back( (ClassType*)it->current() );             // yes
      }
      it->next();
    }
    delete it;
  }
  return l;
}



// Construct a list of the objects to be processed, before making any changes.
list<MethodSymbol*>* collect_dispatched_method_symbol_objects( SuifObject* start_object ) {
  list<MethodSymbol*>* l = new list<MethodSymbol*>;

  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( MethodSymbol::get_class_name() );

    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      if ( ((MethodSymbol*)it->current())->get_is_dispatched() ) {
        l->push_back( (MethodSymbol*)it->current() );
      }
      it->next();
    }
    delete it;
  }
  return l;
}


PointerType *build_single_vtable_pass::pointer_to(Type *type) {
  SuifObjectFactory *suif_of = (SuifObjectFactory *)
    get_suif_env()->get_object_factory(SuifObjectFactory::get_class_name());
  return suif_of->create_pointer_type(sizeof(void*), sizeof(void*), type);
}

// A pointer to where we create the type for vtable entries
CProcedureType *build_single_vtable_pass::vtable_entry_type;



// Find ClassType entries.
void build_single_vtable_pass::do_file_block( FileBlock* file_block )
{
  FileSetBlock *file_set_block = get_suif_env()->get_file_set_block();
  list<MethodSymbol*> empty_list;

  // First create the vtable entry type
  BasicObjectFactory *basic_of = (BasicObjectFactory *)
    get_suif_env()->get_object_factory(BasicObjectFactory::get_class_name());
  SuifObjectFactory *suif_of = (SuifObjectFactory *)
    get_suif_env()->get_object_factory(SuifObjectFactory::get_class_name());
  VoidType *void_type = suif_of->create_void_type(sizeof(int), sizeof(int));
  //create vtable type: no varargs, arguments are known, int-size alignment.
  vtable_entry_type = suif_of->create_c_procedure_type(false, true, sizeof(int));
  //no arguments, but void result.
  vtable_entry_type->append_result(void_type);

  // Put it in the file-set symbol table
  file_set_block->get_file_set_symbol_table()->add_symbol("__vtable_entry_type", vtable_entry_type);

  // Collect the root classes
  list<ClassType *> *l1 = collect_root_class_symbol_objects(
                        file_block->get_symbol_table());

  for (list<ClassType *>::iterator iter1(l1->begin());
       iter1 != l1->end(); iter1++) {
    ClassType *the_class = *iter1;
    do_class_type(file_set_block, the_class, &empty_list);
  }
}  


// Process instance methods for a class and its children.
void build_single_vtable_pass::
do_class_type(FileSetBlock *f, ClassType *the_class, list<MethodSymbol*> *super_list) {
  suif_assert(the_class != NULL);
  list<MethodSymbol*> *my_list = new list<MethodSymbol*>();
  list<MethodSymbol*>::iterator copier, comp;

  // get a list of methods defined in this class
  BasicSymbolTable *the_table = the_class->get_instance_method_symbol_table();
  list<MethodSymbol*> *my_methods = collect_dispatched_method_symbol_objects(the_table);

  // my list is at least my ancestor's list - but my own methods override it
  for(copier=super_list->begin(); copier != super_list->end(); copier++) {
    for(comp=my_methods->begin(); comp != my_methods->end(); comp++) {
      if ((*comp)->get_name() == (*copier)->get_name()) {
        my_list->push_back(*comp);
        my_methods->erase(comp);
        goto nextsym;
      }
    }
    my_list->push_back(*copier);
nextsym: ;
  }

  // Anything left over goes at the end of the table
  for(copier=my_methods->begin(); copier != my_methods->end(); copier++) {
    my_list->push_back(*copier);
  }
  // my_list is now a list of the things we need in the vtable.  This will
  // be processed somehow.

  // Begin by building the vtable type and putting it in the per class
  // symbol table
  BasicObjectFactory *basic_of = (BasicObjectFactory *)
    get_suif_env()->get_object_factory(BasicObjectFactory::get_class_name());
  SuifObjectFactory *suif_of = (SuifObjectFactory *)
    get_suif_env()->get_object_factory(SuifObjectFactory::get_class_name());
  OsuifObjectFactory *osuif_of = (OsuifObjectFactory *)
    get_suif_env()->get_object_factory(OsuifObjectFactory::get_class_name());
  int vtable_length = my_list->size();
  IIntOrSourceOp i0(basic_of->create_int_constant(0));
  IIntOrSourceOp in(basic_of->create_int_constant(vtable_length));
  ArrayType *vtable_type = 
    suif_of->create_array_type(IInteger(vtable_length)*sizeof(void (*)()),
                (int)sizeof (void (*)()) * BITS_PER_BYTE,
                vtable_entry_type, i0, in);
  the_class->get_per_class_symbol_table()->add_symbol("_._vtable_type",vtable_type);

  // Now build the definition
  MultiValueBlock *vtable_creation = suif_of->create_multi_value_block(vtable_type);
  for (int entry=0; entry < vtable_length; entry++ ) {
    vtable_creation->add_sub_block(entry,
         suif_of->create_expression_value_block(
              suif_of->create_load_address_expression(pointer_to(vtable_entry_type),
                   *(my_list->get_nth(entry)))));
  }
  IIntOrSourceOp offset;		// to satisfy the object factory call
  ClassFieldSymbol *vtable_symbol =
    osuif_of->create_class_field_symbol(true, vtable_type, offset, false);
  VariableDefinition *vtable_definition =
    basic_of->create_variable_definition(vtable_symbol,
                (int)sizeof(void (*)()) * BITS_PER_BYTE,
                vtable_creation);
  vtable_symbol->set_definition(vtable_definition);
  the_class->get_per_class_symbol_table()->add_symbol("_._vtable_symbol",vtable_symbol);
  the_class->get_definition_block()->append_variable_definition(vtable_definition);

  // Now we call all child classes with this vtable, so they can do the same thing.
  for (Iter<InheritanceLink*> children = the_class->get_child_classe_iterator();
                             children.is_valid(); children.next()) {
    ClassType *child = ((InheritanceLink*)(children.current()))->get_child_class_type();
    do_class_type(f, child, my_list);
  }
}
