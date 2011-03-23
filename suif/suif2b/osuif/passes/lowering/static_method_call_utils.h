// $Id: static_method_call_utils.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef OSUIFLOWERING__METHOD_CALL_UTILS_H
#define OSUIFLOWERING__METHOD_CALL_UTILS_H

#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "cfenodes/cfe_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifextensionnodes/osuifextension_forwarders.h"


/*
 * StaticMethodCallStatement lowering.
 * The callee_address is constructed based on the
 * target_method.
 */
extern void
lower_static_method_call_statement( StaticMethodCallStatement* smcs );


/*
 * StaticMethodCallExpression lowering.
 * The callee_address is constructed based on the
 * target_method.
 */
extern void
lower_static_method_call_expression( StaticMethodCallExpression* smce );


#endif
