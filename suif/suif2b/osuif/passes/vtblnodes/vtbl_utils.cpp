// $Id: vtbl_utils.cpp,v 1.2 2000/06/12 01:20:32 dlheine Exp $

#include <iostream.h>

#include "osuifutilities/search_utils.h"
#include "osuifutilities/problems.h"
#include "typebuilder/type_builder.h"
#include "utils/value_block_utils.h"

#include "vtblnodes/vtbl_utils.h"


LString k_vtbl_annote("osuif_vtbl_annote");
LString k_vtbl_slot_annote("osuif_vtbl_slot_annote");
LString k_vtbl_slot_count_annote("osuif_vtbl_slot_count_annote");
LString k_vtbl_slot_number_annote("osuif_vtbl_slot_number_annote");
LString k_no_vtbl_annote("osuif_no_vtbl_annote");


void attach_vtbl_annote(  SuifEnv *env,
			  VariableSymbol* vtbl_sym,
			  ClassType* owning_class )
{
  VtblAnnote* vtbl_annote =
    ::create_vtbl_annote( env, k_vtbl_annote, owning_class );
  vtbl_sym->append_annote( vtbl_annote );
}


VariableSymbol* build_vtbl_variable_symbol( SuifEnv* env,
					    ClassType* owning_class,
					    const LString & name )
{
  VariableSymbol* vsym =
    ::create_variable_symbol( env,
			      NULL, // type: inserted later
			      name,
			      false // is_address_taken
			      );

  attach_vtbl_annote( env, vsym, owning_class );
  return vsym;
}


void attach_vtbl_slot_annote( SuifEnv* env,
			      IntConstant* icnst,
			      InstanceMethodSymbol* msym ) {
  VtblSlotAnnote* annote =
    ::create_vtbl_slot_annote( env,
			       k_vtbl_slot_annote,
			       msym );
  icnst->append_annote( annote );
}


IntConstant* build_vtbl_slot_int_constant( SuifEnv* env,
					   InstanceMethodSymbol* msym )
{
  TypeBuilder* tb = get_type_builder( env );

  IntConstant* icnst =
    ::create_int_constant( env,
			   tb->get_integer_type( true ), // result_type
			   IInteger() // value: inserted later
			   );
  attach_vtbl_slot_annote( env, icnst, msym );
  return icnst;
}


void attach_vtbl_slot_count_annote( SuifEnv* env,
				    IntConstant* icnst,
				    ClassType* owning_class )
{
  VtblSlotCountAnnote* annote =
    ::create_vtbl_slot_count_annote( env,
				     k_vtbl_slot_count_annote,
				     owning_class );
  icnst->append_annote( annote );
}


IntConstant* build_vtbl_slot_count_int_constant( SuifEnv* env,
						 ClassType* owning_class )
{
  TypeBuilder* tb = get_type_builder( env );

  IntConstant* icnst =
    ::create_int_constant( env,
			   tb->get_integer_type( true ), // result_type
			   IInteger() // value: inserted later
			   );
  attach_vtbl_slot_count_annote( env, icnst, owning_class );
  return icnst;
}


void attach_vtbl_slot_number_annote( SuifEnv *env,
				     InstanceMethodSymbol* msym,
				     IInteger slot_number )
{
  VtblSlotNumberAnnote* annote =
    ::create_vtbl_slot_number_annote( env,
				      k_vtbl_slot_number_annote,
				      slot_number );
  msym->append_annote( annote );
}


void attach_no_vtbl_annote( SuifEnv* env,
			    ClassType* ctype )
{
  NoVtblAnnote* annote =
    ::create_no_vtbl_annote( env, k_no_vtbl_annote );
  ctype->append_annote( annote );
}


bool has_no_vtbl_annote( ClassType* ctype ) {
  return ctype->peek_annote( k_no_vtbl_annote );
}

/*******************************************************************/

IInteger biggest_local_vtbl_slot( ClassType* ctype ) {
  list<InstanceMethodSymbol* > method_list =
    collect_stos<InstanceMethodSymbol,InstanceMethodSymbol>( ctype->get_instance_method_symbol_table() );    
  
  IInteger max_slot(-1);
  for ( unsigned i = 0 ; i < method_list.size() ; i++ ) {
    InstanceMethodSymbol* msym = method_list[i];

    VtblSlotNumberAnnote* slot_number_annote =
      to<VtblSlotNumberAnnote>( msym->peek_annote(k_vtbl_slot_number_annote) );

    if( slot_number_annote == NULL ) {
      // Ignore --- method is not dispatched
      continue;
    }

    IInteger slot = slot_number_annote->get_slot_number();
    if( slot > max_slot )
      max_slot = slot;
  }

  return max_slot;
}


