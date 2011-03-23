// $Id: offset_annote_pass.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef INSTANCEFIELDSLAYOUTPASS__OFFSET_ANNOTE_PASS_H
#define INSTANCEFIELDSLAYOUTPASS__OFFSET_ANNOTE_PASS_H

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

class InstanceFieldsLayoutOffsetAnnotePass :
  public CollectWalkerPass<InstanceFieldOffsetAnnote>
{
private:
  bool _verbose;

public:
  InstanceFieldsLayoutOffsetAnnotePass( SuifEnv* env,
    const LString& name= "lower_single_inheritance_instance_field_offset" );
  virtual ~InstanceFieldsLayoutOffsetAnnotePass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual void process_suif_object( InstanceFieldOffsetAnnote* annote );
};


#endif
