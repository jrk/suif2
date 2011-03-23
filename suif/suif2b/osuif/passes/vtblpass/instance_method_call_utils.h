// $Id: instance_method_call_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBLPASS__INSTANCE_METHOD_CALL_UTILS_H
#define VTBLPASS__INSTANCE_METHOD_CALL_UTILS_H

#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "cfenodes/cfe_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifextensionnodes/osuifextension_forwarders.h"


class LowerInstanceMethodCalls {
public:
  //  LowerInstanceMethodCalls() { }
  //  virtual ~LowerInstanceMethodCalls)() { }

  // InstanceMethodCallStatement

  virtual void
  lower_method_call_stmt( InstanceMethodCallStatement* stmt );
  virtual void
  lower_dispatched_method_call_stmt( InstanceMethodCallStatement* stmt );
  virtual void
  lower_undispatched_method_call_stmt( InstanceMethodCallStatement* stmt );

  // InstanceMethodCallExpression

  virtual Expression*
  get_this_reference( InstanceMethodCallExpression* expr );
  virtual InstanceFieldSymbol*
  get_vtbl_field_symbol( SingleInheritanceClassType* ctype );

  virtual void
  lower_method_call_expr( InstanceMethodCallExpression* expr );
  virtual void
  lower_dispatched_method_call_expr( InstanceMethodCallExpression* expr );
  virtual void
  lower_undispatched_method_call_expr( InstanceMethodCallExpression* expr );
};


#endif