IInteger biggest_vtbl_slot( SingleInheritanceClassType* ctype ) {
  if( ctype == NULL )
    return -1;

  IInteger max_local_slot = biggest_local_vtbl_slot( ctype );
  IInteger max_super_slot = biggest_vtbl_slot( ctype->parent_class() );

  return ( max_local_slot>max_super_slot ? max_local_slot : max_super_slot );
}

/*******************************************************************/

SingleInheritanceVtbls::SingleInheritanceVtbls( SuifEnv* env ) :
  _env( env ),
  _tb( NULL )
{
  _tb = get_type_builder( _env );
}


bool SingleInheritanceVtbls::x_overrides_y( InstanceMethodSymbol* msym1,
					    InstanceMethodSymbol* msym2 )
{
  return msym1->is_equivalent( msym2 );
  // @@@ return msym1->overrides( msym2 );
}


/*
 * The default type of a vtbl slot is void*.
 */
DataType*
SingleInheritanceVtbls::vtbl_slot_type( SingleInheritanceClassType* ctype ) {
  VoidType *void_type = _tb->get_void_type();
  PointerType* vtbl_type = _tb->get_pointer_type( void_type );

  return vtbl_type;
}


/*
 * The default type of a vtbl slot is void* [ <vtbl-size> ].
 */
DataType*
SingleInheritanceVtbls::get_vtbl_type( SingleInheritanceClassType* ctype,
				       IInteger vtbl_size )
{
  DataType* slot_type = vtbl_slot_type( ctype );
  QualifiedType* q_slot_type = _tb->get_qualified_type( slot_type );

  ArrayType* vtbl_type =
    _tb->get_array_type( q_slot_type,
			 IInteger(0),
			 (vtbl_size - 1) );
  return vtbl_type;
}


String SingleInheritanceVtbls::
vtbl_name( SingleInheritanceClassType* ctype ) {
  return ctype->get_name() + "__vtbl";
}


/*
 * Called by derive_vtbl() if an incomplete
 * SingleInheritanceClassType is encountered.
 *
 * Default behavior is to issue a warning message.
 */
void SingleInheritanceVtbls::
handle_incomplete_class( SingleInheritanceClassType* ctype ) {
#if 0 
  // @@@ our java dismantler actually expects missing 
  // info for all but the single class being compiled at a single time.
  OsuifProblems::warning( "vtbl: ClassType `%s'"
			  " has incomplete methods.",
			  ctype->get_name().c_str() );
#endif
}


/*
 * Derives a new vtbl from the parent's vtbl.
 * Methods whose is_dispatched field is false are ignored.
 */
vtbl_t* SingleInheritanceVtbls::
derive_vtbl( vtbl_t* parent_vtbl,
	     SingleInheritanceClassType* current_class )
{
  suif_assert( current_class );

  if( ! current_class->get_methods_are_complete() )
    handle_incomplete_class( current_class );
  
  if ( parent_vtbl == NULL )
    parent_vtbl = new vtbl_t();

  // copy the parent vtbl
  vtbl_t* current_vtbl = new vtbl_t( *parent_vtbl );

  list<InstanceMethodSymbol* > method_list =
    collect_stos<InstanceMethodSymbol,InstanceMethodSymbol>( current_class->get_instance_method_symbol_table() );    
  
  for ( unsigned i = 0 ; i < method_list.size() ; i++ ) {
    InstanceMethodSymbol* msym = method_list[i];

    if ( msym->get_is_dispatched() )
      insert_method( msym, current_vtbl );
  }

  // insert vtbl in map
  _vtbls.enter_value( current_class, current_vtbl );

  return current_vtbl;
}


/*
 * The MethodSymbol is placed in the vtbl. If the method overrides
 * a method of the superclass, this method replaces the existing method.
 * If the method does not override a method of the superclass, it is
 * appended at the end of the vtbl.
 *
 * A method overrides if x_overrides_y() is true.
 */

void SingleInheritanceVtbls::
insert_method( InstanceMethodSymbol* current_msym, vtbl_t* vtbl ) {
  for ( unsigned i = 0 ; i < vtbl->size() ; i++ ) {
    InstanceMethodSymbol* msym = (*vtbl)[i];
    
    if ( x_overrides_y( current_msym, msym ) ) {
      vtbl->erase( i );
      vtbl->insert( i, current_msym );
      return;
    }
  }

  // method not overwritten
  vtbl->push_back( current_msym );
}


/*
 * Build all vtables up the class hierarchy chain
 * (until an already built vtbl is encountered or
 * the root class is reached)>
 */
vtbl_t*
SingleInheritanceVtbls::build_vtbls( SingleInheritanceClassType* ctype ) {
  suif_map<SingleInheritanceClassType*, vtbl_t*>::iterator iter =
    _vtbls.find( ctype );

  if( iter != _vtbls.end() )
    return (*iter).second;
  
  if( ctype->parent_class() == NULL ) {
    return derive_vtbl( NULL, ctype );
  } else {
    vtbl_t* parent_vtbl = build_vtbls( ctype->parent_class() );
    return derive_vtbl( parent_vtbl, ctype );
  }

  return NULL; // never reached
}


