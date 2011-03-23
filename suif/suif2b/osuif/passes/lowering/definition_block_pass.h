// $Id: definition_block_pass.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef OSUIF_LOWERING__DEFINITION_BLOCK_PASS_H
#define OSUIF_LOWERING__DEFINITION_BLOCK_PASS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/suif_object.h"
#include "suifpasses/suifpasses.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "cfenodes/cfe.h"
#include "osuifutilities/pass_utils.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"
#include "vtblnodes/vtbl_utils.h"


class DefinitionBlockPass : public CollectWalkerPass<ClassType>
{
private:
  bool _verbose;
  
public:
  DefinitionBlockPass( SuifEnv* env,
	    const LString& name= "lower_definition_block" );
  virtual ~DefinitionBlockPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( ClassType* ctype );
};


#endif
