// $Id: type_utils.cpp,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "utils/expression_utils.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifutilities/problems.h"
//#include "osuiftypebuilder/type_builder.h"

#include "osuifutilities/type_utils.h"


bool is_complete_class_type( ClassType* ctype ) {
  return ctype->get_fields_are_complete()
    && ctype->get_methods_are_complete();
}


void do_class_type_layout( StructType* ctype ) {
  IInteger offset(0);
  //  IInteger maxsize(0);
  int maxalignment(0);

  GroupSymbolTable* gst = ctype->get_group_symbol_table();
  Iter <SymbolTableObject*> iter =
    gst->get_symbol_table_object_iterator();

  while ( iter.is_valid() ) {
    FieldSymbol* fsym = to<FieldSymbol>( iter.current() );
    DataType* ftype= fsym->get_type()->get_base_type();

    IInteger fsize= ftype->get_bit_size();
    int falignment= ftype->get_bit_alignment();

    if(!fsize.is_finite()) {
      OsuifProblems::warning("Field `%s' has a non-finite size.",
			     fsym->get_name().c_str());
    }

    //    if( maxsize < fsize ) maxsize = fsize;
    if( maxalignment < falignment ) maxalignment = falignment;

    if( falignment == 0 ) {
      OsuifProblems::warning("Field `%s' has zero alignment.",
			     fsym->get_name().c_str());
    }

    if( (offset % IInteger(falignment)) != 0 ) {
      offset += ( IInteger(falignment) - (offset % IInteger(falignment)) );
    }

    // @@@
    IntConstant* offset_expr =
      ::create_int_constant( ctype->get_suif_env(), offset );

    fsym->set_bit_offset( offset_expr );
    offset += fsize;

    iter.next();
  }

  ctype->set_bit_size( offset );
  ctype->set_bit_alignment( maxalignment );
}


bool is_void_result_type( CProcedureType* ptype ) {
  DataType *result_type = ptype->get_result_type();
  return
    result_type->isKindOf( VoidType::get_class_name() );
}

