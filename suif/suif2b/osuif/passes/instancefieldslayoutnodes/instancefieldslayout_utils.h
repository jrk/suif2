// $Id: instancefieldslayout_utils.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef INSTANCEFIELDSLAYOUTNODES__INSTANCEFIELDSLAYOUT_UTILS_H
#define INSTANCEFIELDSLAYOUTNODES__INSTANCEFIELDSLAYOUT_UTILS_H

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "instancefieldslayoutnodes/instancefieldslayout.h"
#include "instancefieldslayoutnodes/instancefieldslayout_factory.h"
//#include "common/suif_list.h"
//#include "common/suif_map.h"


extern LString k_instancefieldslayout_offset_annote;
extern LString k_instancefieldslayout_complete_annote;

extern void
attach_instancefieldslayout_offset_annote( SuifEnv* env,
					   IntConstant* icnst,
					   InstanceFieldSymbol* fsym,
					   ClassType* owning_class= NULL );

extern IntConstant*
build_instancefieldslayout_offset_int_constant( SuifEnv* env,
						InstanceFieldSymbol* fsym,
						ClassType* owning_class= NULL );


extern void
attach_instancefieldslayout_complete_annote( SuifEnv* env,
					     ClassType* ctype );

extern bool has_instancefieldslayout_complete_annote( ClassType* ctype );


// @@@ Maybe we should change this to a cached version.
class InstanceFieldsLayout {
private:
  SuifEnv* _env;
  TypeBuilder* _tb;

protected:
  virtual InstanceFieldSymbol*
  clone_instance_field_symbol( InstanceFieldSymbol* fsym );

  virtual void handle_incomplete_class( SingleInheritanceClassType* ctype );
  virtual void collect_instance_fields( SingleInheritanceClassType* ctype,
					list<InstanceFieldSymbol* >* fsyms );
public:
  InstanceFieldsLayout( SuifEnv* env );
  virtual ~InstanceFieldsLayout() { }

  virtual void do_instance_field_layout( SingleInheritanceClassType* ctype );
  virtual void print_instance_field_layout( SingleInheritanceClassType* ctype );
};


#endif
