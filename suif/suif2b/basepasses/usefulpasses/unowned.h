#ifndef UNOWNED_H
#define UNOWNED_H


#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"

class UnownedPass : public Pass {
public:
	UnownedPass(SuifEnv *pEnv, const LString &name =
		    "find_unowned");
	Module* clone() const { return (Module*)this; }
	void do_file_set_block(FileSetBlock *pFSB);
	bool is_walkable(Address address, bool is_owned, MetaClass *_meta);
};

#endif

