// $Id: definition_block_pass.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

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

#include "osuiflowering/definition_block_pass.h"


DefinitionBlockPass::
DefinitionBlockPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<ClassType>( env, name )
{
}


void DefinitionBlockPass::initialize_flags() {
  _verbose = false;
}


void DefinitionBlockPass::initialize(){
  CollectWalkerPass<ClassType>::initialize();

  _command_line->set_description(
    "This pass lowers entries in ClassTypes' DefinitionBlocks." );
  
  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about lowering.");

  _flags->add( verbose_option );
}


bool DefinitionBlockPass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<ClassType>::parse_command_line( ts );
}


void DefinitionBlockPass::process_suif_object( ClassType* ctype ) {
  DefinitionBlock* dblock = ctype->get_definition_block();
  if( dblock == NULL ) return;

  if ( _verbose ) {
    cout << "DefinitionBlock-lowering: " << ctype->get_name().c_str()
	 << " ClassType" << endl;
  }

  FileBlock* fb = ::get_associated_file_block( ctype );
  DefinitionBlock* target_dblock = fb->get_definition_block();

  while( dblock->get_variable_definition_count() > 0 ) {
    VariableDefinition* vdef =
      dblock->remove_variable_definition( 0 );
    target_dblock->append_variable_definition( vdef );

    if( _verbose ) {
      VariableSymbol* vsym = vdef->get_variable_symbol();
      cout << "  " << vsym->getClassName() << ": ";
      if( vsym != NULL )
	cout << vsym->get_name().c_str() << endl;
      else
	cout << "??" << endl;
    }
  }
  
  while( dblock->get_procedure_definition_count() > 0 ) {
    ProcedureDefinition* vdef =
      dblock->remove_procedure_definition( 0 );
    target_dblock->append_procedure_definition( vdef );

    if( _verbose ) {
      ProcedureSymbol* psym = vdef->get_procedure_symbol();
      cout << "  " << psym->getClassName() << ": ";
      if( psym != NULL )
	cout << psym->get_name().c_str() << endl;
      else
	cout << "??" << endl;
    }
  }  
}
