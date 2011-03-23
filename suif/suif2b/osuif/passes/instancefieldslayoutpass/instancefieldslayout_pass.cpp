// $Id: instancefieldslayout_pass.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

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
#include "osuifutilities/type_utils.h"

#include "instancefieldslayoutnodes/instancefieldslayout_utils.h"

#include "instancefieldslayoutpass/instancefieldslayout_pass.h"


InstanceFieldsLayoutPass::
InstanceFieldsLayoutPass( SuifEnv* env, const LString& name ) :
  CollectWalkerPass<ClassType>( env, name )
{
  _layout = new InstanceFieldsLayout( env );
}


void InstanceFieldsLayoutPass::initialize_flags() {
  _name_mangling = String("");
  _not_use_annote_name = false;
  _verbose = false;
}


void InstanceFieldsLayoutPass::initialize(){
  CollectWalkerPass<ClassType>::initialize();

  _command_line->set_description(
    "This pass constructs the instance field layout for ClassTypes." );

  // -mangle-name "<how>"
  OptionLiteral* mn_literal= new OptionLiteral( "-mangle-name" );
  OptionString *mn_str =
    new OptionString("_", &_name_mangling);  
  OptionList* mn_flag=
    (new OptionList())->add( mn_literal )->add( mn_str );
  mn_flag->set_description(
    "Mangles the names of the instance fields names." );

  // -ignore-lowering-name-annote
  OptionLiteral* name_annote_option = new OptionLiteral(
			      "-ignore-name-annote",
			      &_not_use_annote_name,
			      !_not_use_annote_name);
  name_annote_option->set_description(
    "Ignore `lowering-name' annotation for name mangling.");

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the object layout.");


  // Make sure flags can be repeated
  OptionSelection* opt_flags = new OptionSelection( false );
  OptionLoop* loop = new OptionLoop( opt_flags );
  _command_line->add( loop );

  opt_flags->add( mn_flag );
  opt_flags->add( name_annote_option );
  opt_flags->add( verbose_option );
}


bool InstanceFieldsLayoutPass::parse_command_line(TokenStream *ts) {
  initialize_flags();
  return CollectWalkerPass<ClassType>::parse_command_line( ts );
}


const LString InstanceFieldsLayoutPass::
mangled_name( InstanceFieldSymbol* fsym ) {
  if ( ! _not_use_annote_name ) {
    // @@@ check for `lowering-name' annote
  }
  
  if ( _name_mangling == String("_") ) {
    if( fsym->get_owning_class() == NULL )
      return fsym->get_name();

    return fsym->get_owning_class()->get_name() + "__" + fsym->get_name();
  }

  // Default: no mangling
  return fsym->get_name();
}


void InstanceFieldsLayoutPass::mangle_fields( ClassType* ctype ) {
  BasicSymbolTable* symtab =
    ctype->get_group_symbol_table();
  Iter<SymbolTableObject*> iter =
    symtab->get_symbol_table_object_iterator();

  while( iter.is_valid() ) {
    InstanceFieldSymbol* fsym =
      to<InstanceFieldSymbol>( iter.current() );

    if ( _verbose ) {
      //      cout << fsym->get_name().c_str() << " -> "
      //	   << mangled_name(fsym).c_str() << endl;
    }

    symtab->change_name( fsym, mangled_name(fsym) );
    iter.next();
  }
}


void InstanceFieldsLayoutPass::process_suif_object( ClassType* ctype ) {
  SingleInheritanceClassType* sictype =
    to<SingleInheritanceClassType>( ctype );
  
  if ( _verbose ) {
    cout << "Instance field layout for "
	 << sictype->get_name().c_str() << " ClassType:" << endl;
  }
  
  _layout->do_instance_field_layout( sictype );
  mangle_fields( ctype );

  ::do_class_type_layout( sictype );

  if ( _verbose ) {
    _layout->print_instance_field_layout( sictype );
  }
}
