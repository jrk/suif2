// $Id: slot_annote_pass.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

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
#include "vtblnodes/vtbl_utils.h"

#include "vtblpass/slot_annote_pass.h"


VtblSlotAnnotePass::VtblSlotAnnotePass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<VtblSlotAnnote>( env, name )
{
}


void VtblSlotAnnotePass::initialize_flags() {
  _verbose = false;
}


void VtblSlotAnnotePass::initialize(){
  CollectWalkerPass<VtblSlotAnnote>::initialize();

  _command_line->set_description(
    "This pass handles VtblSlotAnnotes." );

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


bool VtblSlotAnnotePass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<VtblSlotAnnote>::parse_command_line( ts );
}


void VtblSlotAnnotePass::process_suif_object( VtblSlotAnnote* annote ) {
  IntConstant* icnst =
    to<IntConstant>( annote->get_parent() );
  InstanceMethodSymbol* msym = annote->get_vtbl_slot();

  if (_verbose ) {
    cout << "vtbl: Handled VtblSlotAnnote for "
      //	 << ctype->get_name().c_str() << " "
	 << msym->get_name().c_str() << "." << endl;
  }

  VtblSlotNumberAnnote* slot_number_annote =
    to<VtblSlotNumberAnnote>( msym->peek_annote(k_vtbl_slot_number_annote) );
  suif_assert( slot_number_annote );

  IInteger slot = slot_number_annote->get_slot_number();

  icnst->set_value( slot );
}
