// $Id: search_utils.cpp,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"

#include "osuifutilities/search_utils.h"


FileBlock* get_associated_file_block( SuifObject* obj ) {
  FileBlock* fb = find_ancestor<FileBlock>( obj );
  if ( fb == NULL ) {
    SuifEnv* env = obj->get_suif_env();
    FileSetBlock* fsb = env->get_file_set_block();
    if( fsb->get_file_block_count() == 0 ) {
      fb = ::create_file_block( env, "osuif" );
      fsb->append_file_block( fb );
    } else {
      fb = fsb->get_file_block( 0 );
    }
  }
  return fb;
}


ClassType* get_owning_class( SuifObject* obj ) {
  if( obj == NULL) return NULL;

  if( is_kind_of<ClassType>(obj) )
    return to<ClassType>( obj );

  // Check if owner can be determined directly
  if( is_kind_of<StaticFieldSymbol>( obj ) )
    return to<StaticFieldSymbol>(obj)->get_owning_class();
  if( is_kind_of<InstanceFieldSymbol>( obj ) )
    return to<InstanceFieldSymbol>(obj)->get_owning_class();
  if( is_kind_of<StaticMethodSymbol>( obj ) )
    return to<StaticMethodSymbol>(obj)->get_owning_class();
  if( is_kind_of<InstanceMethodSymbol>( obj ) )
    return to<InstanceMethodSymbol>(obj)->get_owning_class();

  // Check if owner can be infered by following reference links.
  // @@@ Probably stuff missing...
  if( is_kind_of<ProcedureDefinition>(obj) ) {
    return 
      get_owning_class( to<ProcedureDefinition>(obj)->get_procedure_symbol() );
  }
  if( is_kind_of<VariableDefinition>(obj) ) {
    return 
      get_owning_class( to<VariableDefinition>(obj)->get_variable_symbol() );
  }

  return get_owning_class( obj->get_parent() );
}
