// $Id: vtbl_annote_pass.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBL__VTBL_ANNOTE_PASS_H
#define VTBL__VTBL_ANNOTE_PASS_H

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


class VtblAnnotePass : public CollectWalkerPass<VtblAnnote>
{
private:
  SingleInheritanceVtbls* _vtbl_constr;

  bool _verbose;
  
public:
  VtblAnnotePass( SuifEnv* env,
		  const LString& name = "attach_single_inheritance_vtbl_to_variable_symbol" );
  virtual ~VtblAnnotePass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( VtblAnnote* annote );
};


#endif
