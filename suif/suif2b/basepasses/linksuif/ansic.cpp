/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "ansic.h"
#include "typebuilder/type_builder.h"
#include "utils/type_utils.h"
#include "utils/print_utils.h"
#include "basicnodes/basic_factory.h"
#include "iostream.h" //debug
#include "suifkernel/suif_env.h"
#include "suifprinter/suifprinter.h"


void 
ANSIC::add_warning(Type *t1, Type *t2, String reason)
{

  SuifEnv *se = t1->get_suif_env();
  PrintSubSystem *printer = se->get_print_subsystem();
  warnings += "linksuif warning: failed to merge types ";
  warnings += reason;
  warnings += "\n        ";
  warnings += printer->print_to_string(t1);
  warnings += "    and ";
  warnings += printer->print_to_string(t2);
}

      
String
ANSIC::get_warnings(void)
{
  return warnings;
}


// Find the Type resulting from the uniary convertion according to
// ANSI C rule
// Return the new type or the original type.
//
Type *ANSIC::convert_unary(Type *t1)
{
  if (is_kind_of<IntegerType>(t1)) {
    IntegerType *itype = to<IntegerType>(t1);
    if (itype->get_bit_size() < get_int_bit_size(t1->get_suif_env())) {
      return get_integer_type(t1->get_suif_env(), itype->get_is_signed());
    } else
      return t1;
  } else if (is_kind_of<ProcedureType>(t1)) {
    return get_pointer_type(t1->get_suif_env(), t1);
  } else if (is_kind_of<ArrayType>(t1)) {
    return get_pointer_type(t1->get_suif_env(), 
			    to<ArrayType>(t1)->get_element_type());
  }
  return t1;
}



Type *ANSIC::promote_argument(Type *t)
{
  if (is_kind_of<FloatingPointType>(t)) {
    return get_double_floating_point_type(t->get_suif_env());
  }
  return convert_unary(t);
}



// Return the number of bits in an int.
//
int ANSIC::get_int_bit_size(SuifEnv *env)
{
  IntegerType *itype = get_integer_type(env, true);
  return itype->get_bit_size().c_int();
}


// PreCondition  bit_size(larger) >= bit_size(smaller), both have already
//               been converted by usual unary conversion rule.
//
Type* ANSIC::convert_binary_data(DataType *bigger, DataType *smaller)
{
  if (is_kind_of<FloatingPointType>(bigger))
    return bigger;
  if (is_kind_of<FloatingPointType>(smaller))
    return smaller;
  if (!is_kind_of<IntegerType>(bigger)) {
    add_warning(bigger, smaller, "(non-int datatypes)");
    return NULL;
  }
  IntegerType *ilarge = to<IntegerType>(bigger);
  IntegerType *ismall = to<IntegerType>(smaller);
  if (!ilarge->get_is_signed())
    return ilarge;
  if (!ismall->get_is_signed())
    return ismall;
  return bigger;
}



// Return true a common converted type, or NULL.
//
Type *ANSIC::convert_binary(Type* t1, Type *t2)
{
  Type *y1 = convert_unary(t1);
  Type *y2 = convert_unary(t2);
  if (!is_kind_of<DataType>(y1)) {
    add_warning(t1, t2, "(first type not datatypes)");
    return NULL;
  }
  if (!is_kind_of<DataType>(y2)) {
    add_warning(t1, t2, "(second type not datatype)");
    return NULL;
  }
  DataType *d1 = to<DataType>(y1);
  DataType *d2 = to<DataType>(y2);
  if (d2->get_bit_size() > d1->get_bit_size())
    return convert_binary_data(d2, d1);
  else
    return convert_binary_data(d2, d1);
}


// Return true of an automatic cast from oldtype to newtype is ok.
//
bool ANSIC::is_legal_cast(Type *oldtype, Type *newtype)
{
  if (oldtype == newtype)
    return true;
  if (is_kind_of<QualifiedType>(oldtype))
    return is_legal_cast(to<QualifiedType>(oldtype)->get_base_type(), newtype);
  if (is_kind_of<QualifiedType>(newtype))
    return is_legal_cast(oldtype, to<QualifiedType>(newtype)->get_base_type());
  if (is_kind_of<IntegerType>(newtype))
    return (is_kind_of<IntegerType>(oldtype) ||
	    is_kind_of<FloatingPointType>(oldtype) ||
	    is_kind_of<PointerType>(oldtype));
  if (is_kind_of<FloatingPointType>(newtype))
    return (is_kind_of<NumericType>(oldtype));
  if (is_kind_of<PointerType>(newtype) &&
      is_kind_of<ProcedureType>(to<PointerType>(newtype)->
				get_reference_type()))
    return (is_kind_of<IntegerType>(oldtype) ||
	    (is_kind_of<PointerType>(oldtype) &&
	     is_kind_of<ProcedureType>(to<PointerType>(oldtype)->
				       get_reference_type())));
  if (is_kind_of<PointerType>(newtype))
    return (is_kind_of<IntegerType>(oldtype) ||
	    is_kind_of<PointerType>(oldtype));
  if (is_kind_of<VoidType>(newtype))
    return true;
  return false;
}
  



