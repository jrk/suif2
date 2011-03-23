/* file "osuif_demo_pass.cpp" */


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

#pragma implementation "osuif_demo_pass.h"

#include <cast.h>
#include <utilities.h>
#include <error_macros.h>
#include <i_integer.h>
#include <basic_factory.h>
#include <suif.h>
#include <suif_factory.h>
#include <basic_constants.h>
#include <suifkernel_messages.h>

#include "osuif_demo_pass.h"
#include <symbol_utils.h>
#include <execution_utils.h>
#include <osuif.h>

globalize_class_method_symbols_pass::
globalize_class_method_symbols_pass(SuifEnv *env, const LString &name)
  : PipelinablePass(env, name) {};


list<ClassType*>* collect_class_symbol_objects( SuifObject* start_object ) {
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


list<FileBlock*>* collect_fileblock_objects( SuifObject* start_object ) {
  list<FileBlock*>* l = new list<FileBlock*>;

  if ( start_object ) { 
    MetaClass* what = start_object->get_object_factory()->
      find_meta_class( FileBlock::get_class_name() );


    Iterator* it = object_iterator(start_object,
				   start_object->get_meta_class(),
				   what);
    while ( it->is_valid() ) {
      suif_assert( it->current_meta_class() == what );
      l->push_back( (FileBlock*)it->current() );
      it->next();
    }
    delete it;
  }
  return l;
}


void globalize_class_method_symbols_pass::do_file_set_block(
                           FileSetBlock* file_set_block )
{
  SuifObject *p = file_set_block;
  printf("\nGiven a %s\n",p->getClassName().c_str());
  int cnt=0;
  list<ClassType *> *l = collect_class_symbol_objects(p);
  for (list<ClassType *>::iterator iter(l->begin());
       iter != l->end(); iter++) {
    printf("Have one ClassType\n"); cnt++;
  }
  printf("Found %d ClassType objects\n",cnt);
}



void globalize_class_method_symbols_pass::do_file_block(
                           FileBlock*    file_block )
{
  SuifObject *p = file_block;
  printf("\nGiven a %s\n",p->getClassName().c_str());
  int cnt=0;
  list<ClassType *> *l = collect_class_symbol_objects(p);
  for (list<ClassType *>::iterator iter(l->begin());
       iter != l->end(); iter++) {
    printf("Have one ClassType\n"); cnt++;
  }
  printf("Found %d ClassType objects\n",cnt);
}



void globalize_class_method_symbols_pass::do_variable_definition(
			   VariableDefinition *vbl_def)
{
  SuifObject *p = vbl_def;
  printf("\nGiven a %s\n",p->getClassName().c_str());
  int cnt=0;
  list<ClassType *> *l = collect_class_symbol_objects(p);
  for (list<ClassType *>::iterator iter(l->begin());
       iter != l->end(); iter++) {
    printf("Have one ClassType\n"); cnt++;
  }
  printf("Found %d ClassType objects\n",cnt);
}




void globalize_class_method_symbols_pass::do_procedure_definition(
			   ProcedureDefinition *proc_def)
{
  int cnt=0;
  SuifObject *p = proc_def;
  printf("\nGiven a %s\n",p->getClassName().c_str());
  while (p = p->get_parent()) {
    printf(" Owned by a %s\n",p->getClassName().c_str());
  }
  list<ClassType *> *l = collect_class_symbol_objects(proc_def);
  for (list<ClassType *>::iterator iter(l->begin());
       iter != l->end(); iter++) {
printf("Have one ClassType\n"); cnt++;
/*
    IfStatement *the_if = *iter;
    StatementList *replacement = osuif_demo_pass_statement(the_if);
    if (replacement == NULL) continue;
    // ignore the return value here.
    replace_statement_with_list(get_suif_env(), the_if, replacement);
*/
  }
printf("Found %d ClassType objects\n",cnt);
}  

