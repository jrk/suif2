#include "common/system_specific.h"
#include "iokernel/cast.h"
#include "suifkernel/group_walker.h"
#include "suifkernel/suif_object.h"
#include "utils/symbol_utils.h"
#include "utils/type_utils.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "cpp_osuifnodes/cpp_osuif.h"
#include "cpp_osuifnodes/cpp_osuif_factory.h"
#include "suifkernel/utilities.h"

#include "cpp_transforms/cpp_symbol_walkers.h"

void CombinedPassForCpp::do_file_set_block( FileSetBlock* file_set_block ) 
{
    CppCollisionAvoider walker(get_suif_env(),
                               file_set_block->get_external_symbol_table(),
                               file_set_block);
    file_set_block->walk(walker);
}


Walker::ApplyStatus CppCollisionAvoider::operator () (SuifObject *x)
{
    if( ! ConstructorSymbol(x) )
    {
        return CollisionAvoider::operator()(x);
    }

    return Walker::Continue;
}


bool CppCollisionAvoider::ConstructorSymbol( SuifObject *x )
{
    if( is_kind_of<CppInstanceMethodSymbol>(x) )
    {
        CppInstanceMethodSymbol *method_symbol = to<CppInstanceMethodSymbol>(x);
        CppClassType *class_type = to<CppClassType>(
                                  method_symbol->get_symbol_table()->get_parent());
        if( method_symbol->get_name() == class_type->get_name() )
        {
            return true;
        }
    }

    return false;
}
