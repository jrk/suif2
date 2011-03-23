// $Id: attribute_utils.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "iokernel/cast.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"

#include "osuifutilities/attribute_utils.h"


bool supports_attributes( Symbol* sym ) {
  if( is_kind_of<StaticFieldSymbol>( sym ) ||
      is_kind_of<InstanceFieldSymbol>( sym ) ||
      is_kind_of<StaticMethodSymbol>( sym ) ||
      is_kind_of<InstanceMethodSymbol>( sym ) ) {
    return true;
  }

  return false;
}


extern bool has_attribute( Symbol* sym, const LString& attr ) {
  if( is_kind_of<StaticFieldSymbol>( sym ) )
    return to<StaticFieldSymbol>(sym)->has_attribute_member( attr );
  if( is_kind_of<InstanceFieldSymbol>( sym ) )
    return to<InstanceFieldSymbol>(sym)->has_attribute_member( attr );
  if( is_kind_of<StaticMethodSymbol>( sym ) )
    return to<StaticMethodSymbol>(sym)->has_attribute_member( attr );
  if( is_kind_of<InstanceMethodSymbol>( sym ) )
    return to<InstanceMethodSymbol>(sym)->has_attribute_member( attr );
  
  return false;
}


extern void add_attribute( Symbol* sym, const LString& attr ) {
  if( is_kind_of<StaticFieldSymbol>( sym ) )
    to<StaticFieldSymbol>(sym)->append_attribute( attr );
  if( is_kind_of<InstanceFieldSymbol>( sym ) )
    to<InstanceFieldSymbol>(sym)->append_attribute( attr );
  if( is_kind_of<StaticMethodSymbol>( sym ) )
    to<StaticMethodSymbol>(sym)->append_attribute( attr );
  if( is_kind_of<InstanceMethodSymbol>( sym ) )
    to<InstanceMethodSymbol>(sym)->append_attribute( attr );
}
