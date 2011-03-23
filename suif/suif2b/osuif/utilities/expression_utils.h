// $Id: expression_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFUTILITIES__EXPRESSION_UTILS_H
#define OSUIFUTILITIES__EXPRESSION_UTILS_H

#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "cfenodes/cfe_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifextensionnodes/osuifextension_forwarders.h"


/*
 * A InstanceMethodCallExpression is constructed.
 * If 'lowering' is true and the is_dispatched flag of 'msym' is false,
 * the callee_address is set to the address
 * of 'msym'; otherwise callee_address is set to NULL.
 */
extern InstanceMethodCallExpression*
build_InstanceMethodCallExpression( InstanceMethodSymbol* msym,
				    bool is_dispatched,
				    bool lowering = false );


/*
 * A StaticMethodCallExpression is constructed.
 * If 'lowering' is true the callee_address is set to the address
 * of 'msym'; otherwise callee_address is set to NULL.
 */
extern StaticMethodCallExpression*
build_StaticMethodCallExpression( StaticMethodSymbol* msym,
				  bool lowering = false );


extern Expression* build_LoadExpression( Expression* expr );
extern Expression* build_convert_UnaryExpression( DataType* new_type,
						  Expression* expr );

#endif
