// $Id: vtbl_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef VTBLNODES__VTBL_UTILS_H
#define VTBLNODES__VTBL_UTILS_H

#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"
#include "osuifextensionnodes/osuifextension.h"
#include "osuifextensionnodes/osuifextension_factory.h"
#include "vtblnodes/vtbl.h"
#include "vtblnodes/vtbl_factory.h"
#include "common/suif_list.h"
#include "common/suif_map.h"


extern LString k_vtbl_annote;
extern LString k_vtbl_slot_annote;
extern LString k_vtbl_slot_count_annote;
extern LString k_vtbl_slot_number_annote;
extern LString k_no_vtbl_annote;


void attach_vtbl_annote(  SuifEnv* env,
			  VariableSymbol* vtbl_sym,
			  ClassType* owning_class );

VariableSymbol* build_vtbl_variable_symbol( SuifEnv* env,
					    ClassType* owning_class,
					    const LString & name= emptyLString );


void attach_vtbl_slot_annote( SuifEnv* env,
			      IntConstant* icnst,
			      InstanceMethodSymbol* msym );

IntConstant* build_vtbl_slot_int_constant( SuifEnv* env,
					   InstanceMethodSymbol* msym );


extern void attach_vtbl_slot_count_annote( SuifEnv* env,
					   IntConstant* icnst,
					   ClassType* owning_class );

IntConstant* build_vtbl_slot_count_int_constant( SuifEnv* env,
						 ClassType* owning_class );


void attach_vtbl_slot_number_annote( SuifEnv* env,
				     InstanceMethodSymbol* msym,
				     IInteger slot_number );


extern void attach_no_vtbl_annote( SuifEnv* env,
				   ClassType* ctype );

extern bool has_no_vtbl_annote( ClassType* ctype );



IInteger biggest_local_vtbl_slot( ClassType* ctype );
IInteger biggest_vtbl_slot( SingleInheritanceClassType* ctype );



/// Type to logically represent a vtbl
typedef list<InstanceMethodSymbol* > vtbl_t;

class SingleInheritanceVtbls {
protected:
  SuifEnv* _env;
  TypeBuilder* _tb;

  suif_map<SingleInheritanceClassType*, vtbl_t* > _vtbls;

  virtual vtbl_t* derive_vtbl( vtbl_t* parent_vtbl,
			       SingleInheritanceClassType* current_class );
  virtual void insert_method( InstanceMethodSymbol* current_msym, vtbl_t* vtbl );

  virtual void handle_incomplete_class( SingleInheritanceClassType* ctype );  
  virtual vtbl_t* build_vtbls( SingleInheritanceClassType* ctype );
public:
  SingleInheritanceVtbls( SuifEnv* env );
  virtual ~SingleInheritanceVtbls() { }

  virtual bool x_overrides_y( InstanceMethodSymbol* msym1,
			      InstanceMethodSymbol* msym2 );

  /// Returns type to represent a vtbl slot entry
  virtual DataType* vtbl_slot_type( SingleInheritanceClassType* ctype );

  /// Returns type to represent a vtbl
  virtual DataType* get_vtbl_type( SingleInheritanceClassType* ctype,
				   IInteger vtbl_size = IInteger() );


  virtual String vtbl_name( SingleInheritanceClassType* ctype );
  
  virtual void attach_vtbl_slot_number_annotes( SingleInheritanceClassType* ctype );
  virtual void
  attach_precessors_vtbl_slot_number_annotes( SingleInheritanceClassType* ctype );

  virtual VariableDefinition* get_vtbl( SingleInheritanceClassType* ctype );
  virtual void attach_vtbl( SingleInheritanceClassType* ctype,
			    VariableSymbol* vsym );
  virtual void attach_vtbl( SingleInheritanceClassType* ctype );
  
  /// Print vtbl layout to stdout
  virtual void print_vtbl( SingleInheritanceClassType* ctype );
};


#endif
