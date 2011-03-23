// $Id: static_method_call_utils.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#include "typebuilder/type_builder.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "osuifutilities/symbol_utils.h"

#include "osuiflowering/static_method_call_utils.h"


void lower_static_method_call_statement( StaticMethodCallStatement* smcs ) {
  StaticMethodSymbol* msym = smcs->get_target_method();
  StaticMethodType* mtype =
    to<StaticMethodType>( msym->get_type() );
  
  TypeBuilder* tb = get_type_builder( smcs->get_suif_env() );
  PointerType* ptype = tb->get_pointer_type( mtype );

  SymbolAddressExpression* sa_expr =
    ::create_symbol_address_expression( smcs->get_suif_env(),
					ptype,
					msym );    
  smcs->set_callee_address( sa_expr );
}


void lower_static_method_call_expression( StaticMethodCallExpression* smc ) {
  StaticMethodSymbol* msym = smc->get_target_method();
  StaticMethodType* mtype =
    to<StaticMethodType>( msym->get_type() );
  
  TypeBuilder* tb = get_type_builder( smc->get_suif_env() );
  PointerType* ptype = tb->get_pointer_type( mtype );

  SymbolAddressExpression* sa_expr =
    ::create_symbol_address_expression( smc->get_suif_env(),
					ptype,
					msym );    
  smc->set_callee_address( sa_expr );
}
