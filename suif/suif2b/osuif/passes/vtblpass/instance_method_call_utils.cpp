// $Id: instance_method_call_utils.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "typebuilder/type_builder.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "utils/expression_utils.h"
#include "utils/cloning_utils.h"
#include "osuifutilities/symbol_utils.h"
#include "osuifutilities/problems.h"
#include "vtblnodes/vtbl_utils.h"

#include "vtblpass/instance_method_call_utils.h"


void LowerInstanceMethodCalls::
lower_method_call_stmt( InstanceMethodCallStatement* stmt ) {
  InstanceMethodSymbol* msym = stmt->get_target_method();

  if( msym->get_is_dispatched() ) {
    if( stmt->get_is_dispatched() ) {
      lower_dispatched_method_call_stmt( stmt );
    } else {
      lower_undispatched_method_call_stmt( stmt );
    }
  } else {
    suif_assert( !stmt->get_is_dispatched() );
    lower_undispatched_method_call_stmt( stmt );
  }
}

void LowerInstanceMethodCalls::
lower_dispatched_method_call_stmt( InstanceMethodCallStatement* stmt ) {
    //  InstanceMethodType* mtype =
    //    to<InstanceMethodType>( msym->get_type() );

  OsuifProblems::not_implemented();
}

void LowerInstanceMethodCalls::
lower_undispatched_method_call_stmt( InstanceMethodCallStatement* stmt ) {
  OsuifProblems::not_implemented();
}


void LowerInstanceMethodCalls::
lower_method_call_expr( InstanceMethodCallExpression* expr ) {
  InstanceMethodSymbol* msym = expr->get_target_method();

  if( msym->get_is_dispatched() ) {
    if( expr->get_is_dispatched() ) {
      lower_dispatched_method_call_expr( expr );
    } else {
      lower_undispatched_method_call_expr( expr );
    }
  } else {
    suif_assert( !expr->get_is_dispatched() );
    lower_undispatched_method_call_expr( expr );
  }
}


/*
 * An undispatched method call is handled like a static call:
 * The method is directly called.
 */
void LowerInstanceMethodCalls::
lower_undispatched_method_call_expr( InstanceMethodCallExpression* expr ) {
  TypeBuilder* tb = get_type_builder( expr->get_suif_env() );

  InstanceMethodSymbol* msym = expr->get_target_method();
  InstanceMethodType* mtype =
    to<InstanceMethodType>( msym->get_type() );
  
  SymbolAddressExpression* sa_expr =
    ::create_symbol_address_expression( expr->get_suif_env(),
					tb->get_pointer_type(mtype),
					msym );    
  expr->set_callee_address( sa_expr );
}


/*
 * The first argument of the method call is assumed to be the
 * this-reference.
 */
Expression* LowerInstanceMethodCalls::
get_this_reference( InstanceMethodCallExpression* expr ) {
  return deep_suif_clone<Expression>( expr->get_argument(0) );
}


/*
 * The first instance field symbol of the ClassType is assumed
 * to contain the pointer to the vtbl.
 */
InstanceFieldSymbol* LowerInstanceMethodCalls::
get_vtbl_field_symbol( SingleInheritanceClassType* ctype ) {
  suif_assert( ctype->get_group_symbol_table()
	       ->get_symbol_table_object_count() > 0 );
  
  return to<InstanceFieldSymbol>( ctype->get_group_symbol_table()
				  ->get_symbol_table_object(0) );
}


/*
 * Generates dispatch code for vbtls:
 * ( ( <method-type-cast> ) <this-ref-expr>
 *   -> vtbl-slot-field-symbol [ vtbl-slot ] )   ( ... )
 */
void LowerInstanceMethodCalls::
lower_dispatched_method_call_expr( InstanceMethodCallExpression* expr ) {
  TypeBuilder* tb = get_type_builder( expr->get_suif_env() );

  InstanceMethodSymbol* msym = expr->get_target_method();
  InstanceMethodType* mtype =
    to<InstanceMethodType>( msym->get_type() );

  SingleInheritanceClassType* ctype =
    to<SingleInheritanceClassType>( ::get_owning_class(msym) );

  suif_assert_message( ctype,
		       ("Cannot determine owning ClassType of"
			" InstanceMethodSymbol.") );

  /*
    VariableSymbol* vtbl_vsym = ctype->get_vtbl_symbol();
    suif_assert( vtbl_vsym );

    QualifiedType* vtbl_vsym_q_type = vtbl_vsym->get_type();
    ArrayType* vtbl_vsym_type =
      to<ArrayType>( vtbl_vsym_q_type->get_base_type() );
    QualifiedType* vtbl_vsym_q_slot_type = vtbl_vsym_type->get_element_type();
    DataType* vtbl_vsym_slot_type = vtbl_vsym_q_slot_type->get_base_type();
    Expression* base_address_expr =
      ::create_symbol_address_expression( expr->get_suif_env(),
					  tb->get_pointer_type(vtbl_vsym_type),
					  vtbl_vsym );
  */

  Expression* this_ref_expr = get_this_reference( expr );
  InstanceFieldSymbol* vtbl_fsym = get_vtbl_field_symbol( ctype );
  
  Expression* fa_expr =
    ::create_field_access_expression( expr->get_suif_env(),
				      vtbl_fsym->get_type()->get_base_type(),
				      this_ref_expr,
				      vtbl_fsym );
    
    
  VtblSlotNumberAnnote* slot_number_annote =
    to<VtblSlotNumberAnnote>( msym->peek_annote(k_vtbl_slot_number_annote) );
  suif_assert( slot_number_annote );

  IInteger slot = slot_number_annote->get_slot_number();
  IntConstant* slot_expr = ::create_int_constant( expr->get_suif_env(),
						  slot );

  SingleInheritanceVtbls* vtbls =
    new SingleInheritanceVtbls( expr->get_suif_env() );

  Expression* aref_expr =
    ::create_array_reference_expression( expr->get_suif_env(),
					 vtbls->vtbl_slot_type( ctype ),
					 fa_expr,
					 slot_expr );
  Expression* convert_expr =
    ::create_unary_expression( expr->get_suif_env(),
			       tb->get_pointer_type(mtype),
			       k_convert,
			       aref_expr );

  expr->set_callee_address( convert_expr );
}

