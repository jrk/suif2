
#ifndef _SUIFLINK_ANSIC_H_
#define _SUIFLINK_ANSIC_H_

#include "suifnodes/suif.h"
#include "common/suif_list.h"
#include "utils/type_pair_stack.h"

/** This class understands ANSI C standard.
  */
class ANSIC {
 public:

  /** Perform ANSI unary conversion.
    * @param t  the original type.
    * @return   the type after unary conversion of \a t.
    */
  Type *convert_unary(Type* t);

  /** Perform ANSI binary conversion.
    * @param t1 type.
    * @param t2 type.
    * @return the type after binary conversion of \a t1 and \a t2.
    */
  Type *convert_binary(Type* t1, Type* t2);

  /** Perform ANSI C argument promotion.
    * @param t type before promotion.
    * @return the type after promoting \a t.
    */
  Type *promote_argument(Type* t);

  /** Get the composite of two types.
    * @param t1 type.
    * @param t2 type.
    * @param map for stopping infinite recursion.  External caller should just
    *        take the default.
    * @return the composite of \a t1 and \a t2, as defined by ANSI C standard.
    *         Return NULL if \a t1 and \a t2 are not compatible.
    */
  Type *get_composite(Type* t1, Type* t2, TypePairStack *map=NULL);

  /** Find if two types are compatible.
    * @param t1 type.
    * @param t2 type.
    * @return true iff \a t1 and \a t2 are compatible.
    */
  bool is_compatible_type(Type* t1, Type* t2);

  /** Find if it is legal to cast from one type to another.
    * @param oldtype  original type.
    * @param newtype  target type of casting.
    * @return true iff it is legal to cast from \a oldtype to \a newtype.
    */
  bool is_legal_cast(Type* oldtype, Type* newtype);

  String get_warnings();

 private:

  String warnings;

  int get_int_bit_size(SuifEnv*);

  void add_warning(Type *, Type *, String);

  Type* convert_binary_data(DataType *bigger, DataType *smaller);
  Type* get_composite_qualified(QualifiedType *qt1, QualifiedType *qt2, TypePairStack *map);

  Type* get_composite_enumerated(EnumeratedType *t1, EnumeratedType *t2, TypePairStack *map);

  Type* get_composite_enumerated_integer(EnumeratedType *t1,
					 IntegerType *t2,
					 TypePairStack *map);

  ArrayType* find_array_type( SuifEnv *senv,
			      IInteger size_in_bits,
			      int alignment_in_bits,
			      QualifiedType* element_type,
			      Expression *lower_bound,
			      Expression *upper_bound);

  Type* get_composite_array(ArrayType *t1, ArrayType *t2,
			    TypePairStack *map);
  
  Type* get_composite_multi_dim_array(MultiDimArrayType *t1,
				      MultiDimArrayType *t2,
				      TypePairStack *map);
  
  list<QualifiedType*> get_arg_list(CProcedureType *proc);
  
  Type* get_composite_1prototype(Type* ret,
				 CProcedureType *proc,
				 TypePairStack *map);
  
  Type* get_composite_2prototype(Type* ret,
				 CProcedureType *t1,
				 CProcedureType *t2,
				 TypePairStack *map);
  
  Type* get_composite_cprocedure(CProcedureType *t1, CProcedureType *t2,
				 TypePairStack *map);
  
  Type* get_composite_group(GroupType *t1, GroupType *t2,
			    TypePairStack *map);
  
  
  Type* get_composite_pointer(PointerType *t1, PointerType *t2,
			      TypePairStack *map);
  
  
};

#endif /* _SUIFLINK_ANSIC_H_ */