Type* ANSIC::get_composite_qualified(QualifiedType *qt1, QualifiedType *qt2,
					  TypePairStack *map)
{
  if (!TypeHelper::is_same_qualified(qt1, qt2)) {
    add_warning(qt1, qt2, "(qualifier mismatch)");
    return NULL;
  }
  Type *comp = get_composite(qt1->get_base_type(),
				    qt2->get_base_type(),
				    map);
  if (comp == NULL) return NULL;
  if (!is_kind_of<DataType>(comp)) {
    add_warning(qt1, qt2, "(composite is not a datatype)");
    return NULL;
  }
  list<LString> qual;
  Iter<LString> it = qt1->get_qualification_iterator();
  for (; it.is_valid(); it.next()) {
    qual.push_back(it.current());
  }
  return get_qualified_type(qt1->get_suif_env(), to<DataType>(comp), qual);
}

// Standrad says if they come from the same translation unit, then
//  they will never be compatible (unless t1==t2).
// Because translation unit info is not available, I only check
//  of isomorphic equivalence.
//
Type* ANSIC::get_composite_enumerated(EnumeratedType *t1, EnumeratedType *t2,
				      TypePairStack *map)
{
  if (TypeHelper::is_isomorphic_type(t1, t2))
    return t1;
  add_warning(t1, t2, "(enums not isomorphic)");
  return NULL;
}

 
Type* ANSIC::get_composite_enumerated_integer(EnumeratedType *t1,
					      IntegerType *t2,
					      TypePairStack *map)
{
  if (t1->get_bit_size() == t2->get_bit_size() &&
      t1->get_is_signed() == t2->get_is_signed())
    return t1;
  add_warning(t1, t2, "(size or signedness mismatch)");
  return NULL;
}


// Need to create a new instance for lower_bound and upper_bound before
// passing to get_array_type()
//
ArrayType* ANSIC::find_array_type( SuifEnv *senv,
				  IInteger size_in_bits,
				  int alignment_in_bits,
				  QualifiedType* element_type,
				  Expression *lower_bound,
				  Expression *upper_bound)
{
  if (!is_kind_of<IntConstant>(lower_bound))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, 
				String("Unable to handle non IntConstant - ") +
				to_id_string(lower_bound)));
  if (!is_kind_of<IntConstant>(upper_bound))
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, 
				String("Unable to handle non IntConstant - ") +
				to_id_string(upper_bound)));
  IntConstant *lb = to<IntConstant>(lower_bound);
  IntConstant *ub = to<IntConstant>(upper_bound);
  return get_array_type(senv, size_in_bits, alignment_in_bits, element_type,
			create_int_constant(senv, lb->get_result_type(),
					    lb->get_value()),
			create_int_constant(senv, ub->get_result_type(),
					    ub->get_value()));
}
  

  
Type* ANSIC::get_composite_array(ArrayType *t1, ArrayType *t2,
				 TypePairStack *map)
{
  Type *cele = get_composite(t1->get_element_type(),
			     t2->get_element_type(),
			     map);

  if (cele == NULL) return NULL;
  QualifiedType *dele = to<QualifiedType>(cele);
  if (TypeHelper::is_unknown_but_compatible_bound(t1, t2))
    return find_array_type(t2->get_suif_env(), t2->get_bit_size(),
			   t2->get_bit_alignment(), dele,
			   t2->get_lower_bound(), t2->get_upper_bound());
  if (TypeHelper::is_unknown_but_compatible_bound(t2, t1))
    return find_array_type(t1->get_suif_env(), t1->get_bit_size(),
			   t1->get_bit_alignment(), dele,
			   t1->get_lower_bound(), t1->get_upper_bound());
  if (!TypeHelper::is_isomorphic_bound(t1, t2)) {
    add_warning(t1, t2, "(array bounds not isomorphic)");
    return NULL;
  }
  return find_array_type(t1->get_suif_env(), t1->get_bit_size(),
			 t1->get_bit_alignment(), dele,
			 t1->get_lower_bound(), t1->get_upper_bound());
}



  
Type* ANSIC::get_composite_multi_dim_array(MultiDimArrayType *t1,
					   MultiDimArrayType *t2,
					   TypePairStack *map)
{
  unsigned bcnt = t1->get_lower_bound_count();
  if (t2->get_lower_bound_count() != bcnt) {
    add_warning(t1, t2, "(dimensionality mismatch)");
    return NULL;
  }
    
  Type* dele = get_composite(t1->get_element_type(),
			     t2->get_element_type(),
			     map);
  if (dele == NULL) return NULL;
  if (bcnt == 0)
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, to_id_string(t1) +
			     " : illegal zero dimension array"));
  suif_vector<Expression*> lbounds;
  suif_vector<Expression*> ubounds;
  if (TypeHelper::is_unknown_int(t1->get_lower_bound(0)))
    lbounds[0] = t2->get_lower_bound(0);
  if (TypeHelper::is_unknown_int(t2->get_lower_bound(0)))
    lbounds[0] = t1->get_lower_bound(0);
  if (TypeHelper::is_unknown_int(t1->get_upper_bound(0)))
    lbounds[0] = t2->get_upper_bound(0);
  if (TypeHelper::is_unknown_int(t2->get_upper_bound(0)))
    lbounds[0] = t1->get_upper_bound(0);
  for (unsigned i=1; i<bcnt; i++) {
    if (!TypeHelper::is_isomorphic_int(t1->get_lower_bound(i),
				       t2->get_lower_bound(i))) {
      add_warning(t1, t2, "(lower bounds not isomorphic)");
      return NULL;
    }
    if (!TypeHelper::is_isomorphic_int(t1->get_upper_bound(i),
				       t2->get_upper_bound(i))) {
      add_warning(t1, t2, "(upper bounds not isomorphic)");
      return NULL;
    }
    lbounds[i] = t1->get_lower_bound(i);
    ubounds[i] = t1->get_upper_bound(i);
  }
  return get_multi_dim_array_type(t1->get_suif_env(), to<QualifiedType>(dele),
				  lbounds, ubounds);
}




