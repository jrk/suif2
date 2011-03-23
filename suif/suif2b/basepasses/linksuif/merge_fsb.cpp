// This file contains the function merge_fsb() and its sub functions.
// The function merge_fsb() is called by suiflinker in the frist phase
//   of the linking process.
// merge_fsb( FileSetBlock *master, FileSetBlock *slave) does the following:
//  1. Merge the external symbol tables into the master external table.
//     If a type object in the slave table has a duplicate (isomorphic) one
//     in the master, it will not be transferred.  All occurences of this old
//     type in the slave fsb are set to the duplicate object from the master
//     table.  Otherwise, the old object is added to the master table.
//  2. All file blocks from the slave fsb are added to the master fsb.

#include "suif_linker.h"
#include "utils/type_utils.h"
#include "common/suif_map.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/all_walker.h"
#include "typebuilder/type_builder.h"
#include "utils/print_utils.h"

#include <iostream.h>



/*static*/ void print_symbol_table(SymbolTable *symtab, ostream& out,
			       const char* prefix)
{
  out << prefix << " SymbolTable: {" << endl;
  for (Iter<SymbolTableObject*> it = symtab->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    out << to_id_string(it.current()) << endl;
  }
  out << "------lookup-table--------" << endl;
  for (Iter<SymbolTable::lookup_table_pair> it =
	 symtab->get_lookup_table_iterator();
       it.is_valid();
       it.next()) {
    out << it.current().first << " -> " << to_id_string(it.current().second)
	<< endl;
  }
  out << "}" << endl;
}






// Make obj an orpha - remove it from its parent symbol table
//
static void make_orphan(SymbolTableObject *obj)
{
  SuifObject* parent = obj->get_parent();
  if (parent == NULL) {
    return;
  } else if (!is_kind_of<SymbolTable>(parent)) {
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, to_id_string(obj) +
				" not owned by a SymbolTable but by " +
				to_id_string(obj->get_parent())));
  } else {
    to<SymbolTable>(parent)->remove_symbol(obj);
  }
}




// Add to SymbolTable symtab a new type object.
// If the type object already exist in symtab, do nothing.
//
static Type* add_type_to_table(SymbolTable* symtab, Type* newtype)
{
  if (symtab->has_symbol_table_object_member(newtype))
    return newtype;
  if (newtype->get_parent() != NULL)
    make_orphan(newtype);
  symtab->add_symbol(newtype);
  return newtype;
}






/* ***************************************************
 * merge_type
 ***************************************************** */


QualifiedType* SuifLinker::merge_qualified(QualifiedType *oldtype,
					   ReplaceMap *map,
					   SymbolTable *symtab)
{
  Type *basetype = merge_type(oldtype->get_base_type(), map, symtab);
  list<LString> qual;
  for (Iter<LString> it = oldtype->get_qualification_iterator();
       it.is_valid();
       it.next()) {
    qual.push_back(it.current());
  }
  QualifiedType* res = get_qualified_type(symtab->get_suif_env(),
					  to<DataType>(basetype),
					  qual);
  add_type_to_table(symtab, res);
  map->add_entry(oldtype, res);
  return res;
}
			      

PointerType *SuifLinker::merge_pointer(PointerType *oldtype,
				       ReplaceMap *typemap,
				       SymbolTable *symtab)
{
  Type *reftype = merge_type(oldtype->get_reference_type(), typemap, symtab);
  PointerType *res = get_pointer_type(symtab->get_suif_env(), reftype);
  add_type_to_table(symtab, res);
  typemap->add_entry(oldtype, res);
  return res;
}


IntConstant *SuifLinker::merge_bound(Expression* exp,
				     ReplaceMap *typemap,
				     SymbolTable *symtab)
{
  if (!is_kind_of<IntConstant>(exp))
    SUIF_THROW(SuifException(String("Expecting only IntConstant as array "
				    "bounds - ") + to_id_string(exp)));
  IntConstant *bound = to<IntConstant>(exp);
  Type *result_type = merge_type(exp->get_result_type(), typemap, symtab);
  return create_int_constant(exp->get_suif_env(), to<DataType>(result_type),
			     bound->get_value());
}

  
ArrayType *SuifLinker::merge_array(ArrayType *oldtype,
				   ReplaceMap *typemap,
				   SymbolTable *symtab)
{
  Type* eletype = merge_type(oldtype->get_element_type(), typemap, symtab);
  Expression *lbound = merge_bound(oldtype->get_lower_bound(), typemap,
				   symtab);
  Expression *ubound = merge_bound(oldtype->get_upper_bound(), typemap,
				   symtab);
  ArrayType *res = get_array_type(symtab->get_suif_env(),
				  to<QualifiedType>(eletype),
				  lbound, ubound);
  add_type_to_table(symtab, res);
  typemap->add_entry(oldtype, res);
  return res;
}


