#include "strip_annotes_pass.h"
#include "utils/annote_utils.h"
#include "basicnodes/basic.h"

StripAnnotesPass::StripAnnotesPass(SuifEnv *suif_env, const LString &name) :
  Pass(suif_env, name)
{
}


void
StripAnnotesPass::initialize(void)
{
  Pass::initialize();
  _command_line->set_description("Pass to put named annotes in the trash.\n");
  _command_line->add(new OptionLoop(new OptionMultiString( String("annote name"), &annote_name_args),
				    false));
}


void
StripAnnotesPass::do_file_set_block(FileSetBlock *fsb)
{
  list<LString> names;

  for (suif_vector< String >::iterator it = annote_name_args.begin();
       it != annote_name_args.end();
       ++it) {
    names.push_back(LString( *it ));
  }

  trash_named_annotes(_suif_env, names, fsb);
}

		    
				    
