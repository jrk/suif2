#ifndef EXPRESSION_STRING_H
#define EXPRESSION_STRING_H

#include "suifkernel/suifkernel_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
class SourceOp;
class VisitorMap;

class BuildCStringState;

class BuildCStringState {
  VisitorMap *_expression_map;
  VisitorMap *_statement_map;

  String _str;  // this is the return value from a handle.
public:
  
  BuildCStringState(SuifEnv *s);

  String build_expression_string(Expression *ex);

  String build_statement_string(Statement *st);

  //  String build_source_op_string(const SourceOp &op);
  //  String build_source_op_string(Expression *op);

  String build_variable_symbol_string(VariableSymbol *var);

  String build_procedure_symbol_string(ProcedureSymbol *ps);
  

  // These should probably be protected or private, butthen
  // I'd need to have a bunch of friend classes.
  void set_string(const String &str);
  String get_string() const;

};


#endif
