/* file "globalize_class_variables.cpp" */


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

#pragma implementation "globalize_class_variables.h"

#include <cast.h>
#include <utilities.h>
#include <error_macros.h>
#include <i_integer.h>
#include <basic_factory.h>
#include <suif.h>
#include <suif_factory.h>
#include <basic_constants.h>
#include <suifkernel_messages.h>

#include "globalize_class_variables.h"
#ifdef HACK
#include <symbol_utils.h>
#include <execution_utils.h>
#endif
#include <osuif.h>
#include <osuif_factory.h>

globalize_class_variable_symbols_pass::
globalize_class_variable_symbols_pass(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {};


// Construct a list of the objects to be processed, before making any changes.
list<ClassType*>* collect_class_type_objects( SuifObject* start_object ) {
  list<ClassType*>* l = new list<ClassType*>;

  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( ClassType::get_class_name() );


    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      l->push_back( (ClassType*)it->current() );
      it->next();
    }
    delete it;
  }
  return l;
}



// Construct a list of the objects to be processed, before making any changes.
list<VariableSymbol*>* collect_variable_symbol_objects( SuifObject* start_object ) {
  list<VariableSymbol*>* l = new list<VariableSymbol*>;

  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( VariableSymbol::get_class_name() );

    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      l->push_back( (VariableSymbol*)it->current() );
      it->next();
    }
    delete it;
  }
  return l;
}



// Find ClassType entries.
void globalize_class_variable_symbols_pass::do_file_block( FileBlock* file_block )
{
  FileSetBlock *file_set_block = get_suif_env()->get_file_set_block();

  list<ClassType *> *l1 = collect_class_type_objects(
                        file_block->get_symbol_table());
  for (list<ClassType *>::iterator iter(l1->begin());
       iter != l1->end(); iter++) {
    ClassType *the_class = *iter;
    do_class_type(file_set_block, the_class);
  }
}  


// Each ClassType may have VariableSymbol objects 
//  its per_class_symbol_table.  It can also have them in
//  its underlying group_symbol_table, but we ignore those because
//  they are already sufficiently lowered.
void globalize_class_variable_symbols_pass::
do_class_type(FileSetBlock *f, ClassType *the_class) {
  suif_assert(the_class != NULL);

  SuifEnv *s = get_suif_env();
  LString ClassName = the_class->get_name(); 
  BasicSymbolTable *st = f->get_external_symbol_table();
  BasicSymbolTable *the_table;

  the_table = the_class->get_per_class_symbol_table();
  move_variable_symbols(s, ClassName, the_table, st);
}

LString globalize_class_variable_symbols_pass::
mangle(BasicSymbolTable* st, const char* classname, const char* methodname) {
  char buf[512];
  LString key;

  assert(512 >= snprintf(buf,512, "__%s_%s",classname,methodname));
  new (&key) LString(buf);
  if (!st->has_lookup_table_member(key)) { return key; }
  int i;
  for(i=1;i<1000000;i++) {
    assert(512 >= snprintf(buf,512, "__%s_%s_%d",classname,methodname,i));
    new (&key) LString(buf);
    if (!st->has_lookup_table_member(key)) { return key; }
  }
  assert(i<1000000);
  return key;
}

void globalize_class_variable_symbols_pass::
move_variable_symbols(SuifEnv *s, const LString &mname, BasicSymbolTable *the_table, BasicSymbolTable *st) {
  static const LString name_key("NAME");
  static const LString history_key("VARNAME");
// HACK: when SymbolTableAnnotes work
#if 0
  static const LString table_key("SYMTAB");
#endif

  LString VariableName;

  list<VariableSymbol *> *l1 = collect_variable_symbol_objects(the_table);
  for (list<VariableSymbol *>::iterator iter(l1->begin());
       iter != l1->end(); iter++) {
    VariableSymbol *the_variable = *iter;
    VariableName = the_variable->get_name();

    // remove the symbol from its current table
    the_table->remove_symbol(the_variable);

    // add to uppermost table with mangled name
    LString NewName = mangle(st, mname.c_str(), VariableName.c_str());
    st -> add_symbol(NewName, the_variable);
  }
}
