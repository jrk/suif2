#include "common/system_specific.h"
#include <common/suif_copyright.h>

#include <iokernel/cast.h>
#include <common/i_integer.h>
#include <basicnodes/basic_factory.h>
#include <suifnodes/suif.h>
#include <suifnodes/suif_factory.h>
#include <basicnodes/basic.h>
#include <basicnodes/basic_constants.h>
#include <suifkernel/suifkernel_messages.h>
#include "transforms/procedure_walker_utilities.h"
#include <utils/expression_utils.h>
#include "constant_folder.h"


/**************************** Declarations ************************************/
class constant_folder_walker: public GroupWalker {
public:
  constant_folder_walker(SuifEnv *the_env, ProcedureDefinition *proc_def, bool fold_floats = true);
private:
  bool fold_floats;
};

class binary_expression_walker: public SelectiveWalker {
public:
  binary_expression_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, BinaryExpression::get_class_name()) {}

  Walker::ApplyStatus operator () (SuifObject *x);
private:
	bool fold_floats;
};

class if_walker: public ProcedureWalker {
public:
  if_walker::if_walker(SuifEnv *the_env, ProcedureDefinition *def)
    : ProcedureWalker(the_env, def, IfStatement::get_class_name()) {}

  Walker::ApplyStatus operator () (SuifObject *x);
};

/**************************** Implementations ************************************/
ConstantFolderPass::ConstantFolderPass(SuifEnv *pEnv) : PipelinablePass(pEnv, "ConstantFolderPass"){}

void ConstantFolderPass::do_procedure_definition(ProcedureDefinition *proc_def){
    if(proc_def){
      constant_folder_walker walker(get_suif_env(), proc_def);
      proc_def->walk(walker);
    }
}

constant_folder_walker::constant_folder_walker(SuifEnv *the_env, ProcedureDefinition *proc_def, bool fold_floats) : GroupWalker(the_env){
    this->fold_floats = fold_floats;
    binary_expression_walker the_binary_expression_walker(the_env);
    if_walker the_if_walker(the_env, proc_def);

    append_walker(the_binary_expression_walker);
    append_walker(the_if_walker);
}

String float_to_string(double the_float){
    char bfr[128];
    char fmt[16];
    sprintf(fmt, "%%.%de", (int)(sizeof(float) * 2.4));
    sprintf(bfr, fmt, the_float);
    return(bfr);
}

float to_a_number(FloatConstant *float_const){
  String str = float_const->get_value();
  float the_float;

  sscanf(str.c_str(), "%f", &the_float);
  return the_float;
}

bool is_a_number(FloatConstant *float_const){
  String str = float_const->get_value();

  return float_to_string(to_a_number(float_const)) == str;
}

