// $Id: type_builder.cpp,v 1.2 2000/07/18 19:16:33 brm Exp $

#include "suifkernel/suif_object.h"
#include "suifkernel/iter.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module.h"
#include "suifkernel/module_subsystem.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "typebuilder/type_builder.h"

#include "type_builder.h"


const LString osuif_type_builder_class_name("OsuifTypeBuilder");


class OsuifTypeBuilderModule : public Module {
public:
  OsuifTypeBuilderModule(SuifEnv* env) :
    Module(env, osuif_type_builder_class_name) { }
  virtual void initialize() {
    _suif_env->add_object_factory( new OsuifTypeBuilder() );
  }

  virtual Module* clone() const { return (Module *)this; }
};


extern "C" void EXPORT init_osuiftypebuilder( SuifEnv* suif )
{
  ModuleSubSystem *mSubSystem = suif->get_module_subsystem();
  if (!mSubSystem->retrieve_module(osuif_type_builder_class_name)) {
    mSubSystem -> register_module(new OsuifTypeBuilderModule( suif ));
  }
}


void OsuifTypeBuilder::init( SuifEnv *env ) {
  _suif_env = env;
  _tb = get_type_builder( env );
}


const LString& OsuifTypeBuilder::get_class_name() {
  return osuif_type_builder_class_name;
}

const LString& OsuifTypeBuilder::getName() {
  return osuif_type_builder_class_name;
}


InstanceMethodType*
OsuifTypeBuilder::get_instance_method_type( DataType * result_type,
					    list<QualifiedType *> argument_list,
					    bool has_varargs = false,
					    bool arguments_known = true,
					    int bit_alignment = 0)
{
  if (bit_alignment == 0)
    bit_alignment = get_procedure_bit_alignment();

  FileSetBlock* block = _suif_env->get_file_set_block();
  SymbolTable* table = block->get_external_symbol_table();
  SymbolTable* result_type_symtab =
    to<SymbolTable>( result_type->get_parent() );
  table = TypeBuilder::most_nested_common_scope( result_type_symtab,
						 table );
  table = TypeBuilder::most_nested_common_scope( argument_list, table );

  if (!table) {
    table = _suif_env->get_file_set_block()->get_external_symbol_table();
  }

  // Search the table
  Iter<SymbolTableObject *> iter = table->get_symbol_table_object_iterator();
  InstanceMethodType* type = NULL;

  while( iter.is_valid() ) {
    SymbolTableObject *obj = iter.current();
    if (is_a<InstanceMethodType>(obj)) {
      type = to<InstanceMethodType>(obj);
      if (((unsigned)type->get_argument_count() == 
	   argument_list.length())
	  &&  (type->get_has_varargs() == has_varargs)
	  &&  (type->get_arguments_known() == arguments_known)
	  &&  (type->get_bit_alignment() == bit_alignment)
	  &&  (type->get_result_type() == result_type)
	  &&  TypeBuilder::is_argument_types_match(argument_list,type))
	return type;
    }
    iter.next();
  }

  type = ::create_instance_method_type( _suif_env,
					result_type,
					has_varargs,
					arguments_known,
					bit_alignment );
  table->append_symbol_table_object( type );
  append_arguments( type, argument_list );

  return type;
}


StaticMethodType*
OsuifTypeBuilder::get_static_method_type( DataType* result_type,
					  list<QualifiedType *> argument_list,
					  bool has_varargs = false,
					  bool arguments_known = true,
					  int bit_alignment = 0 )
{
  if (bit_alignment == 0)
    bit_alignment = get_procedure_bit_alignment();

  FileSetBlock *block = _suif_env->get_file_set_block();
  SymbolTable *table = block->get_external_symbol_table();
  table = TypeBuilder::most_nested_common_scope( to<SymbolTable>(result_type->get_parent()),
						 table );
  table = TypeBuilder::most_nested_common_scope(argument_list,table);

  if (!table) {
    table = _suif_env->get_file_set_block()->get_external_symbol_table();
  }

  // Search the table
  Iter<SymbolTableObject *> iter = table->get_symbol_table_object_iterator();
  StaticMethodType* type = NULL;

  while( iter.is_valid() ) {
    SymbolTableObject *obj = iter.current();
    if (is_a<StaticMethodType>(obj)) {
      type = to<StaticMethodType>(obj);
      if (((unsigned)type->get_argument_count() == 
	   argument_list.length())
	  &&  (type->get_has_varargs() == has_varargs)
	  &&  (type->get_arguments_known() == arguments_known)
	  &&  (type->get_bit_alignment() == bit_alignment)
	  &&  (type->get_result_type() == result_type)
	  &&  TypeBuilder::is_argument_types_match(argument_list,type))
	return type;
    }
    iter.next();
  }

  type = ::create_static_method_type( _suif_env,
				      result_type,
				      has_varargs,
				      arguments_known,
				      bit_alignment );
  table->append_symbol_table_object( type );
  append_arguments( type, argument_list );

  return type;
}


InstanceFieldSymbol*
OsuifTypeBuilder::add_instance_field_symbol_to_class_type( ClassType* ctype,
							   const LString& symbol_name,
							   QualifiedType* symbol_type )
{
  // @@@
  IntConstant* icnst =
    ::create_int_constant( _suif_env,
			   _tb->get_integer_type(false),
			   IInteger() );

  InstanceFieldSymbol* fsym = 
    ::create_instance_field_symbol( _suif_env,
				    symbol_type,
				    icnst,
				    symbol_name,
				    false );
  ctype->add_instance_field_symbol( fsym );
  return fsym;
}


