// $Id: slot_count_annote_pass.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBLPASS__SLOT_COUNT_ANNOTE_PASS_H
#define VTBLPASS__SLOT_COUNT_ANNOTE_PASS_H

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

class VtblSlotCountAnnotePass :
  public CollectWalkerPass<VtblSlotCountAnnote>
{
private:
  bool _verbose;

public:
  VtblSlotCountAnnotePass( SuifEnv* env,
		      const LString& name= "lower_single_inheritance_vtbl_slot_count_annote" );
  virtual ~VtblSlotCountAnnotePass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( VtblSlotCountAnnote* annote );
};


#endif
