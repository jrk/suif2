// $Id: static_method_call_pass.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

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
#include "osuiflowering/static_method_call_utils.h"

#include "osuiflowering/static_method_call_pass.h"


StaticMethodCallStatementLoweringPass::
StaticMethodCallStatementLoweringPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<StaticMethodCallStatement>( env , name )
{
}


void StaticMethodCallStatementLoweringPass::initialize_flags() {
  _verbose = false;
}


void StaticMethodCallStatementLoweringPass::initialize() {
  CollectWalkerPass<StaticMethodCallStatement>::initialize();

  _command_line->set_description(
    "This pass lowers StaticMethodCallExpressions." );

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the lowered statements.");

  _flags->add( verbose_option );
}


bool StaticMethodCallStatementLoweringPass::parse_command_line(TokenStream* ts) {
  initialize_flags();
  return CollectWalkerPass<StaticMethodCallStatement>::parse_command_line( ts );
}


void StaticMethodCallStatementLoweringPass::
process_suif_object( StaticMethodCallStatement* smcs ) {
  // @@@ Should this assertion stay?
  suif_assert( ! smcs->get_callee_address() );
  
  if ( _verbose ) {
    StaticMethodSymbol* msym = smcs->get_target_method();
    cout << "StaticMethodCallStatement: "
	 << msym->get_name().c_str()
	 << " lowered." << endl;
  }

  ::lower_static_method_call_statement( smcs );
}

/********************************************************************/

StaticMethodCallExpressionLoweringPass::
StaticMethodCallExpressionLoweringPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<StaticMethodCallExpression>( env , name )
{
}


void StaticMethodCallExpressionLoweringPass::initialize_flags() {
  _verbose = false;
}


void StaticMethodCallExpressionLoweringPass::initialize() {
  CollectWalkerPass<StaticMethodCallExpression>::initialize();

  _command_line->set_description(
    "This pass lowers StaticMethodCallExpressions." );

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the lowered expressions.");

  _flags->add( verbose_option );
}


bool StaticMethodCallExpressionLoweringPass::parse_command_line(TokenStream* ts) {
  initialize_flags();
  return CollectWalkerPass<StaticMethodCallExpression>::parse_command_line( ts );
}


void StaticMethodCallExpressionLoweringPass::
process_suif_object( StaticMethodCallExpression* smc ) {
  // @@@ Should this assertion stay?
  suif_assert( ! smc->get_callee_address() );
  
  if ( _verbose ) {
    StaticMethodSymbol* msym = smc->get_target_method();
    cout << "StaticMethodCallExpression: "
	 << msym->get_name().c_str()
	 << " lowered." << endl;
  }

  ::lower_static_method_call_expression( smc );
}
