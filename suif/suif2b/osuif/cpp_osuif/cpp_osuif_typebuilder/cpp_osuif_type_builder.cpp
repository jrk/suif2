/*
 *  SUIF type factory
 *
 * Tries to ensure there is only one type of a given kind in a symbol
 * table tree.
 *
 * Inserts stuff in the right symbol table
 */

#include "suifkernel/suif_object.h"
#include "suifkernel/iter.h"
#include "suifkernel/module.h"
#include "suifkernel/module_subsystem.h"
#include "typebuilder/type_builder.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"
#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifnodes/osuif_factory.h"
#include "osuiftypebuilder/type_builder.h"
#include "cpp_osuifnodes/cpp_osuif_forwarders.h"
#include "cpp_osuifnodes/cpp_osuif.h"
#include "cpp_osuifnodes/cpp_osuif_factory.h"
#include "cpp_osuif_type_builder.h"


//#include "suifkernel/error_macros.h"
#include "iokernel/cast.h"

#include "common/suif_list.h"


class CppOsuifTypeBuilderModule : public Module {
public:
  CppOsuifTypeBuilderModule(SuifEnv *suif) : Module(suif,"CppOsuifTypeBuilder") {}
  virtual void initialize() {
    get_suif_env()->add_object_factory( new CppOsuifTypeBuilder() );
  }
  virtual Module* clone() const { return (Module *)this; }
};

extern "C" void EXPORT init_typebuilder( SuifEnv *suif );
extern "C" void EXPORT init_cpposuiftypebuilder( SuifEnv *suif )
{
  init_osuiftypebuilder(suif);
  ModuleSubSystem *mSubSystem = suif->get_module_subsystem();
  mSubSystem -> register_module(new CppOsuifTypeBuilderModule( suif ));
}

static const char *cpp_osuif_type_builder_class_name = "CppOsuifTypeBuilder";

const LString &CppOsuifTypeBuilder::get_class_name() {
  static LString name = cpp_osuif_type_builder_class_name;

  return (name);
}

const LString &CppOsuifTypeBuilder::getName() {
  static LString name = cpp_osuif_type_builder_class_name;

  return (name);
}

CppOsuifTypeBuilder* get_cpp_osuif_type_builder(SuifEnv *_suif)
    {
    return ((CppOsuifTypeBuilder *)
	  (_suif->get_object_factory(CppOsuifTypeBuilder::get_class_name())));
    }

ReferenceType* CppOsuifTypeBuilder::get_reference_type( IInteger size_in_bits, int alignment_in_bits, Type* reference_type) {
    if (is_kind_of<DataType>(reference_type)) {
      // Qualify it before getting the pointer.
      reference_type = get_qualified_type(_suif_env,to<DataType>(reference_type),emptyLString);
    }

    ReferenceType *element;
    SymbolTable *symbolTable = to<SymbolTable>(reference_type->get_parent());
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
    	SymbolTableObject *obj = iter.current();

    	if (is_a<ReferenceType>(obj)) {
	    element = to<ReferenceType>(obj);

    	    if (element->get_reference_type() == reference_type
             && element->get_bit_size() == size_in_bits
             && element->get_bit_alignment() == alignment_in_bits)
      		return (element);
  	    }
	iter.next();
	}
    element = create_reference_type(get_suif_environment(), size_in_bits, alignment_in_bits,
                                             reference_type);

    symbolTable->append_symbol_table_object(element);

    return (element);
    }


ReferenceType* CppOsuifTypeBuilder::get_reference_type(Type* reference_type)
    {
    return get_reference_type(sizeof(void *)*8,sizeof(void *)*8,reference_type);
    }

IntegerType* CppOsuifTypeBuilder::get_integer_type(IInteger size_in_bits,
                                             int alignment_in_bits,
                                             bool is_signed,
                                             LString name)
    {
    FileSetBlock *block = get_suif_environment()->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<IntegerType>(obj)) {
            IntegerType *element = to<IntegerType>(obj);
            if ( element->get_bit_size() == size_in_bits
                && element->get_bit_alignment() == alignment_in_bits 
                && element->get_is_signed() == is_signed
                && element->get_name() == name)
                return element;
            }
        iter.next();
        }
    IntegerType * element = create_integer_type(get_suif_environment(), size_in_bits,
                                                   alignment_in_bits, is_signed, name);
    symbolTable->append_symbol_table_object(element);
    return (element);

    }

FloatingPointType* CppOsuifTypeBuilder::get_floating_point_type( IInteger size_in_bits, int alignment_in_bits, LString name ) {
    FileSetBlock *block = get_suif_environment()->get_file_set_block();
    suif_assert_message(block, ("fileset block not attached to suifenv"));
    SymbolTable *symbolTable = block->get_external_symbol_table();
    suif_assert_message(symbolTable, ("external_symbol table  not attached"));
    Iter<SymbolTableObject *> iter = symbolTable->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        SymbolTableObject *obj = iter.current();
        if (is_a<FloatingPointType>(obj)) {
            FloatingPointType *element = to<FloatingPointType>(obj);
            if (( element->get_bit_size() == size_in_bits)
                && (element->get_bit_alignment() == alignment_in_bits)
                && (element->get_name() == name))
                return element;
            }
        iter.next();
        }
    FloatingPointType * element = create_floating_point_type(get_suif_environment(), size_in_bits, alignment_in_bits,name);
    symbolTable->append_symbol_table_object(element);
    return (element);
    }

