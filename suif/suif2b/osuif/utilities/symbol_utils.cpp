// $Id: symbol_utils.cpp,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#include "iokernel/cast.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "osuifutilities/search_utils.h"

#include "osuifutilities/symbol_utils.h"


void symbol_has_address_taken( Symbol* sym ) {
  sym->set_is_address_taken( true );
}


void update_is_address_taken( Symbol* sym, bool is_address_taken ) {
  if( ! sym->get_is_address_taken() )
    sym->set_is_address_taken( is_address_taken );
}


ClassType* get_owning_class( Symbol* sym ) {
  ClassType* ctype = NULL;

  if( is_kind_of<StaticFieldSymbol>(sym) )
    ctype = to<StaticFieldSymbol>(sym)->get_owning_class();
  if( is_kind_of<InstanceFieldSymbol>(sym) )
    ctype = to<InstanceFieldSymbol>(sym)->get_owning_class();
  if( is_kind_of<StaticMethodSymbol>(sym) )
    ctype = to<StaticMethodSymbol>(sym)->get_owning_class();
  if( is_kind_of<InstanceMethodSymbol>(sym) )
    ctype = to<InstanceMethodSymbol>(sym)->get_owning_class();

  if( ctype == NULL )
    ctype = ::find_ancestor<ClassType>( sym );

  return ctype;
}
