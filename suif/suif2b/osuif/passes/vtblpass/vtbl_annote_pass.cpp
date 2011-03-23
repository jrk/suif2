// $Id: vtbl_annote_pass.cpp,v 1.2 2000/06/27 16:54:11 brm Exp $

#include <iostream.h>

#include "iokernel/cast.h"
#include "suifnodes/suif.h"
#include "suifpasses/suifpasses.h"
#include "typebuilder/type_builder.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_factory.h"
#include "suifkernel/command_line_parsing.h" 
#include "suifkernel/token_stream.h" 
#include "suifkernel/suifkernel_messages.h"
#include "osuifutilities/search_utils.h"
#include "osuifutilities/walker_utils.h"
#include "vtblnodes/vtbl_utils.h"

#include "vtblpass/vtbl_annote_pass.h"


VtblAnnotePass::VtblAnnotePass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<VtblAnnote>( env, name )
{
  _vtbl_constr = new SingleInheritanceVtbls( env );
}


void VtblAnnotePass::initialize_flags() {
  _verbose = false;
}


void VtblAnnotePass::initialize(){
  CollectWalkerPass<VtblAnnote>::initialize();

  _command_line->set_description(
    "This pass attaches virtual function tables to VariableSymbols"
    " that have a VtblAnnote." );
  
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


bool VtblAnnotePass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<VtblAnnote>::parse_command_line( ts );
}


/*
 * A vtbl for 'ctype' is constructed. The VariableSymbol
 * that holds the vtbl is inserted in the PerClassSymbolTable.
 */
void VtblAnnotePass::process_suif_object( VtblAnnote* annote ) {
  VariableSymbol* vsym =
    to<VariableSymbol>( annote->get_parent() );
  ClassType* ctype = annote->get_owning_class();
  
  SingleInheritanceClassType* sictype =
    to<SingleInheritanceClassType>( ctype );

  // @@@
  //  if( ! ctype->get_methods_are_complete() )
  //    return;
  
  if ( _verbose ) {
    cout << "VtblAnnote-pass: Vtbl for "
	 << ctype->get_name().c_str() << " ClassType appended at "
	 << vsym->get_name().c_str() << endl;
  }

  _vtbl_constr->attach_vtbl( sictype, vsym );

  // Fix type of Expressions that reference the vtbl VariableSymbol:
  // - SymbolAddressExpressions
  // - LoadVariableExpression    (@@@ necessary?)
  // - StoreVariableStatements   (@@@ necessary?)
  // - ... @@@
  // @@@ transitively??? Actually, yes...

  TypeBuilder* tb = get_type_builder( _suif_env );
  
  QualifiedType* vsym_type = vsym->get_type();
  suif_assert( vsym_type );

  // SymbolAddressExpressions
  CollectWalkerT<SymbolAddressExpression> walker( _suif_env );
  _fsb->walk( walker );

  for ( unsigned i = 0 ; i < walker.size() ; i++ ) {
    SymbolAddressExpression* expr = walker.at(i);

    if( expr->get_result_type() != NULL )
      continue;

    if ( _verbose ) {
      cout << "VtblAnnote-pass: Patched result_type for"
	   << " SymbolAddressExpression." << endl;
    }

    expr->set_result_type( tb->get_pointer_type(vsym_type) );
  }
  
  // @@@
  if ( _verbose ) {
    _vtbl_constr->print_vtbl( sictype );
  }
}