CProcedureType *SuifLinker::merge_cprocedure(CProcedureType *oldtype,
					     ReplaceMap *typemap,
					     SymbolTable *symtab)
{
  Type* result = merge_type(oldtype->get_result_type(), typemap, symtab);
  list<QualifiedType*> args;
  for (unsigned i=0; i<oldtype->get_argument_count(); i++)
    args.push_back(to<QualifiedType>(merge_type(oldtype->get_argument(i),
						typemap, symtab)));
  CProcedureType *res = get_c_procedure_type(symtab->get_suif_env(),
					     to<DataType>(result),
					     args,
					     oldtype->get_has_varargs(),
					     oldtype->get_arguments_known(),
					     oldtype->get_bit_alignment());
  add_type_to_table(symtab, res);
  typemap->add_entry(oldtype, res);
  return res;
}



// Return the type object from symtab that is isomorphic with oldtype and
// has the same name as oldtype and its name is not an empty string
//
static GroupType *find_incomplete_group_type(Type *oldtype,
					     SymbolTable *symtab)
{
  LString name = oldtype->get_name();
  if (name.length() == 0)
    return NULL;
  for (Iter<SymbolTableObject*> it = symtab->
	 get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    if (!is_kind_of<GroupType>(it.current()))
      continue;
    if (it.current()->get_name() != name)
      continue;
    GroupType* gtype = to<GroupType>(it.current());
    if (gtype->get_is_complete())
      return gtype;
  }
  return NULL;
}



// merge the field types in gtype
//
GroupType *SuifLinker::merge_group_fields(GroupType* gtype,
						 ReplaceMap *typemap,
						 SymbolTable *symtab)
{
  for (Iter<SymbolTableObject*> it = gtype->get_group_symbol_table()->
	 get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    FieldSymbol* oldfield = to<FieldSymbol>(it.current());
    Type *newtype = merge_type(oldfield->get_type(), typemap, symtab);
    oldfield->set_type(to<QualifiedType>(newtype));
  }
  return gtype;
}
  


GroupType* SuifLinker::merge_group(GroupType *oldtype,
				   ReplaceMap *typemap,
				   SymbolTable *symtab)
{
  Type *image = TypeHelper::find_isomorphic_type(oldtype, symtab);
  if (image != NULL) {
    typemap->add_entry(oldtype, image);
    return to<GroupType>(image);
  }
  if (oldtype->get_is_complete()  && oldtype->get_name().length()>0) {
    GroupType *gimage = find_incomplete_group_type(oldtype, symtab);
    if (gimage != NULL) {
      merge_group_fields(gimage, typemap, symtab);
      symtab->remove_symbol(gimage);
      typemap->add_entry(gimage, oldtype);
      add_type_to_table(symtab, oldtype);
      return oldtype;
    }
  }
  add_type_to_table(symtab, oldtype);
  return oldtype;
}


EnumeratedType* SuifLinker::merge_enumerated(EnumeratedType *oldtype,
					     ReplaceMap *typemap,
					     SymbolTable *symtab)
{
  Type *image = TypeHelper::find_isomorphic_type(oldtype, symtab);
  if (image != NULL) {
    typemap->add_entry(oldtype, image);
    return to<EnumeratedType>(image);
  }
  add_type_to_table(symtab, oldtype);
  return oldtype;
}



