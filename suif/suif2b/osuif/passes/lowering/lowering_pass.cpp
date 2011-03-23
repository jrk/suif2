// $Id: lowering_pass.cpp,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

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

#include "osuiflowering/lowering_pass.h"


LoweringPass::LoweringPass(SuifEnv *env, const LString &name) :
  Pass(env, name),
  _gwalker( new GroupWalker(env) )
{
  initialize_flags();
}


void LoweringPass::initialize_flags() {
  _lower_instance_methods = false;
  _lower_static_fields = false;
  _lower_static_methods = false;
  _lower_per_class_symbol_table = false;
  _not_use_annote_name = false;

  _symtab = String("");
  _name_mangling = String("");
  _verbose = false;
}


void LoweringPass::do_file_set_block( FileSetBlock* fsb ) {
  suif_assert_message( fsb !=NULL,
		       ("FileSetBlock is NULL.") );
  
  append_walkers();
  fsb->walk( *_gwalker );
  process_walkers();
}


void LoweringPass::append_walkers() {
  // Collects all ClassTypes
  CollectWalkerT<ClassType>* walker =
    new CollectWalkerT<ClassType>( get_suif_env() );

  append_walker( walker );
}


void LoweringPass::append_walker(CollectWalker* walker) {
  _walkers.push_back( walker );
  _gwalker->append_walker( *walker );
}


void LoweringPass::process_walkers() {
  for ( unsigned i = 0 ; i < _walkers.size() ; i++ ) {
    process_objects( _walkers[i]->get_hit_list() );
  }
}



SymbolTable* LoweringPass::destination_symtab( SymbolTableObject* sym ) {
  FileSetBlock* fsb = get_suif_env()->get_file_set_block();

  if ( _symtab == String("external") )
    return fsb->get_external_symbol_table();

  if ( _symtab == String("file_set") )
    return fsb->get_file_set_symbol_table();

  if ( _symtab == String("file_block") ) {
    FileBlock* fb = find_ancestor<FileBlock>( sym );
    if ( fb == NULL ) {
      // Same symtab (i.e., do nothing)
      return sym->get_symbol_table();
    } else {
      return fb->get_symbol_table();
    }
  }

  if ( _symtab == String("one-up") ) {
    SymbolTable* symtab = find_ancestor<SymbolTable>( sym->get_parent() );
    if ( symtab == NULL ) {
      // Same symtab (i.e., do nothing)
      return sym->get_symbol_table();
    } else {
      return symtab;
    }
  }
 
  // Default: "file_set"
  return fsb->get_file_set_symbol_table();
}


const LString LoweringPass::mangled_sto_name( SymbolTableObject* sto ) {
  if ( ! _not_use_annote_name ) {
    // @@@ check for `lowering-name' annote
  }
  
  if ( _name_mangling == String("_") ) {
    SymbolTable* sto_symtab = sto->get_symbol_table();
    ClassType* sto_ctype= to<ClassType>( sto_symtab->get_parent() );
    
    return sto_ctype->get_name() + "__" + sto->get_name();
  }

  // Default: no mangling
  return sto->get_name();
}


void LoweringPass::process_objects( list<SuifObject* > obj_list ) {
  for ( unsigned i = 0 ; i < obj_list.size() ; i++ ) {
    ClassType* ctype = to<ClassType>( obj_list[i] );

    if ( _verbose ) {
      cout << "Symbols-lowering: " << ctype->get_name().c_str()
	   << " ClassType" << endl;
    }

    if ( _lower_instance_methods ) {
      list<SymbolTableObject* > sto_list =
 	collect_stos<InstanceMethodSymbol>( ctype->get_instance_method_symbol_table() );    
      lower_stos( sto_list );
    }

    if ( _lower_static_methods ) {
      list<SymbolTableObject* > sto_list =
	collect_stos<StaticMethodSymbol>( ctype->get_per_class_symbol_table() );
      lower_stos( sto_list );
    }
    
    if ( _lower_static_fields ) {
      list<SymbolTableObject* > sto_list =
	collect_stos<StaticFieldSymbol>( ctype->get_per_class_symbol_table() );
      lower_stos( sto_list );
    }

    if ( _lower_per_class_symbol_table ) {
      list<SymbolTableObject* > sto_list =
	collect_stos<SymbolTableObject>( ctype->get_per_class_symbol_table() );
      lower_stos( sto_list );
    }

  }
}


