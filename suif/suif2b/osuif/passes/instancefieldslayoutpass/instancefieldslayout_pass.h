// $Id: instancefieldslayout_pass.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef INSTANCEFIELDSLAYOUTPASS__INSTANCEFIELDSLAYOUTPASS_H
#define INSTANCEFIELDSLAYOUTPASS__INSTANCEFIELDSLAYOUTPASS_H

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
#include "instancefieldslayoutnodes/instancefieldslayout_utils.h"

#include "instancefieldslayoutpass/instancefieldslayout.h"


class InstanceFieldsLayoutPass : public CollectWalkerPass<ClassType>
{
private:
  InstanceFieldsLayout* _layout;

  String _name_mangling;
  bool _not_use_annote_name;
  bool _verbose;
  
public:
  InstanceFieldsLayoutPass( SuifEnv* env,
    const LString& name= "layout_single_inheritance_instance_fields" );
  virtual ~InstanceFieldsLayoutPass()  { }

  virtual void initialize_flags();
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  virtual const LString mangled_name( InstanceFieldSymbol* fsym );
  virtual void mangle_fields( ClassType* ctype );

  virtual void process_suif_object( ClassType* ctype );
};


#endif
