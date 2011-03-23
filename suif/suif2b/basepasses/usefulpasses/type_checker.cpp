#include "common/system_specific.h"
#include "common/suif_copyright.h"

#include "iokernel/cast.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"
#include "suifkernel/suifkernel_messages.h"
#include "transforms/procedure_walker_utilities.h"
#include "type_checker.h"
#include "utils/type_utils.h"
#include "cfenodes/cfe.h"
#include "utils/print_utils.h"

/**************************** Declarations ************************************/

class type_checker_walker: public SelectiveWalker {
public:
  type_checker_walker(SuifEnv *the_env)
    : SelectiveWalker(the_env, SuifObject::get_class_name()) {}

  Walker::ApplyStatus operator () (SuifObject *x);
};


/**************************** Implementations ************************************/
TypeCheckerPass::TypeCheckerPass(SuifEnv *pEnv, const LString &name) : 
  Pass(pEnv, name) {}

void TypeCheckerPass::do_file_set_block(FileSetBlock *fsb){
  type_checker_walker walker(get_suif_env());
  fsb->walk(walker);
}

static ProcedureDefinition *find_owning_procedure(Statement *st) {
  SuifObject *obj = st->get_parent();
  while(obj) {
    if (is_kind_of<ProcedureDefinition>(obj)) 
      return(to<ProcedureDefinition>(obj));
    // We are only walking up through execution Objects.
    if (!is_kind_of<ExecutionObject>(obj))
      return(NULL);
    obj = obj->get_parent();
  }
  return(NULL);
}

// This will find the last non-StatementList statement
//
static Statement *find_last_statement(Statement *st) {
  if (st == NULL) return NULL;
  if (is_kind_of<StatementList>(st)) {
    StatementList *the_list = to<StatementList>(st);
    size_t len = the_list->get_statement_count();
    // An empty statement list
    if (len == 0) return(st);
    return(find_last_statement(the_list->get_statement(len -1)));
  }
  return(st);
}