Walker::ApplyStatus binary_expression_walker::operator () (SuifObject *x){
    SuifEnv *the_env = get_env();
    BinaryExpression *the_expr = to<BinaryExpression>(x);
    Expression *result = NULL;

    LString opcode   = the_expr->get_opcode();
    Expression *src1 = the_expr->get_source1();
    Expression *src2 = the_expr->get_source2();

    /* Make src1 "more constant" than src2 */
    if(is_kind_of<Constant>(src2) && opcode!=k_subtract && opcode!=k_divide && opcode!=k_remainder){
      // swap
      Expression *tmp = src1;
      src1 = src2;
      src2 = tmp;
    }

    if(opcode==k_add){
      if(is_a<IntConstant>(src1)){
        IInteger int1 = (to<IntConstant>(src1))->get_value();
        if(is_a<IntConstant>(src2)){
          /* Both operands are ints */
          IInteger int2 = (to<IntConstant>(src2))->get_value();
          result = create_int_constant(the_env, the_expr->get_result_type(), int1.add(int2));
        }else
          if(int1.is_c_int() && int1.c_int()==0){
            /* 0+x */
            result = src2;
          }
      }else
      if(is_a<FloatConstant>(src1) && fold_floats && is_a_number(to<FloatConstant>(src1)) &&
        is_a<FloatConstant>(src2) && is_a_number(to<FloatConstant>(src2))){
          /* Both operands are floats */
          String float2 = (to<FloatConstant>(src1))->get_value();
          result = 
			  create_float_constant(the_env, the_expr->get_result_type(), 
				float_to_string(
				((float)to_a_number(to<FloatConstant>(src1)))+
				(float)to_a_number(to<FloatConstant>(src2))) );
      }
    }else
    if(opcode==k_subtract){
      if(is_a<IntConstant>(src1)){
        IInteger int1 = (to<IntConstant>(src1))->get_value();
        if(is_a<IntConstant>(src2)){
          /* Both operands are ints */
          IInteger int2 = (to<IntConstant>(src2))->get_value();
          result = create_int_constant(the_env, the_expr->get_result_type(), int1.subtract(int2));
        }else
          if(int1.is_c_int() && int1.c_int()==0){
            /* 0-x */
            result = create_unary_expression(the_env, the_expr->get_result_type(), k_negate, src2);
          }
      }else
      if(is_a<FloatConstant>(src1) && fold_floats && is_a_number(to<FloatConstant>(src1)) &&
        is_a<FloatConstant>(src2) && is_a_number(to<FloatConstant>(src2))){
          /* Both operands are floats */
          String float2 = (to<FloatConstant>(src1))->get_value();
          result = create_float_constant(the_env, 
			  the_expr->get_result_type(), 
			  float_to_string(to_a_number(to<FloatConstant>(src1))-
			  to_a_number(to<FloatConstant>(src2))));
      }
    }else
    if(opcode==k_multiply){
      if(is_a<IntConstant>(src1)){
        IInteger int1 = (to<IntConstant>(src1))->get_value();
        if(is_a<IntConstant>(src2)){
          /* Both operands are ints */
          IInteger int2 = (to<IntConstant>(src2))->get_value();
          result = create_int_constant(the_env, the_expr->get_result_type(), int1.multiply(int2));
        }else
          if(int1.is_c_int() && int1.c_int()==1){
            /* 1*x */
            result = src2;
          }else
          if(int1.is_c_int() && int1.c_int()==0){
            /* 0*x */
            result = create_int_constant(the_env, the_expr->get_result_type(), IInteger(0));
          }
      }else
      if(is_a<FloatConstant>(src1) && fold_floats && is_a_number(to<FloatConstant>(src1)) &&
        is_a<FloatConstant>(src2) && is_a_number(to<FloatConstant>(src2))){
          /* Both operands are floats */
          String float2 = (to<FloatConstant>(src1))->get_value();
          result = create_float_constant(the_env, 
			  the_expr->get_result_type(), 
			  float_to_string(to_a_number(to<FloatConstant>(src1))*
			  to_a_number(to<FloatConstant>(src2))));
      }
    }else
    if(opcode==k_divide){
      if(is_a<IntConstant>(src1)){
        IInteger int1 = (to<IntConstant>(src1))->get_value();
        if(is_a<IntConstant>(src2)){
          /* Both operands are ints */
          IInteger int2 = (to<IntConstant>(src2))->get_value();
          result = create_int_constant(the_env, the_expr->get_result_type(), int1.div(int2));
        }else
          if(int1.is_c_int() && int1.c_int()==0){
            /* 0/x = 0 */
            result = create_int_constant(the_env, the_expr->get_result_type(), IInteger(0));
          }else
            if(is_a<IntConstant>(src2)){
              IInteger int2 = (to<IntConstant>(src2))->get_value();
              if(int2.is_c_int() && int2.c_int()==1){
                /* x/1 = x */
                result = src1;
              }
            }
      }else
      if(is_a<FloatConstant>(src1) && fold_floats && is_a_number(to<FloatConstant>(src1)) &&
        is_a<FloatConstant>(src2) && 
			is_a_number(to<FloatConstant>(src2)) && 
			to_a_number(to<FloatConstant>(src2))!=0.0){
          /* Both operands are floats, src2!=0 */
          String float2 = (to<FloatConstant>(src1))->get_value();
          result = create_float_constant(the_env, 
			  the_expr->get_result_type(), 
			  float_to_string(to_a_number(to<FloatConstant>(src1))/
			  to_a_number(to<FloatConstant>(src2))));
      }
    }else
    if(opcode==k_remainder){
      if(is_a<IntConstant>(src1)){
        IInteger int1 = (to<IntConstant>(src1))->get_value();
        if(is_a<IntConstant>(src2)){
          /* Both operands are ints */
          IInteger int2 = (to<IntConstant>(src2))->get_value();
          result = create_int_constant(the_env, the_expr->get_result_type(), int1.mod(int2));
        }else
          if(int1.is_c_int() && int1.c_int()==0){
            /* 0%x = 0 */
            result = create_int_constant(the_env, the_expr->get_result_type(), IInteger(0));
          }
      }
    }

    /* Perform the actual replacement */
    if(result!=NULL){
      the_expr->get_parent()->replace(the_expr, result);
      set_address(result);
      return Walker::Replaced;
    }else
      return Walker::Continue;
}

