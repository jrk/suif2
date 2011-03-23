// $Id: instancefieldslayout_utils.cpp,v 1.2 2000/06/12 01:20:32 dlheine Exp $

#include <iostream.h>

#include "common/suif_list.h"
#include "typebuilder/type_builder.h"
#include "utils/expression_utils.h"
#include "utils/cloning_utils.h"
#include "osuifutilities/problems.h"
#include "osuifutilities/type_utils.h"

#include "instancefieldslayoutnodes/instancefieldslayout_utils.h"


LString k_instancefieldslayout_offset_annote
  ("osuif_instancefieldslayout_offset_annote");

LString k_instancefieldslayout_complete_annote
  ("osuif_instancefieldslayout_complete_annote");


void attach_instancefieldslayout_offset_annote( SuifEnv* env,
						IntConstant* icnst,
						InstanceFieldSymbol* fsym,
						ClassType* owning_class )
{
  InstanceFieldOffsetAnnote* annote =
    create_instance_field_offset_annote( env,
					 k_instancefieldslayout_offset_annote,
					 fsym );
  annote->set_owning_class( owning_class );
  icnst->append_annote( annote );
}


IntConstant*
build_instancefieldslayout_offset_int_constant( SuifEnv* env,
						InstanceFieldSymbol* fsym,
						ClassType* owning_class )
{
  TypeBuilder* tb = get_type_builder( env );

  IntConstant* icnst =
    ::create_int_constant( env,
			   tb->get_integer_type( true ), // result_type
			   IInteger() // value: inserted later
			   );
  attach_instancefieldslayout_offset_annote( env, icnst, fsym, owning_class );
  return icnst;
}


void attach_instancefieldslayout_complete_annote( SuifEnv* env,
						  ClassType* ctype )
{
  InstanceFieldLayoutCompleteAnnote* annote =
    ::create_instance_field_layout_complete_annote( env, k_instancefieldslayout_complete_annote );
  ctype->append_annote( annote );
}


bool has_instancefieldslayout_complete_annote( ClassType* ctype ) {
  return
    ctype->peek_annote( k_instancefieldslayout_complete_annote );
}

/*************************************************************************/

InstanceFieldsLayout::InstanceFieldsLayout( SuifEnv* env ) :
  _env( env ),
  _tb( NULL )
{
  _tb = get_type_builder( _env );
}


InstanceFieldSymbol* InstanceFieldsLayout::
clone_instance_field_symbol( InstanceFieldSymbol* fsym ) {
  return deep_suif_clone<InstanceFieldSymbol>( fsym );
}


/*
 * Called by collect_instance_fields() if an incomplete
 * SingleInheritanceClassType is encountered.
 *
 * Default behavior is to issue a warning message.
 */
void InstanceFieldsLayout::
handle_incomplete_class( SingleInheritanceClassType* ctype ) {
#if 0 
  // @@@ our java dismantler actually expects missing 
  // info for all but the single class being compiled at a single time.
  OsuifProblems::warning( "instancefieldslayout: ClassType `%s'"
			  " has incomplete fields.",
			  ctype->get_name().c_str() );
#endif
}


/*
 * After the call 'fsyms' holds the list of InstanceFieldSymbols
 * that constitute the object layout for 'ctype'.
 * Note that 'fsyms' contains the original (i.e. uncloned!)
 * InstanceFieldSymbols.
 */ 
void InstanceFieldsLayout::
collect_instance_fields( SingleInheritanceClassType* ctype,
			 list<InstanceFieldSymbol* >* fsyms )
{
  if( ctype == NULL )
    return;

  if( ! ctype->get_fields_are_complete() )
    handle_incomplete_class( ctype );

  // get object layout of super class if current class
  // has not a complete layout
  if( ! has_instancefieldslayout_complete_annote(ctype) ) {
    collect_instance_fields( ctype->parent_class(), fsyms );
  }
  
  GroupSymbolTable* gst =
    ctype->get_group_symbol_table();
  Iter <SymbolTableObject*> iter =
    gst->get_symbol_table_object_iterator();

  while ( iter.is_valid() ) {
    InstanceFieldSymbol* fsym =
      to<InstanceFieldSymbol>( iter.current() );
      
    fsyms->push_back( fsym );
    iter.next();
  }
}


/*
 * Add precessor classes' instance fields to 'ctype'
 * in order to get the complete class layout.
 * The precessor classes' instance fields are cloned! This
 * means that references (i.e., from FieldAccessExpressions
 * still point to the original FieldSymbol.
 */
void InstanceFieldsLayout::
do_instance_field_layout( SingleInheritanceClassType* ctype ) {  
  suif_assert( ctype );

  if( ! ctype->get_fields_are_complete() )
    handle_incomplete_class( ctype );

  if( ctype->parent_class() == NULL )
    return;

  // collect all precessor classes' fields (excluding the current class)
  list<InstanceFieldSymbol* >* fsyms_super =
    new list<InstanceFieldSymbol* >();
  collect_instance_fields( ctype->parent_class(), fsyms_super );

  // remove fields in symtab of current class
  // @@@ (SUIF interface bug)
  list<InstanceFieldSymbol* > fsyms_current;
  GroupSymbolTable* gst =
    ctype->get_group_symbol_table();
  Iter <SymbolTableObject*> iter =
    gst->get_symbol_table_object_iterator();
  while ( iter.is_valid() ) {
    InstanceFieldSymbol* fsym =
      to<InstanceFieldSymbol>( iter.current() );      
    fsyms_current.push_back( fsym );
    iter.next();
  }
  for ( unsigned i = 0 ; i < fsyms_current.size() ; i++ ) {
    InstanceFieldSymbol* fsym = fsyms_current[i];    
    gst->remove_symbol( fsym );
  }
  
  // add cloned(!) precessor classes' fields
  for ( unsigned i = 0 ; i < fsyms_super->size() ; i++ ) {
    InstanceFieldSymbol* fsym = (*fsyms_super)[i];    
    gst->add_symbol( clone_instance_field_symbol(fsym) );
  }
  
  // add current class's fields
  for ( unsigned i = 0 ; i < fsyms_current.size() ; i++ ) {
    InstanceFieldSymbol* fsym = fsyms_current[i];
    gst->add_symbol( fsym );
  }

  // mark that class has complete layout
  ::attach_instancefieldslayout_complete_annote( _env, ctype );
}


void InstanceFieldsLayout::
print_instance_field_layout( SingleInheritanceClassType* ctype ) {
  BasicSymbolTable* symtab =
    ctype->get_group_symbol_table();
  Iter<SymbolTableObject*> iter =
    symtab->get_symbol_table_object_iterator();

  while( iter.is_valid() ) {
    InstanceFieldSymbol* fsym =
      to<InstanceFieldSymbol>( iter.current() );

    IInteger offset =
      ::get_expression_constant( fsym->get_bit_offset() );
    //    offset.print( *cout );
    
    cout << "  " <<  offset.to_string() << ": "
	 << fsym->get_name().c_str() << endl;
    
    iter.next();
  }
}
