// $Id: instance_method_call_pass.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBLPASS__INSTANCE_METHOD_CALL_PASS_H
#define VTBLPASS__INSTANCE_METHOD_CALL_PASS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/suif_object.h"
#include "suifpasses/suifpasses.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"
#include "osuifutilities/pass_utils.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"
#include "vtblpass/instance_method_call_utils.h"


class InstanceMethodCallStatementLoweringPass :
  public CollectWalkerPass<InstanceMethodCallStatement>
{
private:
  LowerInstanceMethodCalls* _lower_instance_method_calls;
  bool _verbose;
  
public:
  InstanceMethodCallStatementLoweringPass( SuifEnv* env,
    const LString &name= "lower_instance_method_call_statements" );
  virtual ~InstanceMethodCallStatementLoweringPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( InstanceMethodCallStatement* smcs );
};


class InstanceMethodCallExpressionLoweringPass :
  public CollectWalkerPass<InstanceMethodCallExpression>
{
private:
  LowerInstanceMethodCalls* _lower_instance_method_calls;
  bool _verbose;
  
public:
  InstanceMethodCallExpressionLoweringPass( SuifEnv* env,
					    const LString &name= "InstanceMethodCallExpression-pass" );
  virtual ~InstanceMethodCallExpressionLoweringPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( InstanceMethodCallExpression* smcs );
};


#endif