Walker::ApplyStatus if_walker::operator() (SuifObject *x){
    IfStatement *the_if = to<IfStatement>(x);
    Statement *result = NULL;

    Expression *condition = the_if->get_condition();

    if(!is_kind_of<IntConstant>(condition)){
      return Walker::Continue;
    }

    IInteger cond_int = (to<IntConstant>(condition))->get_value();
    Statement *then_part = the_if->get_then_part();
    Statement *else_part = the_if->get_else_part();

    if(cond_int.is_c_int() && cond_int.c_int()!=0){
      /* Leave the then branch only */
      the_if->set_else_part(NULL);
      remove_suif_object(else_part);
    }else{
      /* Leave the else branch only -- reverse the labels */
      remove_suif_object(condition);

      // likewise of these parts - remove from if.
      the_if->set_then_part(NULL);
      the_if->set_else_part(NULL);
      remove_suif_object(then_part);
      remove_suif_object(else_part);

      if (then_part != NULL) {
        StatementList *replacement = create_statement_list( get_env() );
        result = replacement;
        UnaryExpression* negated_condition =
          create_unary_expression( get_env(), condition->get_result_type(), k_negate, condition );

        if (else_part != NULL) {
          CodeLabelSymbol *else_label = create_new_label();
          CodeLabelSymbol *done_label = create_new_label();
          replacement-> append_statement(create_branch_statement(get_env(),
                                                    negated_condition, else_label));
          replacement->append_statement(then_part);
          replacement->append_statement(create_jump_statement(get_env(),done_label));
          replacement-> append_statement(create_label_location_statement(get_env(), else_label));
          replacement->append_statement(else_part);
          replacement-> append_statement(create_label_location_statement(get_env(), done_label));
        }else{
          CodeLabelSymbol *done_label = create_new_label();
          replacement-> append_statement(create_branch_statement(get_env(),
                                                negated_condition, done_label));
          replacement->append_statement(then_part);
          replacement-> append_statement(create_label_location_statement(get_env(), done_label));
        }
      }else {
      if (else_part != NULL){
        StatementList *replacement = create_statement_list(get_env());
        result = replacement;
              CodeLabelSymbol *done_label = create_new_label();
              replacement-> append_statement(create_branch_statement(get_env(), condition, done_label));
              replacement->append_statement(else_part);
              replacement-> append_statement(create_label_location_statement(get_env(), done_label));
      }else{
        EvalStatement *replacement = create_eval_statement(get_env());
        result = replacement;
        replacement->append_expression(condition);
        }
      }
    }

    if(result!=NULL){
      the_if->get_parent()->replace(the_if,result);
      set_address(result);
      return Walker::Replaced;
    }else{
      return Walker::Continue;
    }
}
