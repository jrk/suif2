// $Id: expression_utils.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "typebuilder/type_builder.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "osuifutilities/symbol_utils.h"

#include "osuifutilities/expression_utils.h"


InstanceMethodCallExpression*
build_InstanceMethodCallExpression( InstanceMethodSymbol* msym,
				    bool is_dispatched,
				    bool lowering )
{
  SuifEnv* env = msym->get_suif_env();
  InstanceMethodType* mtype =
    to<InstanceMethodType>( msym->get_type() );
  DataType* result_type = mtype->get_result_type();

  suif_assert( msym->get_is_dispatched() || !is_dispatched );

  Expression* callee_address = NULL;
  if( lowering ) {
    suif_assert( !msym->get_is_dispatched() );
    suif_assert( !is_dispatched );

    TypeBuilder* tb = get_type_builder( env );
    PointerType* ptype = tb->get_pointer_type( msym->get_type() );

    callee_address =
      ::create_symbol_address_expression( env, ptype, msym );
    symbol_has_address_taken( msym );
  }
  
  InstanceMethodCallExpression* expr =
    ::create_instance_method_call_expression( env,
					      result_type,
					      callee_address,
					      msym,
					      is_dispatched );
  return expr;
}


StaticMethodCallExpression*
build_StaticMethodCallExpression( StaticMethodSymbol* msym,
				  bool lowering )
{
  SuifEnv* env = msym->get_suif_env();
  StaticMethodType* mtype =
    to<StaticMethodType>( msym->get_type() );
  DataType* result_type = mtype->get_result_type();

  Expression* callee_address = NULL;
  if( lowering ) {
    TypeBuilder* tb = get_type_builder( env );
    PointerType* ptype = tb->get_pointer_type( msym->get_type() );

    callee_address =
      ::create_symbol_address_expression( env, ptype, msym );
    symbol_has_address_taken( msym );
  }
  
  StaticMethodCallExpression* expr =
    ::create_static_method_call_expression( env,
					    result_type,
					    callee_address,
					    msym );
  return expr;
}



Expression* build_LoadExpression( Expression* expr ) {
  DataType* dtype = expr->get_result_type();
  PointerType* ptype = to<PointerType>( dtype );
  QualifiedType* q_dereftype =
    to<QualifiedType>( ptype->get_reference_type() );
  DataType* dereftype = q_dereftype->get_base_type();

  return ::create_load_expression( expr->get_suif_env(),
				   dereftype,
				   expr );
}


Expression* build_convert_UnaryExpression( DataType* new_type,
					   Expression* expr )
{
  return ::create_unary_expression( expr->get_suif_env(),
				    new_type,
				    k_convert,
				    expr );
}

