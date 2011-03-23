#ifndef TYPE_BUILDER__CPP_OSUIFTYPE_BUILDER_H
#define TYPE_BUILDER__CPP_OSUIFTYPE_BUILDER_H

#include "typebuilder/type_builder.h"
#include "osuiftypebuilder/type_builder.h"
#include "suifkernel/real_object_factory.h"
#include "basicnodes/basic_forwarders.h"
#include "basicnodes/basic.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_forwarders.h"
#include "cpp_osuifnodes/cpp_osuif_forwarders.h"
#include "cpp_osuifnodes/cpp_osuif.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "common/i_integer.h"


extern "C" void EXPORT init_cpposuiftypebuilder( SuifEnv *suif );


//
class CppOsuifTypeBuilder : public RealObjectFactory {
public:

  static const LString &get_class_name();
  virtual const LString &getName();

  virtual ReferenceType* get_reference_type(
                          IInteger size_in_bits,
                          int alignment_in_bits,
                          Type* reference_type);


   virtual ReferenceType* get_reference_type(Type* reference_type);

    virtual IntegerType* get_integer_type(IInteger size_in_bits,
                                             int alignment_in_bits,
                                             bool is_signed,
                                             LString name);

    virtual FloatingPointType* get_floating_point_type(IInteger size_in_bits,
                                                       int alignment_in_bits, LString name );

};


CppOsuifTypeBuilder* get_cpp_osuif_type_builder( SuifEnv* suif_env );

#endif
