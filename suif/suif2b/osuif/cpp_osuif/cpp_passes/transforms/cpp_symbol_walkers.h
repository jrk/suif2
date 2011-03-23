#ifndef CPP_PASSES__CPP_SYMBOL_WALKERS_H
#define CPP_PASSES__CPP_SYMBOL_WALKERS_H

#include <common/suif_copyright.h>
#include "suifnodes/suif.h"

#include "transforms/symbol_walkers.h"


class CombinedPassForCpp : public Pass
{
  public:
    Module* clone() const {return (Module*)this;}

    void do_file_set_block( FileSetBlock* file_set_block );

    CombinedPassForCpp(SuifEnv *env) : Pass(env,"rename_colliding_symbols_for_cpp") {}
};

class CppCollisionAvoider : public CollisionAvoider
{
  public:
    CppCollisionAvoider(SuifEnv *env,
                         BasicSymbolTable *the_external_symbol_table,
                         FileSetBlock* file_set_block)
                : CollisionAvoider(env,
                                   the_external_symbol_table,
                                   NULL,
                                   (file_set_block->get_file_block(0))->get_source_file_name(),
                                   false) {}

     ApplyStatus operator () (SuifObject *x);

  protected:
    bool ConstructorSymbol( SuifObject * );

};

#endif