void SingleInheritanceVtbls::
attach_vtbl_slot_number_annotes( SingleInheritanceClassType* ctype ) {
  vtbl_t* vtbl = build_vtbls( ctype );
  unsigned vtbl_size = vtbl->size();
    
  for ( unsigned i = 0 ; i < vtbl_size ; i++ ) {
    InstanceMethodSymbol* msym = (*vtbl)[i];
    ::attach_vtbl_slot_number_annote( _env, msym, IInteger(i) );
  }
}


void SingleInheritanceVtbls::
attach_precessors_vtbl_slot_number_annotes( SingleInheritanceClassType* ctype ) {
  if( ctype == NULL )
    return;

  attach_vtbl_slot_number_annotes( ctype );
  attach_precessors_vtbl_slot_number_annotes( ctype->parent_class() );
}


/*
 * Constructs the actual vtbl.
 * A vtbl is implemented as an array of pointers that point to
 * the addresses of the dispatched methods.
 * A method address is kept in a ExpressionValueBlock.
 *
 * The VariableDefinition that holds the array is returned. It
 * must be 
 * - inserted into a DefinitionBlock
 * - attached to a VariableSymbol.
 */
VariableDefinition*
SingleInheritanceVtbls::get_vtbl( SingleInheritanceClassType* ctype ) {
  vtbl_t* vtbl = build_vtbls( ctype );
  unsigned vtbl_size = vtbl->size();

//  DataType* slot_type = vtbl_slot_type( ctype );
//  QualifiedType* q_slot_type = _tb->get_qualified_type( slot_type );

  DataType* vtbl_type = get_vtbl_type( ctype, vtbl_size );

  //    _tb->get_array_type( q_slot_type,
  //			 IInteger(0),
  //			 IInteger(vtbl_size - 1) );
			 
  MultiValueBlock* mvb = ::create_multi_value_block( _env, vtbl_type );
    
  for ( unsigned i = 0 ; i < vtbl_size ; i++ ) {
    InstanceMethodSymbol* msym = (*vtbl)[i];

    ::attach_vtbl_slot_number_annote( _env, msym, IInteger(i) );
    
    ProcedureType* mtype = msym->get_type();
    PointerType* ptr_to_mtype = get_pointer_type( _env, mtype );

    SymbolAddressExpression* sa_expr =
      ::create_symbol_address_expression( _env, ptr_to_mtype, msym );
    msym->set_is_address_taken( true );

    ExpressionValueBlock* evb =
      ::create_expression_value_block( _env, sa_expr );

    append_to_multi_value_block( mvb, evb );
  }

  return ::create_variable_definition( _env,
				       NULL, // variable_symbol
				       0, // @@@ bit_alignment
				       mvb,
				       true // is_static
				       );
}


void SingleInheritanceVtbls::attach_vtbl( SingleInheritanceClassType* ctype,
					  VariableSymbol* vsym ) {
  VariableDefinition* vdef = get_vtbl( ctype );
  vdef->set_variable_symbol( vsym );
  vsym->set_definition( vdef );

  // Insert reference to vsym in ClassType
  ctype->set_vtbl_symbol( vsym );

  // Insert vdef in ClassType
  ctype->get_definition_block()->append_variable_definition( vdef );

  // Set type of vsym
  DataType* vtbl_type =
    vdef->get_initialization()->get_type();
  QualifiedType* q_vtbl_type =  _tb->get_qualified_type( vtbl_type );
  vsym->set_type( q_vtbl_type );
}


void SingleInheritanceVtbls::attach_vtbl( SingleInheritanceClassType* ctype ) {
  VariableSymbol* vsym =
    ::create_variable_symbol( _env,
			      NULL,  // type: set by vtbl construction
			      vtbl_name( ctype ),
			      false );

  SymbolTable* symtab = ctype->get_per_class_symbol_table();
  symtab->add_symbol( vsym );

  attach_vtbl( ctype, vsym );
}


void SingleInheritanceVtbls::print_vtbl( SingleInheritanceClassType* ctype ) {
  suif_map<SingleInheritanceClassType*, vtbl_t*>::iterator iter =
    _vtbls.find( ctype );

  if( iter != _vtbls.end() ) {
    vtbl_t* vtbl = (*iter).second;
    unsigned vtbl_size = vtbl->size();
    
    for ( unsigned i = 0 ; i < vtbl_size ; i++ ) {
      InstanceMethodSymbol* msym = (*vtbl)[i];
      ClassType* owning_ctype = msym->get_owning_class();

      cout << "  " << i << ": ";
      if( owning_ctype != NULL )
	cout << owning_ctype->get_name().c_str() << " ";
      else
	cout << "?? ";
      cout << msym->get_name().c_str() << endl;
    }
  } else {
    cout << "<Vtbl not constructed.>" << endl;
  }
}