list<QualifiedType*> ANSIC::get_arg_list(CProcedureType *proc)
{
  list<QualifiedType*> lst;
  for (Iter<QualifiedType*> it = proc->get_argument_iterator();
       it.is_valid();
       it.next()) {
    lst.push_back(it.current());
  }
  return lst;
}
  

Type* ANSIC::get_composite_1prototype(Type* ret,
				      CProcedureType *proc,
				      TypePairStack *map)
{
  if (proc->get_has_varargs()) {
    add_warning(proc, proc, "(has varargs)");
    return NULL;
  }
  Iter<QualifiedType*> it = proc->get_argument_iterator();
  for (; it.is_valid(); it.next()) {
    Type *carg = get_composite(it.current(),
			       convert_unary(it.current()),
			       map);
    if (carg == NULL) return NULL;
  }
  list<QualifiedType*> args = get_arg_list(proc);
  return get_c_procedure_type(proc->get_suif_env(), to<DataType>(ret), args,
			      false, true, proc->get_bit_alignment());
}


Type* ANSIC::get_composite_2prototype(Type* ret,
				      CProcedureType *t1,
				      CProcedureType *t2,
				      TypePairStack *map)
{
  if (t1->get_has_varargs() != t2->get_has_varargs()) {
    add_warning(t1, t2, "(varargness mismatch)");
    return NULL;
  }
  if (t1->get_argument_count() != t2->get_argument_count()) {
    add_warning(t1, t2, "(argument count mismatch)");
    return NULL;
  }
  list <QualifiedType*> cargs;
  for (size_t i=0; i<t1->get_argument_count(); i++) {
    Type *c = get_composite(t1->get_argument(i), t2->get_argument(i),
				   map);
    if (c == NULL) {
      add_warning(t1, t2, String("(type mismatch in parameter ") + String(i) + String(")"));
      return NULL;
    }
    cargs.push_back(to<QualifiedType>(c));
  }
  return get_c_procedure_type(t1->get_suif_env(), to<DataType>(ret), cargs,
			      t1->get_has_varargs(), true,
			      t1->get_bit_alignment());
}

    