Type* SuifLinker::merge_subtype(Type *oldtype,
				ReplaceMap *typemap,
				SymbolTable *symtab)
{
  if (is_kind_of<QualifiedType>(oldtype))
    return merge_qualified(to<QualifiedType>(oldtype), typemap, symtab);
  else if (is_kind_of<PointerType>(oldtype))
    return merge_pointer(to<PointerType>(oldtype), typemap, symtab);
  else if (is_kind_of<ArrayType>(oldtype))
    return merge_array(to<ArrayType>(oldtype), typemap, symtab);
  else if (is_kind_of<CProcedureType>(oldtype))
    return merge_cprocedure(to<CProcedureType>(oldtype), typemap, symtab);
  else if (is_kind_of<GroupType>(oldtype))
    return merge_group(to<GroupType>(oldtype), typemap, symtab);
  else if (is_kind_of<EnumeratedType>(oldtype))
    return merge_enumerated(to<EnumeratedType>(oldtype), typemap, symtab);
  else if (is_a<IntegerType>(oldtype) ||
	   is_a<FloatingPointType>(oldtype) ||
	   is_a<BooleanType>(oldtype) ||
	   is_a<VoidType>(oldtype) ||
	   is_a<LabelType>(oldtype)) {
     Type *newtype = TypeHelper::find_isomorphic_type(oldtype, symtab);
     if (newtype != NULL) {
       typemap->add_entry(oldtype, newtype);
       return newtype;
     } else {
       add_type_to_table(symtab, oldtype);
       return oldtype;
     }
  } else {
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, String("Unknown type ") +
				to_id_string(oldtype)));
    return 0;
  }
}
 





// If oldtype is mapped to a new type in typemap, return the new type.
// Otherwise, if an equivalent (isomorphically) type exists in symtab, enter
//   a new entry in typemap and return the type found in symtab.
// Otherwise, enter oldtype into symtab and return oldtype.
//
// Return a type (could be the same as oldtype) instance that exists in
//   symtab.
// Modify symtab  new entries may be added
//        typemap new entries may be added
//
// If <oldtype,newtype> exist in typemap, then newtype already exists in master
// and every occurence of oldtype should be replaced with newtype.
// If a type instance T exists in symtab, then all components of T of type 
//  Type, would be in symtab also.
//
// precondition: oldtype has no parent
// postcondition: return type object has parent \a symtab
//
Type* SuifLinker::merge_type(Type *oldtype,
			     ReplaceMap *typemap,
			     SymbolTable *symtab)
{
  Type *newtype = typemap->map_type(oldtype);
  if (newtype != NULL) return newtype;
  newtype = merge_subtype(oldtype, typemap, symtab);
  return newtype;
}








// Move the symbol table objects from slave table to the master table
// Make sure that the types are not duplicated in the master table.
// Modify typemap  append <oldtype, newtype> pair for any replacement
//                 of oldtype from slave table to newtype in master table
// Modify master   all symbols are transferred from slave to master and
//                 all non-duplicated type definitions are transferred too.
//                 Duplicated types will be appended as <oldtype, newtype>
//                 pair to typemap.
//
void SuifLinker::merge_symtab(SymbolTable *master,
			      SymbolTable *slave,
			      ReplaceMap *typemap)
{
  suif_vector<SymbolTableObject*> lsym;
  for (Iter<SymbolTableObject*> it = slave->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    SymbolTableObject *symobj = it.current();
    /* This shouldn't happen, but occasionally the front-end generates
     * NULL entries into symbol tables.
     */
    if (symobj != 0) lsym.push_back(symobj);
  }
  for (unsigned i=0; i<lsym.size(); i++) {
    SymbolTableObject *symobj = lsym[i];
    slave->remove_symbol(symobj);
    if (is_kind_of<Type>(symobj))
      merge_type(to<Type>(symobj), typemap, master);
    else if (is_kind_of<Symbol>(symobj)) {
      master->add_symbol(symobj);
    } else {
      SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				  String("Illegal object ") +
				  to_id_string(symobj) + " in symbol table " +
				  to_id_string(slave)));
    }
  }
  for (unsigned i=0; i<lsym.size(); i++) {
    if (lsym[i]->get_parent() == NULL)
      add_garbage(lsym[i]);
  }
}


// Merge (remove and append components from) the symbol tables
// and the file blocks from slave to master.
// Modify: both slave and master.
//
void SuifLinker::merge_fsb(FileSetBlock *slave)
{
  if (_master_fsb->get_file_set_symbol_table()->
      get_symbol_table_object_count() > 0)
    SUIF_THROW(SuifDevException(__FILE__, __LINE__,
				String("non empty file_set_symbol_table in ") +
				to_id_string(_master_fsb)));
  if (slave->get_file_set_symbol_table()->get_symbol_table_object_count() > 0)
    SUIF_THROW(SuifDevException(__FILE__, __LINE__, 
				String("non empty file_set_symbol_table in ") +
				to_id_string(slave)));
  merge_symtab(_master_fsb->get_external_symbol_table(),
	       slave->get_external_symbol_table(),
	       &_type_map);
  while (slave->get_file_block_count() > 0) {
    _master_fsb->append_file_block(slave->remove_file_block(0));
  }
}
