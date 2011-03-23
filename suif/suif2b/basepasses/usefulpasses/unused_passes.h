#ifndef UNUSED_PASSES_H
#define UNUSED_PASSES_H

#include "suifpasses/suifpasses.h"

class RemoveTrashPass : public Pass {
public:
  RemoveTrashPass(SuifEnv *env) :
    Pass(env, "recycle_trash") {}
  Module *clone() const { return ((Module*)this); }
  void do_file_set_block(FileSetBlock *fsb);
};

#endif /* UNUSED_PASSES_H */