int OsuifTypeBuilder::get_procedure_bit_alignment() {
  TargetInformationBlock* ti_block =
    find_target_information_block( _suif_env );
  int bit_alignment = ti_block->get_procedure_alignment();

  if (bit_alignment == 0) {
    // Only works for THIS machine.
    bit_alignment = sizeof(int)*8;
  }

  return bit_alignment;
}


void OsuifTypeBuilder::append_arguments( CProcedureType* ptype,
					 list<QualifiedType *>& argument_list )
{
  list<QualifiedType *>::iterator it = argument_list.begin();
  while (it != argument_list.end()) {
    ptype->append_argument(*it);
    it ++;
  }
}


list<QualifiedType *>&
OsuifTypeBuilder::build_arg_list( DataType* arg0 ) {
  list<QualifiedType *>* arg_list =
    new list<QualifiedType *>();
  arg_list->push_back( _tb->get_qualified_type(arg0) );
  return *arg_list;
}


list<QualifiedType *>&
OsuifTypeBuilder::build_arg_list( DataType* arg0,
				  DataType* arg1 )
{
  list<QualifiedType *>* arg_list =
    new list<QualifiedType *>();
  arg_list->push_back( _tb->get_qualified_type(arg0) );
  arg_list->push_back( _tb->get_qualified_type(arg1) );
  return *arg_list;
}


list<QualifiedType *>&
OsuifTypeBuilder::build_arg_list( DataType* arg0,
				  DataType* arg1,
				  DataType* arg2 )
{
  list<QualifiedType *>* arg_list =
    new list<QualifiedType *>();
  arg_list->push_back( _tb->get_qualified_type(arg0) );
  arg_list->push_back( _tb->get_qualified_type(arg1) );
  arg_list->push_back( _tb->get_qualified_type(arg2) );
  return *arg_list;
}


InstanceMethodType*
OsuifTypeBuilder::get_instance_method_type( bool has_varargs,
					    bool arguments_known )
{
  list<QualifiedType *> arg_list;
  return get_instance_method_type( _tb->get_void_type(),
				   arg_list,
				   has_varargs,
				   arguments_known,
				   get_procedure_bit_alignment() );
}


InstanceMethodType*
OsuifTypeBuilder::get_instance_method_type( bool has_varargs,
					    DataType* arg0 )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(arg0) );
  return get_instance_method_type( _tb->get_void_type(),
				   arg_list,
				   has_varargs,
				   true,
				   get_procedure_bit_alignment() );
}


InstanceMethodType*
OsuifTypeBuilder::get_instance_method_type( DataType* result_type,
					    bool has_varargs,
					    bool arguments_known )
{
  list<QualifiedType *> arg_list;
  return get_instance_method_type( result_type,
				   arg_list,
				   has_varargs,
				   arguments_known,
				   get_procedure_bit_alignment() );
}


InstanceMethodType*
OsuifTypeBuilder::get_instance_method_type( DataType* result_type,
					    bool has_varargs,
					    DataType* arg0 )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(arg0) );
  return get_instance_method_type( result_type,
				   arg_list,
				   has_varargs,
				   true,
				   get_procedure_bit_alignment() );
}




CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( bool has_varargs,
					bool arguments_known)
{
  list<QualifiedType *> arg_list;
  return _tb->get_c_procedure_type( _tb->get_void_type(),
				    arg_list,
				    has_varargs,
				    arguments_known,
				    get_procedure_bit_alignment() );
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( bool has_varargs,
					DataType *argument0_type)
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  return _tb->get_c_procedure_type( _tb->get_void_type(),
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() );
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( bool has_varargs,
					DataType* argument0_type,
					DataType* argument1_type )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  arg_list.push_back( _tb->get_qualified_type(argument1_type) );
  return _tb->get_c_procedure_type( _tb->get_void_type(),
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() );
}

 
CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( bool has_varargs,
					DataType* argument0_type,
					DataType* argument1_type,
					DataType* argument2_type )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  arg_list.push_back( _tb->get_qualified_type(argument1_type) );
  arg_list.push_back( _tb->get_qualified_type(argument2_type) );
  return _tb->get_c_procedure_type( _tb->get_void_type(),
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() );
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( DataType* result_type,
					bool has_varargs,
					bool arguments_known )
{ 
  list<QualifiedType *> arg_list;
  return _tb->get_c_procedure_type( result_type,
				    arg_list,
				    has_varargs,
				    arguments_known,
				    get_procedure_bit_alignment() ); 
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( DataType* result_type,
					bool has_varargs,
					DataType* argument0_type)
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  return _tb->get_c_procedure_type( result_type,
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() ); 
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( DataType* result_type,
					bool has_varargs,
					DataType* argument0_type,
					DataType* argument1_type )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  arg_list.push_back( _tb->get_qualified_type(argument1_type) );
  return _tb->get_c_procedure_type( result_type,
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() ); 
}


CProcedureType*
OsuifTypeBuilder::get_c_procedure_type( DataType* result_type,
					bool has_varargs,
					DataType* argument0_type,
					DataType* argument1_type,
					DataType* argument2_type )
{
  list<QualifiedType *> arg_list;
  arg_list.push_back( _tb->get_qualified_type(argument0_type) );
  arg_list.push_back( _tb->get_qualified_type(argument1_type) );
  arg_list.push_back( _tb->get_qualified_type(argument2_type) );
  return _tb->get_c_procedure_type( result_type,
				    arg_list,
				    has_varargs,
				    true,
				    get_procedure_bit_alignment() ); 
}



OsuifTypeBuilder* get_osuif_type_builder(SuifEnv* env) {
  return ((OsuifTypeBuilder *)
	  (env->get_object_factory(OsuifTypeBuilder::get_class_name())));
}
