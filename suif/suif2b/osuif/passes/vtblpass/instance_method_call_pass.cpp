// $Id: instance_method_call_pass.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include <iostream.h>

#include "suifkernel/command_line_parsing.h" 
#include "suifkernel/token_stream.h" 
#include "suifkernel/suifkernel_messages.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "suifpasses/suifpasses.h"
#include "typebuilder/type_builder.h"
#include "vtblpass/instance_method_call_utils.h"

#include "vtblpass/instance_method_call_pass.h"


InstanceMethodCallStatementLoweringPass::
InstanceMethodCallStatementLoweringPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<InstanceMethodCallStatement>( env , name )
{
  _lower_instance_method_calls = new LowerInstanceMethodCalls();
}


void InstanceMethodCallStatementLoweringPass::initialize_flags() {
  _verbose = false;
}


void InstanceMethodCallStatementLoweringPass::initialize() {
  CollectWalkerPass<InstanceMethodCallStatement>::initialize();

  _command_line->set_description(
    "This pass lowers InstanceMethodCallExpressions." );

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the lowered statements.");


  // Make sure flags can be repeated
  OptionSelection* opt_flags = new OptionSelection( false );
  OptionLoop* loop = new OptionLoop( opt_flags );
  _command_line->add( loop );

  opt_flags->add( verbose_option );
}


bool InstanceMethodCallStatementLoweringPass::parse_command_line(TokenStream* ts) {
  initialize_flags();
  return CollectWalkerPass<InstanceMethodCallStatement>::parse_command_line( ts );
}


void InstanceMethodCallStatementLoweringPass::
process_suif_object( InstanceMethodCallStatement* stmt ) {
  _lower_instance_method_calls->lower_method_call_stmt( stmt );

  if ( _verbose ) {
    InstanceMethodSymbol* msym = stmt->get_target_method();
    cout << "InstanceMethodCallStatement: "
	 << msym->get_name().c_str()
	 << " lowered." << endl;
  }
}

/********************************************************************/

InstanceMethodCallExpressionLoweringPass::
InstanceMethodCallExpressionLoweringPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<InstanceMethodCallExpression>( env , name )
{
  _lower_instance_method_calls = new LowerInstanceMethodCalls();
}


void InstanceMethodCallExpressionLoweringPass::initialize_flags() {
  _verbose = true; // @@@
}


void InstanceMethodCallExpressionLoweringPass::initialize() {
  CollectWalkerPass<InstanceMethodCallExpression>::initialize();

  _command_line->set_description(
    "This pass lowers InstanceMethodCallExpressions." );

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the lowered expressions.");

  _flags->add( verbose_option );
}


bool InstanceMethodCallExpressionLoweringPass::parse_command_line(TokenStream* ts) {
  initialize_flags();
  return CollectWalkerPass<InstanceMethodCallExpression>::parse_command_line( ts );
}


void InstanceMethodCallExpressionLoweringPass::
process_suif_object( InstanceMethodCallExpression* expr ) {
  if ( _verbose ) {
    InstanceMethodSymbol* msym = expr->get_target_method();
    cout << "InstanceMethodCallExpression: "
      	 << msym->get_owning_class()->get_name().c_str() << " " // @@@
	 << msym->get_name().c_str()
	 << " lowered." << endl;
  }

  _lower_instance_method_calls->lower_method_call_expr( expr );
}
