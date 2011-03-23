#include "expression_string.h"
#include "suifkernel/visitor_map.h"
#include "basicnodes/basic_constants.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"

class BuildCStringState;
#define HANDLE_DISPATCH(lower, name) \
static void handle_static_ ## lower(BuildCStringState *map, name * ## lower);
#include "expr.defs"
#include "stmt.defs"
#undef HANDLE_DISPATCH


void BuildCStringState::set_string(const String &str) { _str = str; }
String BuildCStringState::get_string() const { return(_str); }

BuildCStringState::BuildCStringState(SuifEnv *s) {
  _expression_map = new VisitorMap(s);
  _statement_map = new VisitorMap(s);

#define HANDLE_DISPATCH(lower, name) \
  _expression_map->register_visit_method( this, \
	   (VisitMethod)handle_static_ ## lower ##, \
	   name ## ::get_class_name());
#include "expr.defs"
#undef HANDLE_DISPATCH
#define HANDLE_DISPATCH(lower, name) \
  _statement_map->register_visit_method( this, \
	   (VisitMethod)handle_static_ ## lower ##, \
	   name ## ::get_class_name());
#include "stmt.defs"
#undef HANDLE_DISPATCH
					    
  }

String BuildCStringState::build_variable_symbol_string(VariableSymbol *var) {
  // I'd like to figure out what procedure and BLOCK scope this
  // is in...
  return(String(var->get_name()));
}

String BuildCStringState::build_procedure_symbol_string(ProcedureSymbol *ps) {
  return(String("P:") + ps->get_name());
}
  
String BuildCStringState::build_expression_string(Expression *ex) {
  // need a visitor for this.
  if (ex == NULL) return("(nil)");
  _expression_map->apply(ex);
  return(get_string());
}

String BuildCStringState::build_statement_string(Statement *stmt) {
  // need a visitor for this.
  _statement_map->apply(stmt);
  return(get_string());
}


//String BuildCStringState::build_source_op_string(const SourceOp &op) {
//String BuildCStringState::build_source_op_string(Expression *op) {
//  return(build_expression_string(op));
//}

//
//  Here are the visit methods
//  for the c-like printing
//
// All of these handle_
// will set the string in the state before returning.
static 
void handle_static_expression(BuildCStringState *state,
			      Expression *expr)
{
  // Use the iterator over source ops and
  // get the classname
  String opname = expr->get_class_name();
  String return_str = String("?") + opname + "(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_source_op_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_expression_string(opn);
    return_str += op;
  }
  return_str += ")";
  state->set_string(return_str);
}

static 
void handle_static_unary_expression(BuildCStringState *state,
					       UnaryExpression *expr)
{
  LString opc = expr->get_opcode();
  String src_string = state->build_expression_string(expr->get_source());
  if (opc == k_convert) { state->set_string(src_string);  return; }
  state->set_string(String(opc) + "(" + src_string + ")");
  return;
}

static 
void handle_static_binary_expression(BuildCStringState *state,
				     BinaryExpression *expr)
{
  LString opc = expr->get_opcode();
  String src_string1 = state->build_expression_string(expr->get_source1());
  String src_string2 = state->build_expression_string(expr->get_source2());
  // Should we special case: add, mod, etc?
  state->set_string(String(opc) + "(" + src_string1 + "," + src_string2 + ")");
  return;
}
static 
void handle_static_array_reference_expression(BuildCStringState *state,
					      ArrayReferenceExpression *expr)
{
  String base = state->build_expression_string(expr->get_base_array_address());
  String idx = state->build_expression_string(expr->get_index());
  
  state->set_string(String("&(") + base + "[" + idx + "])");
  return;
}
static 
void handle_static_load_expression(BuildCStringState *state,
				   LoadExpression *expr)
{
  String source = state->build_expression_string(expr->get_source_address());
  // If the previous began with a "&", return the rest of it.
  if(source.length() >= 1 &&
     source.c_str()[0] == '&') {
    // This is clearly cheating with the string class.
    state->set_string(((char *)source.c_str())+1);
  } else {
    state->set_string(String("*") + source);
  }
}
static 
void handle_static_load_variable_expression(BuildCStringState *state,
					    LoadVariableExpression *expr)
{
  String source = state->build_variable_symbol_string(expr->get_source());
  state->set_string(source);
}
static 
void handle_static_call_statement(BuildCStringState *state,
				  CallStatement *expr)
{
  String addr = state->build_expression_string(expr->get_callee_address());
  String return_str;
  if (expr->get_destination() != NULL) {
    return_str += state->build_variable_symbol_string(expr->get_destination());
    return_str += " = ";
  }
    
  return_str += String("(") + addr + ")(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_argument_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_expression_string(opn);
    return_str += op;
  }
  return_str += ")";
  state->set_string(return_str);
}

