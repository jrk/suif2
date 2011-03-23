#ifndef CPP_TRANSFORMS__DISMANTLE_CPP_VTABLES
#define CPP_TRANSFORMS__DISMANTLE_CPP_VTABLES

#include "suifkernel/module_subsystem.h"
#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"
#include "suifnodes/suif.h"
#include "osuifnodes/osuif.h"
#include "osuifextensionnodes/osuifextension.h"
#include "common/suif_hash_map.h"
#include "cpp_osuifnodes/cpp_osuif.h"
#include "common/lstring.h"

//	Two passes - BuildCppVTablesPass creates vtables for classes
//		     Dismantle lowers code from Build into C code
//	NB: These passes are not inverses

class MethodDefList;


class DismantleCppVTablesPass : public Pass {
  TypeBuilder *tb;
  IntegerType *_pointer_sized_int;
  suif_hash_map<InstanceMethodSymbol *,ProcedureSymbol *> *_lowered_symbols;
  void add_vtable_pointer_set(CppInstanceMethodSymbol *,ParameterSymbol *this_sym);
  void dismantle_method(CppInstanceMethodSymbol *);
  void lower_instance_method_call(InstanceMethodCallExpression *,CppInstanceMethodSymbol*);
public:
  DismantleCppVTablesPass(SuifEnv *the_env,  const LString &name =
                               "dismantle_cpp_methods");
  Module *clone() const;
  void do_file_set_block( FileSetBlock* file_set_block );
};

class BuildCppVTablesPass : public Pass {
  suif_hash_map<CppClassType *,CppObject *> *_virtual_objects;
  CppClassType *_parent;
  
  TypeBuilder *tb;
  IntegerType *_pointer_sized_int;

  CppObject * build_object_lattice(CppClassType *cct);
  void build_vtable(CppClassType *cct);
  void append_constant(SuifEnv *env,MultiValueBlock *mvb,GroupType *stype,
                            DataType *size_type,int value);
  void append_symbol_address(SuifEnv *env,MultiValueBlock *mvb,GroupType *stype,Symbol *sym);
  CppObject *get_object_lattice(CppClassType *cct);
  VariableSymbol *get_rtti_symbol(CppClassType *cct);

  void build_vtable(CppObject *object,MethodDefList &methods);

  


public:
  BuildCppVTablesPass(SuifEnv *the_env,  const LString &name =
                               "build_cpp_vtables");
  Module *clone() const;
  void do_file_set_block( FileSetBlock* file_set_block );
};


#endif
