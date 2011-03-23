// $Id: offset_annote_pass.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#include <iostream.h>

#include "suifnodes/suif.h"
#include "suifpasses/suifpasses.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "suifkernel/command_line_parsing.h" 
#include "suifkernel/token_stream.h" 
#include "suifkernel/suifkernel_messages.h"
#include "utils/expression_utils.h"
#include "typebuilder/type_builder.h"

#include "instancefieldslayoutnodes/instancefieldslayout_utils.h"

#include "instancefieldslayoutpass/offset_annote_pass.h"


InstanceFieldsLayoutOffsetAnnotePass::
InstanceFieldsLayoutOffsetAnnotePass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<InstanceFieldOffsetAnnote>( env, name )
{
}


void InstanceFieldsLayoutOffsetAnnotePass::initialize_flags() {
  _verbose = false;
}


void InstanceFieldsLayoutOffsetAnnotePass::initialize(){
  CollectWalkerPass<InstanceFieldOffsetAnnote>::initialize();

  _command_line->set_description(
    "This pass handles InstanceFieldOffsetAnnotes." );

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information.");


  // Make sure flags can be repeated
  OptionSelection* opt_flags = new OptionSelection( false );
  OptionLoop* loop = new OptionLoop( opt_flags );
  _command_line->add( loop );

  opt_flags->add( verbose_option );
}


bool InstanceFieldsLayoutOffsetAnnotePass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<InstanceFieldOffsetAnnote>::parse_command_line( ts );
}


void InstanceFieldsLayoutOffsetAnnotePass::
process_suif_object( InstanceFieldOffsetAnnote* annote ) {
  IntConstant* icnst =
    to<IntConstant>( annote->get_parent() );
  InstanceFieldSymbol* fsym =
    annote->get_instance_field();

  Expression* offset_expr = fsym->get_bit_offset();
  IInteger offset = ::get_expression_constant( offset_expr );

  TargetInformationBlock* iblock =
    ::find_target_information_block( annote->get_suif_env() );
  IInteger byte_size = iblock->get_byte_size();

  suif_assert( offset.is_divisible_by(byte_size) );
  icnst->set_value( offset.div(byte_size) );

  // @@@ trash annote?

  if (_verbose ) {
    cout << "instancefieldsoffset: Handled InstanceFieldOffsetAnnote"
	 << " for " << fsym->get_name().c_str() << "." << endl;
  }
}