Walker::ApplyStatus type_checker_walker::operator () (SuifObject *x){
    //SuifEnv *the_env = get_env();
    //Type *the_expr = to<Type>(x);

    // checks for pointer types
    if (is_kind_of<PointerType>(x)) {
      PointerType *pt = to<PointerType>(x);
      Type *ref = pt->get_reference_type();
      if (is_kind_of<DataType>(ref)) {
	suif_warning(pt, 
		     "Pointer type reference to DataType not QualifiedType");
      }
    }
    if (is_kind_of<ArrayType>(x)) {
      ArrayType *t = to<ArrayType>(x);
      if (t->get_upper_bound() == NULL) {
	suif_warning(t, "Array type with NULL upper bound");
      }
      if ((t->get_lower_bound() == NULL)) {
	suif_warning(t, "Array type with NULL lower bound");
      }
    }

    if (is_kind_of<ProcedureDefinition>(x)) {
      ProcedureDefinition *pd = to<ProcedureDefinition>(x);
      ExecutionObject *body = pd->get_body();
      if (body && is_kind_of<Statement>(body)) {
	Statement *last_st = find_last_statement(to<Statement>(body));
	if (!is_kind_of<ReturnStatement>(last_st)) {
	  suif_warning(pd, "ProcedureDefinition does not have a ReturnStatement at the end of the body");
	}
      }
    }

    if (is_kind_of<ReturnStatement>(x)) {
      ReturnStatement *ret = to<ReturnStatement>(x);
      Expression *ret_expr = ret->get_return_value();

      // Find the owning procedure
      ProcedureDefinition *pd = find_owning_procedure(ret);
      ProcedureSymbol *ps = pd->get_procedure_symbol();
      ProcedureType *t = to<ProcedureType>(ps->get_type());
      if (is_kind_of<CProcedureType>(t)) {
	CProcedureType *ct = to<CProcedureType>(t);
	DataType *result = ct->get_result_type();
	if (is_kind_of<VoidType>(result)) {
	  // Expect the return expression to be NULL
	  if (ret_expr != NULL) {
	    suif_warning(ret, "In Procedure with no return value, ReturnStatement return_value is not NULL");
	  }
	} else {
	  // They should be the same.
	  if (ret_expr == NULL) {
	    suif_warning(ret, "In Procedure with return value, ReturnStatement return_value is NULL");
	  } else {
	    DataType *act = ret_expr->get_result_type();
	    if (act != result) {
	      suif_warning(ret, "In Procedure with return value, ReturnStatement expression type does not match");
	    }
	  }
	}
      }
    }

    if (is_kind_of<Expression>(x)) {
      if (to<Expression>(x)->get_result_type() == NULL) {
	suif_warning(x, "No result type for Expression");
	return Walker::Continue;
      }
    }

    if (is_kind_of<ArrayReferenceExpression>(x)) {
      ArrayReferenceExpression *t = to<ArrayReferenceExpression>(x);
      DataType *return_type = t->get_result_type();
      if (!is_kind_of<PointerType>(return_type)) {
	suif_warning(t, "ArrayReferenceExpressiont:"
		     " result type is not a Pointer");
	return Walker::Continue;
      }

      PointerType *pt = to<PointerType>(return_type);
      Type *elem = pt->get_reference_type();
      if (!is_kind_of<QualifiedType>(elem)) {
	suif_warning(t, "ArrayReferenceExpressiont:"
		     " result type is not a Pointer to a qualified type");
	return Walker::Continue;
      }
      QualifiedType *qt = to<QualifiedType>(elem);

      
      DataType *base_type = t->get_base_array_address()->get_result_type();
      if (!is_kind_of<PointerType>(base_type)) {
	suif_warning(t, "ArrayReferenceExpression:"
		     " base_address result type is not a Pointer");
	return Walker::Continue;
      }
      PointerType *base_pt = to<PointerType>(base_type);
      Type *base_elem = base_pt->get_reference_type();
      if (!is_kind_of<QualifiedType>(base_elem)) {
	suif_warning(t, "ArrayReferenceExpression:"
		     " base_address result type is not a Pointer to a "
		     "qualified type");
	return Walker::Continue;
      }
      DataType *base_e = to<QualifiedType>(base_elem)->get_base_type();
      if (!is_kind_of<ArrayType>(base_e)) {
	suif_warning(t, "ArrayReferenceExpression:"
		     " base_address result type is not a Pointer to a "
		     "qualified Array");
	return Walker::Continue;
      }
      
      ArrayType *at = to<ArrayType>(base_e);
      QualifiedType *array_elem_qt = at->get_element_type();

      if (array_elem_qt != qt) {
	suif_warning(t, "ArrayReferenceExpression:"
		     " result type is not a Pointer to the same qualified"
		     " type as the element type of the array pointed to by"
		     " the base_address result type");
	return Walker::Continue;
      }
    }

      
    if (is_kind_of<LoadVariableExpression>(x)) {
      LoadVariableExpression *ld = to<LoadVariableExpression>(x);
      if (get_data_type(ld->get_source()) !=
	  ld->get_result_type()) {
	suif_warning(ld, 
		     "LoadVariableExpression result type does not match unqualified variable type");
      }
    }

    if (is_kind_of<CallExpression>(x)) {
      CallExpression *expr = to<CallExpression>(x);
      if(expr->get_result_type()==NULL) {
	        suif_warning(expr,
		     "Expression result type is NULL");
      }
    }


    if (is_kind_of<SymbolAddressExpression>(x)) {
      // The result type should be a pointer to the type of the symbol.
      SymbolAddressExpression *expr = to<SymbolAddressExpression>(x);
      Symbol *sym = expr->get_addressed_symbol();
      DataType *t = expr->get_result_type();
      if (!is_kind_of<PointerType>(t)) {
	  suif_warning(expr,
		       "SymbolAddressExpression result type is not a PointerType\n");
      } else {
	PointerType *pt = to<PointerType>(t);
	Type *pt_t = unqualify_type(pt->get_reference_type());
	Type *s_t = unqualify_type(sym->get_type());
	if (pt_t != s_t) {
	  suif_warning(expr, 
		       "SymbolAddressExpression [%s]: result type [%s] is not a"
		       " pointer to the symbol type [%s]\n",
		       to_id_string(x).c_str(), to_id_string(pt_t).c_str(),
		       to_id_string(s_t).c_str());
	}
      }
    }

    if (is_kind_of<LoadExpression>(x)) {
      LoadExpression *ld = to<LoadExpression>(x);
      Expression *ld_source = ld->get_source_address();
      // expect the load source to be a pointer type
      DataType *ld_src_t = ld_source->get_result_type();
      if (!is_kind_of<PointerType>(ld_src_t)) {
	suif_warning(ld, 
		     "LoadExpression source result type is not a pointer");
      } else {
	PointerType *pt = to<PointerType>(ld_src_t);
	Type *pt_ref = pt->get_reference_type();
	if (!is_kind_of<QualifiedType>(pt_ref)) {
	  suif_warning(ld, 
		       "LoadExpression source result type is not a pointer to a QualifiedType");
	} else {
	  DataType *dt = to<QualifiedType>(pt_ref)->get_base_type();
	  if (dt != ld->get_result_type()) {
	    suif_warning(ld, 
			 "LoadExpression unqualified source result type is not the same as the result type");
	  }
	}
      }
    }
	    
    return(Walker::Continue);
}
