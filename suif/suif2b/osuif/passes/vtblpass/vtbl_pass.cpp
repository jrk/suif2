// $Id: vtbl_pass.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

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
#include "osuifutilities/search_utils.h"
#include "vtblnodes/vtbl_utils.h"

#include "vtblpass/vtbl_pass.h"


VtblPass::VtblPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<ClassType>( env, name )
{
  _vtbl_constr = new SingleInheritanceVtbls( env );
}


void VtblPass::initialize_flags() {
  _construct_vtbl = false;
  _attach_vtbl_slot_number_annotes = false;
  _verbose = false;
}


void VtblPass::initialize(){
  CollectWalkerPass<ClassType>::initialize();

  _command_line->set_description(
    "This pass constructs virtual function tables for ClassTypes." );

  // -construct-vtbls
  OptionLiteral* vtbl_option =
    new OptionLiteral( "-construct-vtbls",
		       &_construct_vtbl,
		       !_construct_vtbl );
  vtbl_option->set_description(
    "Build vtbls for for ClassTypes with complete methods." );

  // -attach-vtbl-slot-number-annotes
  OptionLiteral* vtbl_annotes_option =
    new OptionLiteral( "-attach-vtbl-slot-number-annotes",
		       &_attach_vtbl_slot_number_annotes,
		       !_attach_vtbl_slot_number_annotes );
  vtbl_annotes_option->set_description(
    "Attach annotes to all dispatched InstanceMethodSymbols in ClassTypes." );
  
  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the vtbl layout.");


  // Make sure flags can be repeated
  OptionSelection* opt_flags = new OptionSelection( false );
  OptionLoop* loop = new OptionLoop( opt_flags );
  _command_line->add( loop );

  opt_flags->add( vtbl_option );
  opt_flags->add( vtbl_annotes_option );
  opt_flags->add( verbose_option );
}


bool VtblPass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<ClassType>::parse_command_line( ts );
}


/*
 * A vtbl for 'ctype' is constructed. The VariableSymbol
 * that holds the vtbl is inserted in the PerClassSymbolTable.
 */
void VtblPass::process_suif_object( ClassType* ctype ) {
  SingleInheritanceClassType* sictype =
    to<SingleInheritanceClassType>( ctype );

  if( _attach_vtbl_slot_number_annotes ) {
    _vtbl_constr->attach_vtbl_slot_number_annotes( sictype );

    if(  _verbose ) {
      cout << "vtbl: VtblSlotNumberAnnotes attached for "
	   << ctype->get_name().c_str() << " ClassType." << endl;
    }
  }

  if( _construct_vtbl ) {
    if( has_no_vtbl_annote(ctype) )
      return;

    if( ! ctype->get_methods_are_complete() )
      return;
  
    _vtbl_constr->attach_vtbl( sictype );

    if( _verbose ) {
      cout << "vtbl: Vtbl for "
	   << ctype->get_name().c_str() << " ClassType:" << endl;
      _vtbl_constr->print_vtbl( sictype );
    }
  }
}
