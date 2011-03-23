// $Id: static_method_call_pass.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef OSUIF_LOWERING__STATIC_METHOD_CALL_LOWERING_PASS_H
#define OSUIF_LOWERING__STATIC_METHOD_CALL_LOWERING_PASS_H

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

#include "osuiflowering/lowering.h"


class StaticMethodCallStatementLoweringPass :
  public CollectWalkerPass<StaticMethodCallStatement>
{
private:
  bool _verbose;
  
public:
  StaticMethodCallStatementLoweringPass( SuifEnv* env,
				const LString &name= "lower_static_method_call_statements" );
  virtual ~StaticMethodCallStatementLoweringPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( StaticMethodCallStatement* smcs );
};


class StaticMethodCallExpressionLoweringPass :
  public CollectWalkerPass<StaticMethodCallExpression>
{
private:
  bool _verbose;
  
public:
  StaticMethodCallExpressionLoweringPass( SuifEnv* env,
				const LString &name= "lower_static_method_call_expressions" );
  virtual ~StaticMethodCallExpressionLoweringPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( StaticMethodCallExpression* smcs );
};


#endif /* OSUIF_LOWERING__STATIC_METHOD_CALL_LOWERING_PASS_H */
