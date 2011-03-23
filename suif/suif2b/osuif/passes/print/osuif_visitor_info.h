// $Id: osuif_visitor_info.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFPRINT__OSUIF_VISITOR_INFO_H
#define OSUIFPRINT__OSUIF_VISITOR_INFO_H

#include <iostream.h>
#include <new.h>

#include "common/suif_list.h"
#include "common/suif_vector.h"
#include "common/suif_indexed_list.h"
#include "common/formatted.h"
#include "suifkernel/suifkernel_messages.h" // needed by iter.h
#include "suifkernel/iter.h"
#include "common/i_integer.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"
#include "suifkernel/forwarders.h"
#include "iokernel/meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/union_meta_class.h"
#include "iokernel/clone_stream.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "iokernel/object_factory.h"
#include "iokernel/virtual_iterator.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/real_object_factory.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/module.h"
#include "basicnodes/basic_factory.h"

#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifnodes/osuif_factory.h"

#include "osuifprint/printing_maps.h"

class OsuifVisitorInfo{
private:
  PrintingMaps *printingMaps; 
public:
  OsuifVisitorInfo(PrintingMaps *map);

  void get_object_address(char* result, Address obj);

  static void do_method_type(Address vis, MethodType* method_type);
  virtual void print_method_type(MethodType *method_type, ostream& output,
                                 bool blnStatic);

  static void do_static_method_type(Address vis, 
                                StaticMethodType* static_method_type);
  virtual void print_static_method_type(StaticMethodType *static_method_type,
                                        ostream& output);

  static void do_instance_method_type(Address vis, 
                                InstanceMethodType* instance_method_type);
  virtual void print_instance_method_type(
                                InstanceMethodType *instance_method_type,
                                ostream& output);

  static void do_instance_method_symbol(Address vis, 
                              InstanceMethodSymbol* method_symbol);
  virtual void print_instance_method_symbol(
                        InstanceMethodSymbol* instance_method_symbol,
                        ostream& output);

  static void do_instance_field_symbol(Address vis, 
                                       InstanceFieldSymbol* method_symbol);
  virtual void print_instance_field_symbol(
                        InstanceFieldSymbol *instance_field_symbol,
                        ostream& output);

  static void do_static_field_symbol(Address vis, 
                                     StaticFieldSymbol* static_field_symbol);
  virtual void print_static_field_symbol(StaticFieldSymbol *static_field_symbol,
                                     ostream& output);

  static void do_static_method_symbol(Address vis,
                                   StaticMethodSymbol* static_method_symbol);
  virtual void  print_static_method_symbol(
                      StaticMethodSymbol *static_method_symbol,
                      ostream& output);

  static void do_class_type(Address vis, ClassType* class_type);
  virtual void print_class_type(ClassType* class_type, ostream& output);

  static void do_inheritance_link(Address vis, 
                                  InheritanceLink* inheritance_link);
  virtual void print_inheritance_link(InheritanceLink* inheritance_link,
                                   ostream& output);

  static void do_static_method_call_statement(Address vis,
                             StaticMethodCallStatement* method_call_statement);
  virtual void print_static_method_call_statement(
          StaticMethodCallStatement *method_call_expression, ostream& output);

  static void do_instance_method_call_statement(Address vis,
                           InstanceMethodCallStatement* method_call_statement);
  virtual void print_instance_method_call_statement(
          InstanceMethodCallStatement *method_call_statement, ostream& output);

  static void do_procedure_definition(Address vis,
                               ProcedureDefinition* procedure_definition);
  virtual void print_procedure_definition(
          ProcedureDefinition *procedure_definition, ostream& output);

  static void do_statement_list(Address vis,
                          StatementList* statement_list);
  virtual void print_statement_list(StatementList *statement_list, 
                                 ostream& output);

  static void do_eval_statement(Address vis,
                                EvalStatement* eval_statement);
  virtual void print_eval_statement(EvalStatement *eval_statement, 
                                 ostream& output);

  static void do_call_statement(Address vis,
                             CallStatement* call_statement);
  virtual void print_call_statement(CallStatement *call_statement, 
                                 ostream& output);

  static void do_basic_symbol_table(Address vis, 
                                BasicSymbolTable *basic_symbol_table);
  virtual void print_basic_symbol_table(BasicSymbolTable* basicSymbolTable,
                                 ostream& output);

  static void do_variable_definition(Address vis,
                                VariableDefinition *variableDefinition);
  virtual void print_variable_definition(VariableDefinition *variableDefinition,
                                ostream& output);

  static void do_definition_block(Address vis,
                                DefinitionBlock *definitionBlock);
  virtual void print_definition_block(
                                DefinitionBlock *definitionBlock,
                                ostream& output);

  static void do_symbol_address_expression(Address vis,
                             SymbolAddressExpression* symbol_addess_expression);
  virtual void print_symbol_address_expression(
                            SymbolAddressExpression *symbol_address_expression, 
                            ostream& output);
  void initOsuifObjects();
};

#endif /* OSUIFPRINT__OSUIF_VISITOR_INFO_H */