Type* ANSIC::get_composite_cprocedure(CProcedureType *t1, CProcedureType *t2,
			   TypePairStack *map)
{
  Type *ret = get_composite(t1->get_result_type(),
			    t2->get_result_type(),
			    map);
  if (ret == NULL) {
        add_warning(t1, t2, "(return type mismatch)");
	return NULL;
  }
  if (!t1->get_arguments_known() && !t2->get_arguments_known()) {
    list<QualifiedType*> args;
    return get_c_procedure_type(t1->get_suif_env(), to<DataType>(ret), args,
				false, false, t1->get_bit_alignment());
  }
  if (t1->get_arguments_known() && !t2->get_arguments_known())
    return get_composite_1prototype(ret, t1, map);
  if (!t1->get_arguments_known() && t2->get_arguments_known())
    return get_composite_1prototype(ret, t2, map);
  return get_composite_2prototype(ret, t1, t2, map);
}


  
Type* ANSIC::get_composite_group(GroupType *t1, GroupType *t2,
				 TypePairStack *map)
{
  if (t1->get_name() != t2->get_name()) {
    add_warning(t1, t2, "(group type mismatch)");
    return NULL;
  }
  if (!t1->get_is_complete() && !t2->get_is_complete())
    return t1;
  if (t1->get_is_complete() && !t2->get_is_complete())
    return t1;
  if (!t1->get_is_complete() && t2->get_is_complete())
    return t2;
  // both are complete
  GroupSymbolTable *gtab1 = t1->get_group_symbol_table();
  GroupSymbolTable *gtab2 = t2->get_group_symbol_table();
  if (gtab1->get_symbol_table_object_count() !=
      gtab2->get_symbol_table_object_count()) {
    add_warning(t1, t2, "(group size mismatch)");
    return NULL;
  }
  TypePairStack mymap(t1, t2, map);
  for (Iter<SymbolTableObject*> it = gtab1->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    FieldSymbol* f1 = to<FieldSymbol>(it.current());
    FieldSymbol* f2 = TypeHelper::lookup_field(gtab2, f1->get_name());
    if (f2 == NULL) {
      add_warning(t1, t2, String("(field ") + f1->get_name() + String(" does not exist in the second type)") );
      return NULL;
    }
    if (!TypeHelper::is_isomorphic_int(f1->get_bit_offset(),
				       f2->get_bit_offset())) {
      add_warning(t1, t2, "(bit offset mismatch)");
      return NULL;
    }
    if (get_composite(f1->get_type(), f2->get_type(), &mymap) == NULL) {
      add_warning(t1, t2, String("(type mismatch of field ") + f1->get_name() + String(")") );
      return NULL;
    }
  }
  return t1;
}

Type* ANSIC::get_composite_pointer(PointerType *t1, PointerType *t2,
				   TypePairStack *map)
{
  Type *comp = get_composite(t1->get_reference_type(),
				    t2->get_reference_type(),
				    map);
  if (comp == NULL) return NULL;
  return get_pointer_type(t1->get_suif_env(), comp);
}



Type* ANSIC::get_composite(Type* t1, Type* t2, TypePairStack *map)
{
  if (t1 == t2) return t1;
  if (t1->get_name() != t2->get_name()) {
    add_warning(t1, t2, "(name mismatch)");
    return NULL;
  }
  if (map != NULL && map->is_in(t1, t2))
    return t1;
  if (is_kind_of<QualifiedType>(t1) && is_kind_of<QualifiedType>(t2))
    return get_composite_qualified(to<QualifiedType>(t1),
				   to<QualifiedType>(t2),
				   map);
  if (is_kind_of<EnumeratedType>(t1) && is_kind_of<EnumeratedType>(t2))
    return get_composite_enumerated(to<EnumeratedType>(t1),
				    to<EnumeratedType>(t2),
				    map);
  if (is_kind_of<EnumeratedType>(t1) && is_kind_of<IntegerType>(t2))
    return get_composite_enumerated_integer(to<EnumeratedType>(t1),
					    to<IntegerType>(t2),
					    map);
  if (is_kind_of<EnumeratedType>(t2) && is_kind_of<IntegerType>(t1))
    return get_composite_enumerated_integer(to<EnumeratedType>(t2),
					    to<IntegerType>(t1),
					    map);
  if (is_kind_of<ArrayType>(t1) && is_kind_of<ArrayType>(t2))
    return get_composite_array(to<ArrayType>(t1), to<ArrayType>(t2), map);
  if (is_kind_of<MultiDimArrayType>(t1) && is_kind_of<MultiDimArrayType>(t2))
    return get_composite_multi_dim_array(to<MultiDimArrayType>(t1),
					 to<MultiDimArrayType>(t2),
					 map);
  if (is_kind_of<CProcedureType>(t1) && is_kind_of<CProcedureType>(t2))
    return get_composite_cprocedure(to<CProcedureType>(t1),
				    to<CProcedureType>(t2),
				    map);
  if (is_kind_of<GroupType>(t1) && is_kind_of<GroupType>(t2))
    return get_composite_group(to<GroupType>(t1), to<GroupType>(t2), map);
  if (is_kind_of<PointerType>(t1) && is_kind_of<PointerType>(t2))
    return get_composite_pointer(to<PointerType>(t1), to<PointerType>(t2),
				 map);

  add_warning(t1, t2, "(no composite creation rule)");
  return NULL;
}


bool ANSIC::is_compatible_type(Type* t1, Type* t2)
{
  warnings = "";
  Type *comp = get_composite(t1,t2);
  if (comp==NULL) {
    add_warning(t1, t2, "(symbol types are not compatible)");
  }
    
  return (comp!=NULL);
}