void LoweringPass::lower_stos(list<SymbolTableObject* > sto_list) {
  for ( unsigned i = 0 ; i < sto_list.size() ; i++ ) {
    SymbolTableObject* sto = sto_list[i];
    const LString mangled_name = mangled_sto_name( sto );

    if ( _verbose ) {
      cout << "  " << sto->getClassName() 
	   << ": " << sto->get_name().c_str()
	   << " -> " << mangled_name << endl;
    }

    SymbolTable* src_symtab = sto->get_symbol_table();
    SymbolTable* dst_symtab = destination_symtab( sto );

    // Move the sto.
    // These methods take care of the bookkeeping
    // (as opposed to remove/append_symbol_table_object()).
    // Only the first name of the sto is moved!
    src_symtab->remove_symbol( sto );
    dst_symtab->add_symbol( sto );

    
    // Set the mangled name as the new name of the sto
    // (Calling sto->set_name() does not change the lookup table!)
    dst_symtab->change_name( sto, mangled_name );
  }
}


void LoweringPass::initialize(){
  Pass::initialize();

  _command_line->set_description(
    "This pass helps to convert the OSUIF representation to plain SUIF." );

  // -lower-instance-methods
  OptionLiteral* methods_option =
    new OptionLiteral( "-lower-instance-methods",
		       &_lower_instance_methods,
		       !_lower_instance_methods );
  methods_option->set_description("Lower instance methods of class type.");

  // -lower-static-field
  OptionLiteral* static_fields_option = new OptionLiteral(
				"-lower-static-fields",
				&_lower_static_fields,
				!_lower_static_fields);

  static_fields_option->set_description("Lower static fields of class type.");

  // -lower-static-methods
  OptionLiteral* static_methods_option = new OptionLiteral(
			      "-lower-static-methods",
			      &_lower_static_methods,
			      !_lower_static_methods);

  static_methods_option->set_description("Lower static methods of class type.");

  // -lower-per-class-symbol-table
  OptionLiteral* per_class_symtab_option = new OptionLiteral(
			      "-lower-per-class-symbol-table",
			      &_lower_per_class_symbol_table,
			      !_lower_per_class_symbol_table);

  per_class_symtab_option->set_description("Lower all entries in the PerClassSymbolTable");

  // -symtab "<which>"
  OptionLiteral* symtab_literal= new OptionLiteral( "-symtab" );
  OptionString *symtab_str =
    new OptionString("external|file_set|file_block|one-up", &_symtab);  
  OptionList* symtab_flag=
    (new OptionList())->add( symtab_literal )->add( symtab_str );
  symtab_flag->set_description(
    "Move symbol table objects to specified symbol table." );

  // -mangle-name "<how>"
  OptionLiteral* mn_literal= new OptionLiteral( "-mangle-name" );
  OptionString *mn_str =
    new OptionString("_", &_name_mangling);  
  OptionList* mn_flag=
    (new OptionList())->add( mn_literal )->add( mn_str );
  mn_flag->set_description(
    "Mangles the name of all class members that are lowered.");

  // -ignore-lowering-name-annote
  OptionLiteral* name_annote_option = new OptionLiteral(
			      "-ignore-lowering-name-annote",
			      &_not_use_annote_name,
			      !_not_use_annote_name);

  name_annote_option->set_description(
    "Ignore `lowering-name' annotation for name mangling.");

  // -verbose
  OptionLiteral* verbose_option = new OptionLiteral( "-verbose",
						     &_verbose,
						     !_verbose );
  verbose_option->set_description("Print information about the lowered symbols.");


  // Make sure flags can be repeated
  OptionSelection* opt_flags = new OptionSelection( false );
  OptionLoop* loop = new OptionLoop( opt_flags );
  _command_line->add( loop );

  opt_flags->add( methods_option );
  opt_flags->add( static_fields_option );
  opt_flags->add( static_methods_option );
  opt_flags->add( per_class_symtab_option );
  opt_flags->add( mn_flag );
  opt_flags->add( symtab_flag );
  opt_flags->add( name_annote_option );
  opt_flags->add( verbose_option );
}


bool LoweringPass::parse_command_line(TokenStream *stream) {
  initialize_flags();

  return Pass::parse_command_line( stream );
}
