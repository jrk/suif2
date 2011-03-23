// $Id: type_builder.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFTYPEBUILDER__TYPE_BUILDER_H
#define OSUIFTYPEBUILDER__TYPE_BUILDER_H

#include "common/lstring.h"
#include "common/suif_list.h"
#include "typebuilder/type_builder.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "cfenodes/cfe_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"


extern const LString osuif_type_builder_class_name;

extern "C" void EXPORT init_osuiftypebuilder( SuifEnv *suif );


class OsuifTypeBuilder : public RealObjectFactory {
protected:
  TypeBuilder* _tb;

public:
  virtual void init(SuifEnv* suif);

  //  OsuifTypeBuilder();
  //  virtual ~OsuifTypeBuilder() { }

  static const LString &get_class_name();
  virtual const LString &getName();

  virtual InstanceMethodType*
  get_instance_method_type( DataType * result_type,
			    list<QualifiedType *> argument_list,
			    bool has_varargs = false,
			    bool arguments_known = true,
			    int bit_alignment = 0);

  virtual StaticMethodType*
  get_static_method_type( DataType* result_type,
			  list<QualifiedType *> argument_list,
			  bool has_varargs = false,
			  bool arguments_known = true,
			  int bit_alignment = 0 );

  virtual InstanceFieldSymbol*
  add_instance_field_symbol_to_class_type( ClassType* ctype,
					   const LString& symbol_name,
					   QualifiedType* symbol_type );

  virtual int get_procedure_bit_alignment();
  virtual void append_arguments( CProcedureType* ptype,
				 list<QualifiedType *>& argument_list );

  list<QualifiedType *>& build_arg_list( DataType* arg0 );
  list<QualifiedType *>& build_arg_list( DataType* arg0,
					 DataType* arg1 );
  list<QualifiedType *>& build_arg_list( DataType* arg0,
					 DataType* arg1,
					 DataType* arg2 );
  

  // @@@ Tentative interface (up to 3 args?)
  // @@@ (if someone needs these I'm gonna write all of them...)
  InstanceMethodType* get_instance_method_type( bool has_varargs,
						bool arguments_known );
  InstanceMethodType* get_instance_method_type( bool has_varargs,
						DataType* arg0 );
  InstanceMethodType* get_instance_method_type( DataType* result_type,
						bool has_varargs,
						bool arguments_known );
  InstanceMethodType* get_instance_method_type( DataType* result_type,
						bool has_varargs,
						DataType* arg0 );

  CProcedureType *get_c_procedure_type( bool has_varargs,
					bool arguments_known);
  CProcedureType *get_c_procedure_type( bool has_varargs,
					DataType *argument0_type );
  CProcedureType *get_c_procedure_type( bool has_varargs,
					DataType *argument0_type,
					DataType *argument1_type );
  CProcedureType *get_c_procedure_type( bool has_varargs,
					DataType *argument0_type,
					DataType *argument1_type,
					DataType *argument2_type );
  CProcedureType *get_c_procedure_type( DataType *result_type,
					bool has_varargs,
					bool arguments_known );
  CProcedureType *get_c_procedure_type( DataType *result_type,
					bool has_varargs,
					DataType *argument0_type);
  CProcedureType *get_c_procedure_type( DataType *result_type,
					bool has_varargs,
					DataType *argument0_type,
					DataType *argument1_type );
  CProcedureType *get_c_procedure_type( DataType *result_type,
					bool has_varargs,
					DataType *argument0_type,
					DataType *argument1_type,
					DataType *argument2_type );
};


OsuifTypeBuilder* get_osuif_type_builder( SuifEnv* suif_env );


#endif /* OSUIFTYPEBUILDER__TYPE_BUILDER_H */
