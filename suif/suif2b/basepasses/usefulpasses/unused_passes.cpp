#include "unused_passes.h"
#include "utils/trash_utils.h"
#include "basicnodes/basic.h"

void RemoveTrashPass::do_file_set_block(FileSetBlock *fsb) {

  take_out_trash(fsb->get_suif_env());
}