/*
static 
void handle_static_call_expression(BuildCStringState *state,
				   CallExpression *expr)
{
  String addr = state->build_expression_string(expr->get_callee_address());
  String return_str = String("(") + addr + ")(";
  bool needs_comma = false;
  for (Iter<Expression *> iter = expr->get_argument_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_expression_string(opn);
    return_str += op;
  }
  return_str += ")";
  state->set_string(return_str);
}
*/

static 
void handle_static_symbol_address_expression(BuildCStringState *state,
					   SymbolAddressExpression *expr)
{
  Symbol *sym = expr->get_addressed_symbol();
  if (is_kind_of<VariableSymbol>(sym)) {
    state->set_string(String("&(") + state->build_variable_symbol_string(to<VariableSymbol>(sym)) + ")");
    return;
  }
  if (is_kind_of<ProcedureSymbol>(sym)) {
    state->set_string(String("&") + state->build_procedure_symbol_string(to<ProcedureSymbol>(sym))+ ")");
    return;
  }
  state->set_string(String("&") + sym->get_name());
}
static 
void handle_static_va_arg_expression(BuildCStringState *state,
			      VaArgExpression *expr)
{
  String op = state->build_expression_string(expr->get_ap_address());
  state->set_string(String("va_arg(") + op + ")");
}

static 
void handle_static_int_constant(BuildCStringState *state,
				IntConstant *expr)
{
  IInteger v = expr->get_value();
  String str;
  v.write(str,10);
  state->set_string(str);
}
static 
void handle_static_float_constant(BuildCStringState *state,
				  FloatConstant *expr) {
  state->set_string(expr->get_value().c_str());
}

/* Still need to add some support for finding field annotations 
 * (Makes the output MUCH more readable.
 */

static 
void handle_static_statement(BuildCStringState *state,
			     Statement *stmt)
{
  // Use the iterator over 
  //    destination_vars
  //    source ops
  //    source variables
  // Use the iterator over source ops and
  // get the classname

  String return_str = "(";
  bool needs_comma = false;
  {for (Iter<VariableSymbol *> iter = stmt->get_destination_var_iterator();
       iter.is_valid();
       iter.next()) {
    VariableSymbol *var = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_variable_symbol_string(var);
    return_str += op;
  }}
  return_str += ") = ";

  String opname = stmt->getClassName();
  return_str += String("?") + opname + "(";
  needs_comma = false;
  {for (Iter<Expression *> iter = stmt->get_source_op_iterator();
       iter.is_valid();
       iter.next()) {
    Expression *opn = iter.current();
    if (needs_comma) {
      return_str += ",";
    } else {
      needs_comma = true;
    }
    String op = state->build_expression_string(opn);
    return_str += op;
  }}
  return_str += ")";
  state->set_string(return_str);
}

static 
void handle_static_return_statement(BuildCStringState *state,
				    ReturnStatement *stmt)
{
  String return_str = "Return";
  if (stmt->get_return_value() != NULL) {
    String op = state->build_expression_string(stmt->get_return_value());
    return_str = return_str + "(" + op + ")";
  }
  state->set_string(return_str);
}

static 
void handle_static_store_variable_statement(BuildCStringState *state,
					    StoreVariableStatement *stmt)
{
  String return_str = 
    state->build_variable_symbol_string(stmt->get_destination()) + " = ";
  String op = state->build_expression_string(stmt->get_value());
  return_str += op;
  state->set_string(return_str);
}

static 
void handle_static_store_statement(BuildCStringState *state,
				   StoreStatement *stmt)
{
  String return_str = "*(";
  return_str += 
    state->build_expression_string(stmt->get_destination_address());
  
  return_str += ") = ";
  return_str += 
    state->build_expression_string(stmt->get_value());
  state->set_string(return_str);
}

static 
void handle_static_mark_statement(BuildCStringState *state,
				  MarkStatement *stmt)
{
  String return_str;
  BrickAnnote *br = to<BrickAnnote>(stmt->peek_annote("line"));
  return_str = "Mark";
  if (br != 0) {
    String file = to<StringBrick>(br->get_brick(1))->get_value();
    IInteger line = to<IntegerBrick>(br->get_brick(0))->get_value();
    return_str += " ";
    return_str += file + ":" + line.to_String();
  }
  state->set_string(return_str);
}

static 
void handle_static_if_statement(BuildCStringState *state,
				IfStatement *stmt)
{
  String return_str = "If (";
  return_str += 
    state->build_expression_string(stmt->get_condition());
  return_str += ") {...}";
  state->set_string(return_str);
}

