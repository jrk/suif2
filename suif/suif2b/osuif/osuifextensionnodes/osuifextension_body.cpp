// $Id: osuifextension_body.cpp,v 1.1.1.1 2000/06/08 00:10:01 afikes Exp $

#include <iostream.h>
#include <string.h>

#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/union_meta_class.h"
#include "iokernel/clone_stream.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "iokernel/object_factory.h"
#include "iokernel/virtual_iterator.h"
#include "suifkernel/real_object_factory.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/module.h"
#include "basicnodes/basic_factory.h"


void init_osuifextension_cloning(CloneSubSystem * css) {
  // nothing to be done.
};



SingleInheritanceClassType* SingleInheritanceClassType::parent_class()
{
  SingleInheritanceClassType *current_class = this;

  if (get_parent_classe_count() == 0)
    return NULL;
  
  suif_assert( get_parent_classe_count() == 1 );

  InheritanceLink* ilink = current_class->get_parent_classe(0);
  return
    to<SingleInheritanceClassType>( ilink->get_parent_class_type() );
}


bool SingleInheritanceClassType::
is_super_class( SingleInheritanceClassType* sub_class )
{
  if( sub_class == NULL )
    return false;

  //  cout << get_name().c_str() << " "
  //       << ancestor_class->get_name().c_str() << endl;

  if( sub_class == this )
    return true;

  return is_super_class( sub_class->parent_class() );
}


bool SingleInheritanceClassType::
is_sub_class( SingleInheritanceClassType* super_class ) {
   if( super_class == this )
    return true;

   if( parent_class() == NULL )
     return false;

   return parent_class()->is_sub_class( super_class );
}


StaticMethodSymbol*
SingleInheritanceClassType::lookup_static_method( const LString &nm, 
						  StaticMethodType* method_type)
{
  SingleInheritanceClassType *current_class = this;
  
  do {
    suif_assert( false );
    int method_count = 0; // @@@ num_static_method_symbol_name( nm );
    int current_method = 0;

    while (current_method < method_count) {
      StaticMethodSymbol * method = NULL;
	// @@@
	// current_class -> lookup_static_method_symbol(nm, method_count);
      if( method == NULL ){
	current_method++;
	continue;
      }
      if( method_type->is_equivalent( to<StaticMethodType>(method->get_type ()) ) )
	return method;
      current_method++;
    }

    current_class = current_class->parent_class();

  } while (current_class);

  return NULL;
}


StaticFieldSymbol*
SingleInheritanceClassType::lookup_static_field (const LString &nm) {
  SingleInheritanceClassType *current_class = this;

  do {
    // @@@ int field_count = num_static_field_symbol_name (nm);

    StaticFieldSymbol * f =
      current_class -> lookup_static_field_symbol( nm );
    if( f ) return f;

    current_class = current_class->parent_class();
    
  } while (current_class);

  return NULL;
}


InstanceMethodSymbol* SingleInheritanceClassType::
  lookup_local_instance_method_symbol( const LString& name,
				       InstanceMethodType* method_type )
{
  InstanceMethodSymbolTable* symtab =
    get_instance_method_symbol_table();
  
  int num_hits = symtab->num_lookup_table_with_key( name );

  for( int i = 0 ; i<num_hits ; i++ ) {
    SymbolTableObject* sto =
      symtab->lookup_lookup_table( name, i );
    InstanceMethodSymbol* msym =
      to<InstanceMethodSymbol>( sto );
    InstanceMethodType* mtype =
      to<InstanceMethodType>( msym->get_type() );

    if( method_type->is_equivalent(mtype) )
      return msym;
  }

  return NULL;
}


InstanceMethodSymbol* SingleInheritanceClassType::
  lookup_transitive_instance_method_symbol( const LString& name,
					    InstanceMethodType* method_type )
{
  InstanceMethodSymbol* msym =
    lookup_local_instance_method_symbol(name, method_type);
  if( msym ) return msym;

  if( parent_class() )
    return parent_class()->lookup_local_instance_method_symbol(name, method_type);
  return NULL;
}


InstanceFieldSymbol* SingleInheritanceClassType::
  lookup_transient_instance_field( const LString& name )
{
  InstanceFieldSymbol* fsym = lookup_instance_field_symbol( name );
  if( fsym ) return fsym;

  if( parent_class() )
    return parent_class()->lookup_instance_field_symbol( name );
  return NULL;
}
