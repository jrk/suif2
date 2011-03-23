// $Id: slot_count_annote_pass.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include <iostream.h>

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "suifpasses/suifpasses.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "suifkernel/command_line_parsing.h" 
#include "suifkernel/token_stream.h" 
#include "suifkernel/suifkernel_messages.h"
#include "utils/expression_utils.h"
#include "osuifutilities/search_utils.h"
#include "vtblnodes/vtbl_utils.h"

#include "vtblpass/slot_count_annote_pass.h"


VtblSlotCountAnnotePass::VtblSlotCountAnnotePass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<VtblSlotCountAnnote>( env, name )
{
}


void VtblSlotCountAnnotePass::initialize_flags() {
  _verbose = false;
}


void VtblSlotCountAnnotePass::initialize(){
  CollectWalkerPass<VtblSlotCountAnnote>::initialize();

  _command_line->set_description(
    "This pass handles VtblSlotCountAnnotes." );

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


bool VtblSlotCountAnnotePass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<VtblSlotCountAnnote>::parse_command_line( ts );
}


void VtblSlotCountAnnotePass::
process_suif_object( VtblSlotCountAnnote* annote ) {
  IntConstant* icnst =
    to<IntConstant>( annote->get_parent() );
  SingleInheritanceClassType* ctype =
    to<SingleInheritanceClassType>( annote->get_owning_class() );

  if (_verbose ) {
    cout << "VtblSlotCountAnnote-pass: Handled "
      	 << ctype->get_name().c_str() << " ClassType." << endl;
  }

  // The vtbl count is computed by finding the biggest vtbl slot
  IInteger max_slot = ::biggest_vtbl_slot( ctype );
  
  // @@@ Empty vtbl should be legal, but is unlikely.
  // For debugging only!
  suif_assert( max_slot >= 0 );

  icnst->set_value( max_slot + 1 );
}
