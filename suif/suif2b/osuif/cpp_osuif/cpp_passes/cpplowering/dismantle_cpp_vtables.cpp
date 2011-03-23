#include "common/system_specific.h"
/* FILE dismantle_cpp_vtables.cpp */

/*
       Copyright (c) 1999 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.

*/  

#include "common/suif_copyright.h"

#include "iokernel/cast.h"
#include "suifkernel/utilities.h"
#include "common/i_integer.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "cfenodes/cfe.h"
#include "cfenodes/cfe_factory.h"
#include "basicnodes/basic_constants.h"
#include "suifkernel/suifkernel_messages.h"
#include "typebuilder/type_builder.h"
#include "cpp_osuifnodes/cpp_osuif.h"
#include "cpp_osuifnodes/cpp_osuif_factory.h"
#include "utils/value_block_utils.h"
#include "utils/symbol_utils.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "common/suif_vector.h"
#include "common/lstring.h"

#include "dismantle_cpp_vtables.h"

static LString next_field_name() {
    static char field_name[20] = "f0";
    int i = 0;
    while (field_name[i]) i++;
    i --;
    while (field_name[i] == '9')
	i--;
    if (field_name[i] != 'f') {
	field_name[i] ++;
	return field_name;
	}
    i ++;
    field_name[i] = '1';
    i++;
    while (field_name[i]) {
	field_name[i] = '0';
	i++;
	}
    field_name[i] = '0';
    i++;
    field_name[i] = 0;
    return field_name;
    }

DismantleCppVTablesPass::DismantleCppVTablesPass(SuifEnv *the_env,
                             const LString &name) :
  Pass(the_env, name),_pointer_sized_int(0) {
    tb = (TypeBuilder *)the_env->get_object_factory(TypeBuilder::get_class_name());
    }

Module *DismantleCppVTablesPass::clone() const {
    return((Module*)this);
    }

void DismantleCppVTablesPass::add_vtable_pointer_set(
	CppInstanceMethodSymbol *cim,ParameterSymbol *this_sym) {
    SuifEnv *env = get_suif_env();
    ProcedureDefinition *def = cim->get_definition();
    if (!def)
        return;

    Statement* body = to<Statement>(def->get_body());

    CppClassType *cct = to<CppClassType>(cim->get_owning_class());
    
    VariableSymbol *vtable_sym = cct->get_vtable_sym();

    // Start by generating the address into a temp

    PointerType *pt = tb->get_pointer_type(vtable_sym->get_type());

    VariableSymbol *temp = new_anonymous_variable(env,body,
		tb->get_qualified_type(pt));

    SymbolAddressExpression *lsae = create_symbol_address_expression(
		env,pt,vtable_sym);
    StoreVariableStatement *st = create_store_variable_statement(env,temp,lsae);

    insert_statement_before(body,st);

    // Now store this into the principal vtable
    FieldSymbol *vtable_field = cct->get_vtable_field();

    LoadVariableExpression  *load_par = create_var_use(this_sym);
    Expression *fae = create_field_access_expression(env,pt,load_par,vtable_field);
    Statement *fst = create_store_statement(env,fae,create_var_use(temp));
    insert_statement_after(fst,st);

    // There may be additional places where we need to store if there is
    // multiple inheritance involved. The vtable ptr for the first parent is
    // shared, so this is in place

    // TODO finish this code

    bool first = true;
    for (Iter<InheritanceLink*> piter = cct->get_parent_classe_iterator();
            piter.is_valid();piter.next()) {
        InheritanceLink* il = piter.current();
        CppClassType *p = to<CppClassType>(il->get_parent_class_type());
        if (p->get_vtable_type() && !first) {
	    }
	else {
	    first = false;
	    }
	}
    }

struct shortcuts {
    LString type_name;
    int size;
    char *shortcut;
    };

shortcuts shortforms[] = {
	{"char",sizeof(char)*BITSPERBYTE,"c"},
	{"short",sizeof(short)*BITSPERBYTE,"s"},
	{"int",sizeof(int)*BITSPERBYTE,"i"},
	{"long",sizeof(long)*BITSPERBYTE,"l"},
	{"float",sizeof(float)*BITSPERBYTE,"f"},
	{"double",sizeof(double)*BITSPERBYTE,"d"},
	{"bool",sizeof(bool)*BITSPERBYTE,"b"},
	{"",65535,0}};

static String mangle_parameters(CProcedureType *type,int start=0) {
    String mn;
    bool in_basic;
    
    for (int i=start;i < type->get_argument_count(); i++) {
	Type *arg_type = unqualify_type(type->get_argument(i));
	// The following is C specific
 	String tn = arg_type->get_name();
	int j = -1;
	if (arg_type->get_name() == emptyLString) {
	    if (is_kind_of<IntegerType>(arg_type)) {
		int size = to<IntegerType>(arg_type)->get_bit_size().c_int();

		j=0;
		//	This code make assumptions about the sizes of 
		//	various fields. This is not great
		if (!to<IntegerType>(arg_type)->get_is_signed())
		    mn += "U";
		while ((j < 3) && (size > shortforms[j].size))
		    j++;
		}
	    else if (is_kind_of<FloatingPointType>(arg_type)) {
		int size = to<FloatingPointType>(arg_type)->get_bit_size().c_int();
		j = 4;
		if (size != shortforms[j].size)
		    j++;
		}
	    }
        else {
	    j = 0;
	    if (tn.starts_with("unsigned")) {
	        tn = tn.Right(-9);
	        mn += 'U';
	        in_basic = true;
	        while ((shortforms[j].shortcut != 0) && (shortforms[j].type_name != tn))
	            j ++;
	        }
	    else {
		
	         LString tn = arg_type->get_name();
	         while ((shortforms[j].shortcut != 0) && (shortforms[j].type_name != tn))
                    j ++;
		}
	    if (shortforms[j].shortcut == 0)
		j=-1;
	    }
	if (j >= 0) {
	    if (!in_basic)
		mn += "F";
	    in_basic = true;
	    mn += shortforms[j].shortcut;
	    }
	else {
	    if (in_basic) 
		mn += "P";
	    in_basic= false;
	    mn += arg_type->get_name().length();
	    mn += arg_type->get_name();
	    }
	}
    return mn;
    }

static String mangle_name(ProcedureSymbol *sym) {
    return sym->get_name() + mangle_parameters(to<CProcedureType>(sym->get_type()));
    }

static String mangle_name(InstanceMethodSymbol *sym) {
    return sym->get_name() + "__" + sym->get_owning_class()->get_name() + mangle_parameters(to<CProcedureType>(sym->get_type()),1);
    }

//    Find entry for symbol
// There are two possibilities. Either this symbol is in a parent type, or it is in
// the most derived type. 
// If it is in the most derived type, its slot number is
// the slot number in symbol. We can go directly to the symbol using the index
// If it is a parent symbol, this slot number is the
// slot number in the parent symbol table. It may not correspond to the index in this 
// table. In that case, we search for the symbol starting at the end of the vtable

static int find_vtable_entry_index(CppVTableType *table,CppInstanceMethodSymbol *cim) {
    int index = cim->get_vtable_slot_no();
    CppVTableEntry *entry = table->get_table_entry(index);
    for (int i=0;i < entry->get_symbol_count();i++) {
        if (entry->get_symbol(i) == cim) {
          return index;
          }
      }
    index = table->get_table_entry_count();
    while (index > 0) {
      index --;
      CppVTableEntry *entry = table->get_table_entry(index);
        for (int i=0;i < entry->get_symbol_count();i++) {
            if (entry->get_symbol(i) == cim) {
                return index;
              }
            }
        }
    suif_assert_message(false,("Vtable entry not found"));
    return -1;
    }



/**	Add this parameter to a method and lower it into a function
 */
void DismantleCppVTablesPass::dismantle_method(
		CppInstanceMethodSymbol *cim) {
    SuifEnv *env = get_suif_env();
    ClassType *ct = cim->get_owning_class();
    CppClassType *cct = to<CppClassType>(ct);
    SymbolTable *symtab = to<SymbolTable>(cct->get_parent());

    // create a method in the symbol table of the class type

    String fname = mangle_name(cim);

    CProcedureType *type = to<CProcedureType>(cim->get_type());
    list<QualifiedType *> argument_list;

#ifdef METHODS_NEED_ADDED_THIS
    QualifiedType *this_type = tb->get_qualified_type(tb->get_pointer_type(cct));
    argument_list.push_back(this_type);
#endif
    for (int i=0;i < type->get_argument_count(); i++) {
        argument_list.push_back(type->get_argument(i));
	}

    CProcedureType *new_type = tb->get_c_procedure_type(
		type->get_result_type(),
		argument_list,
		type->get_has_varargs(),
		type->get_arguments_known(),
		type->get_bit_alignment());
    ProcedureSymbol *new_sym= create_procedure_symbol(env,new_type,fname,true);
    symtab->append_symbol_table_object(new_sym);

    // remove the old method type from the symbol table containing it

    if (type->get_parent()) {
	to<SymbolTable>(type->get_parent())->remove_symbol_table_object(type);
	type->set_parent(0);
	}

    // if this method is virtual, then the vtable has a field which will point at this
    // function. Its type needs updating

    if (cim->get_is_dispatched()) {
	CppVTableType *vtable = cct->get_vtable_type();
	int index = find_vtable_entry_index(vtable,cim);
        GroupSymbolTable *gst = vtable->get_group_symbol_table();
        FieldSymbol *func = to<FieldSymbol>(gst->get_symbol_table_object(2*index+1));
	func->set_type(tb->get_qualified_type(tb->get_pointer_type(new_type)));
	}

    _lowered_symbols->enter_value(cim,new_sym);

    // now, if we have a body, create a definition and put that in the new symbol

    ProcedureDefinition *def = cim->get_definition();
    if (!def)
	return;

    DefinitionBlock *def_db = to<DefinitionBlock>(def->get_parent());
    def_db->remove_procedure_definition(def);
	
    Statement* body = to<Statement>(def->get_body());
    if (!body)
        return;

    body->set_parent(0);
    cim->set_definition(0); // only copy body once

    SymbolTable *pst = def->get_symbol_table();
    pst->set_parent(0);

    DefinitionBlock *pdb = def->get_definition_block();
    pdb->set_parent(0);

    ProcedureDefinition *new_def = create_procedure_definition(env,new_sym,body,pst,pdb);
    // need to put this into the definition block corresponding to symtab
    DefinitionBlock *symtab_def = get_corresponding_definition_block(symtab);
    symtab_def->append_procedure_definition(new_def);
    
    // need to trash def here

    // now move across the parameters from the old definition, but first we may need 
    // a new "this" parameter

    ParameterSymbol *this_par;
#ifdef METHODS_NEED_ADDED_THIS
    this_par = create_parameter_symbol(env,this_type,"this");
    pst->append_symbol_table_object(this_par);
    new_def->append_formal_parameter(this_par);
#else
    this_par = def->get_formal_parameter(0);
#endif

    for (int pno = 0; pno < def->get_formal_parameter_count(); pno ++) {
	ParameterSymbol *par = def->get_formal_parameter(pno);
	// par->set_parent(0);
	new_def->append_formal_parameter(par);
	// pst->append_symbol_table_object(par);
	}
    if (cim->is_constructor() || cim->is_destructor())
        add_vtable_pointer_set(cim,this_par);
    to<SymbolTable>(cim->get_parent())->remove_symbol_table_object(cim);
    }

void DismantleCppVTablesPass::lower_instance_method_call(
	InstanceMethodCallExpression *imce,CppInstanceMethodSymbol *ims) {
    // convert from an InstanceMethodCallExpression to a simple CallExpression
    SuifEnv *env = get_suif_env();

    // the method may be dispatching or non-dispatching. If it is non-dispatching
    // we do a simple replace. If it is dispatching we must do rather more work

    CallExpression *exp;

    if (!ims->get_is_dispatched()) {

    	Expression * callee_address = imce->get_callee_address();
    	callee_address->set_parent(0);

    	exp = create_call_expression(env,
		imce->get_result_type(),
		callee_address);
    	for (int i = 0;i < imce->get_argument_count();i++) {
	    Expression *arg = imce->get_argument(i);
	    arg->set_parent(0);
	    exp->append_argument(arg);
	    }
	}
    else {  // this is the dispatched case. The first parameter is a class
	    // variable (the this) which points at an object containing a
 	    // pointer to a vtable. The address of this pointer is the same
	    // as that in the class type to which the method belongs, so
	    // we don't have to identify the actual type (indeed, in general,
	    // that is dynamic)

	// step 1 - develop an expression for the address of the vtable
	Expression *this_par = imce->get_argument(0);
	CppClassType *cct = to<CppClassType>(ims->get_owning_class());
	FieldSymbol *vtable_ptr_field = cct->get_vtable_field();
        VariableSymbol *vtable_sym = cct->get_vtable_sym();

	CppVTableType *vtable_type = to<CppVTableType>(get_data_type(vtable_sym));
        PointerType *pt = tb->get_pointer_type(vtable_type);

	FieldAccessExpression *fae = create_field_access_expression(
		env,
		tb->get_pointer_type(pt),
		to<Expression>(this_par->deep_clone()),
		vtable_ptr_field);
	LoadExpression *vtable_load = create_load_expression(
		env,
		pt,
		fae);
	int index = find_vtable_entry_index(vtable_type,ims);
        // We could index from the start of the vtable but actually we have
        // fields defined for all the entries, so we can do better
    	// We need to do the adjustment on a char array
	IntegerType *char_type = tb->get_smallest_integer_type();
	ArrayType *char_array = tb->get_array_type(
		tb->get_qualified_type(char_type),0,0);
	PointerType *char_array_ptr = tb->get_pointer_type(char_array);
 	GroupSymbolTable *gst = vtable_type->get_group_symbol_table();
	FieldSymbol *adjust = to<FieldSymbol>(gst->get_symbol_table_object(2*index));
	FieldSymbol *func = to<FieldSymbol>(gst->get_symbol_table_object(2*index+1));
	FieldAccessExpression *adjust_fae = create_field_access_expression(
                env,
                tb->get_pointer_type(adjust->get_type()),
                to<Expression>(vtable_load->deep_clone()),
                adjust);

	Expression *adjusted_this =
		create_array_reference_expression(env,
			tb->get_pointer_type(cct),
			create_unary_expression(env,char_array_ptr,k_convert,
				to<Expression>(this_par->deep_clone())),
			create_load_expression(
			    env,unqualify_data_type(adjust->get_type()),adjust_fae));
 	
	//	Now we must develop the address of the function

        FieldAccessExpression *func_fae = create_field_access_expression(
                env,
                tb->get_pointer_type(func->get_type()),
		vtable_load,func);
	 LoadExpression *callee_address = create_load_expression(
                env,
                get_data_type(func),
                func_fae);
	//	substitute the first parameter and the call expression
	exp = create_call_expression(env,
                imce->get_result_type(),
                callee_address);

	exp->append_argument(adjusted_this);
	for (int i = 1;i < imce->get_argument_count();i++) {
            Expression *arg = imce->get_argument(i);
            arg->set_parent(0);
            exp->append_argument(arg);
            }
	// phew
	}
    imce->get_parent()->replace(imce,exp);
    }

void DismantleCppVTablesPass::do_file_set_block( FileSetBlock* fsb) {
    // Now find all constructors and destructors and add updates of the vtable pointers
    // in the correct places

    _lowered_symbols = new suif_hash_map<InstanceMethodSymbol *,ProcedureSymbol *>;


    SuifEnv *env = get_suif_env();

    _pointer_sized_int = tb->get_integer_type(
              BITSPERBYTE*sizeof(void *),
                BITSPERBYTE*sizeof(void *),true);



    list<CppInstanceMethodSymbol*> *m = collect_objects<CppInstanceMethodSymbol>(fsb);
    for (list<CppInstanceMethodSymbol*>::iterator iter = m->begin();
                iter != m->end(); iter++) {
        CppInstanceMethodSymbol *cim = *iter;
	dismantle_method(cim);
        }
    delete m;

    // We now need to find every reference to a method and replace it by a reference
    // to the lowered routine. Some of these will be instance call expressions, which
    // we must also lower to procedure calls
    list<SymbolAddressExpression*>*exp = collect_objects<SymbolAddressExpression>(fsb);
    for (list<SymbolAddressExpression*>::iterator eiter = exp->begin();
                eiter != exp->end(); eiter++) {
	SymbolAddressExpression *sae = *eiter;

	Symbol *sym = sae->get_addressed_symbol();

	if (is_kind_of<CppInstanceMethodSymbol>(sym)) {
	    suif_hash_map<InstanceMethodSymbol *,ProcedureSymbol *>::iterator iter =
		_lowered_symbols->find(to<CppInstanceMethodSymbol>(sym));
	    if (iter != _lowered_symbols->end()) {
  		sae->set_addressed_symbol((*iter).second);
		SuifObject *parent = sae->get_parent();
		if (is_kind_of<InstanceMethodCallExpression>(parent)) {
		    InstanceMethodCallExpression *cim = to<InstanceMethodCallExpression>(parent);
        	    lower_instance_method_call(cim,to<CppInstanceMethodSymbol>(sym));
		    }
		}
	    }
        }
    delete exp;

    delete _lowered_symbols;

    // There are sundry converts in the code (nodes of types CppBaseClassConvert and
    // CppDerivedClassConvert: both derived from CppClassConvert) which require
    // "this" pointer adjustments when performed.

    list<CppClassConvert*>*ccc = collect_objects<CppClassConvert>(fsb);

    IntegerType *char_type = tb->get_smallest_integer_type();
    ArrayType *char_array = tb->get_array_type(tb->get_qualified_type(char_type),0,0);
    PointerType *char_array_ptr = tb->get_pointer_type(char_array);

    for (list<CppClassConvert*>::iterator citer = ccc->begin();
                citer != ccc->end(); citer++) {
        CppClassConvert *cce = *citer;
        Expression *sub_exp = cce->get_source();
        PointerType *cct_ptr = to<PointerType>(sub_exp->get_result_type());
        CppClassType *cct = to<CppClassType>(tb->unqualify_type(cct_ptr->get_reference_type()));
        PointerType *target_ptr = to<PointerType>(cce->get_result_type());
        CppClassType *target = to<CppClassType>(tb->unqualify_type(target_ptr->get_reference_type()));

	if (is_kind_of<CppBaseClassConvert>(cce)) {
	    CppBaseClassConvert *cbe = to<CppBaseClassConvert>(cce);

	    // What we have is a pointer to an object. We need to find the 
	    // vtable for the object we are converting to inside the object
	    // we are converting from. The first entry of that table is the
	    // offset from the start of the parent object to the object we
	    // have in hand (ie, it is negative - we have to subtract it to
	    // do the adjustment)

	    
  	    // The class we are casting to (the target) is a direct base of the
	    // class we are casting from. Find the associated object
	    // CAREFUL EASY OUCH!
	    // We cannot just do target->get_object_lattice() because that 
	    // points at the object that was built with the target class as
	    // the parent - it is different to the object built with cct as the
	    // parent
	    CppObject *cct_object = cct->get_object_lattice();
	    CppObject *target_object = 0;
	    int i =0;
	    while (i < cct_object->get_sub_object_count()) {
		CppObject *sub = cct_object->get_sub_object(i);
		if (sub->get_type() == target) {
		    target_object = sub;
		    break;
		    }
		i ++;
		}

	    // It also seems to give us these with casts to virtual (non-direct)
	    // bases, so look for one of those too

	    i = 0;
            while (i < cct_object->get_sub_object_closure_count()) {
                CppObject *sub = cct_object->get_sub_object_closure(i);
                if (sub->get_is_virtual() && (sub->get_type() == target)) {
                    target_object = sub;
                    break;
                    }
                i ++;
                }

	    suif_assert_message(target_object != 0,("Missing subobject"));

	    // OK, now we have the target object. Its offset is contained in the
	    // vtable for the parent object. If it is not virtual, we know this
	    // value statically. Otherwise, we must use the vtable value.

	    if (target_object->get_is_virtual()) {
	    	int index = target_object->get_vtable_first_slot();
        	FieldSymbol *vtable_ptr_field = cct->get_vtable_field();
        	VariableSymbol *vtable_sym = cct->get_vtable_sym();

        	CppVTableType *vtable_type = to<CppVTableType>(get_data_type(vtable_sym));
        	PointerType *pt = tb->get_pointer_type(vtable_type);

       	 	FieldAccessExpression *fae = create_field_access_expression(
                	env,
                	tb->get_pointer_type(pt),
                	to<Expression>(sub_exp->deep_clone()),
                	vtable_ptr_field);
        	LoadExpression *vtable_load = create_load_expression(
                	env,
                	pt,
                	fae);

        	GroupSymbolTable *gst = vtable_type->get_group_symbol_table();
        	FieldSymbol *adjust = to<FieldSymbol>(gst->get_symbol_table_object(2*index));
		sub_exp->set_parent(0);
        	FieldAccessExpression *adjust_fae = create_field_access_expression(
                	env,
                	tb->get_pointer_type(adjust->get_type()),
			vtable_load,adjust);

        	Expression *adjusted_this =
                	create_array_reference_expression(env,
                        	cct_ptr,
                        	create_unary_expression(env,char_array_ptr,k_convert,
                                to<Expression>(sub_exp->deep_clone())),
                        create_load_expression(
                            env,unqualify_data_type(adjust->get_type()),adjust_fae));
		cbe->get_parent()->replace(cbe,adjusted_this);
		}
	    else {
		IInteger offset = target_object->get_offset();
		sub_exp->set_parent(0);
        	Expression *adjusted_this =
                	create_array_reference_expression(env,
                        	target_ptr,
                        	create_unary_expression(env,char_array_ptr,k_convert,sub_exp),
				create_int_constant(env,_pointer_sized_int,offset));
		cbe->get_parent()->replace(cbe,adjusted_this);
		}
	    }
	else {
	    // perform a cast from a base class to a more derived class.
	    // The first entry of the vtable should do this.
            int index = 0;
            FieldSymbol *vtable_ptr_field = cct->get_vtable_field();
            VariableSymbol *vtable_sym = cct->get_vtable_sym();

            CppVTableType *vtable_type = to<CppVTableType>(get_data_type(vtable_sym));
            PointerType *pt = tb->get_pointer_type(vtable_type);

            FieldAccessExpression *fae = create_field_access_expression(
                        env,
                        tb->get_pointer_type(pt),
                        to<Expression>(sub_exp->deep_clone()),
                        vtable_ptr_field);
            LoadExpression *vtable_load = create_load_expression(
                        env,
                        pt,
                        fae);

            GroupSymbolTable *gst = vtable_type->get_group_symbol_table();
            FieldSymbol *adjust = to<FieldSymbol>(gst->get_symbol_table_object(2*index));
            sub_exp->set_parent(0);
            FieldAccessExpression *adjust_fae = create_field_access_expression(
                        env,
                        tb->get_pointer_type(adjust->get_type()),
                        vtable_load,adjust);

            Expression *adjusted_this =
                        create_array_reference_expression(env,
                                cct_ptr,
                                create_unary_expression(env,char_array_ptr,k_convert,
                                to<Expression>(sub_exp->deep_clone())),
                        create_load_expression(
                            env,unqualify_data_type(adjust->get_type()),adjust_fae));
            cce->get_parent()->replace(cce,adjusted_this);
	    }
	}


    }

BuildCppVTablesPass::BuildCppVTablesPass(SuifEnv *the_env,
                             const LString &name) :
   Pass(the_env, name) ,_pointer_sized_int(0){
	tb = (TypeBuilder *)the_env->get_object_factory(TypeBuilder::get_class_name());
	}

Module *BuildCppVTablesPass::clone() const {
    return((Module*)this);
    }   

#if 0
void BuildCppVTablesPass::add_local_elements(CppClassType *cct) {
    CppVTableType * vtable = cct->get_vtable_type();
    InstanceMethodSymbolTable * methods_table = cct->get_instance_method_symbol_table();
    Iter<SymbolTableObject*> iter = methods_table->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
	CppInstanceMethodSymbol *method = to<CppInstanceMethodSymbol>(iter.current());
	if (method->get_is_dispatched()) {
	    FieldSymbol *sym = tb->add_symbol_to_group(vtable,next_field_name(),
				tb->get_qualified_type(
				    tb->get_pointer_type(method->get_type())));
	    CppVTableEntry* c = create_cpp_v_table_entry(get_suif_env(),0);
	    c->append_symbol(sym);
	    vtable->append_table_entry(c);
	    }
	iter.next();
	}
    }
#endif

static bool is_dispatched(CppClassType *cct) {
    InstanceMethodSymbolTable * methods_table = cct->get_instance_method_symbol_table();
    Iter<SymbolTableObject*> iter = methods_table->get_symbol_table_object_iterator();
    while (iter.is_valid()) {
        InstanceMethodSymbol *method = to<InstanceMethodSymbol>(iter.current());
        if (method->get_is_dispatched()) {
	    return true;
	    }
	iter.next();
	}

    for (Iter<InheritanceLink*> piter = cct->get_parent_classe_iterator();
            piter.is_valid();piter.next()) {
        InheritanceLink* il = piter.current();
        CppClassType *p = to<CppClassType>(il->get_parent_class_type());
	if (is_dispatched(p))
	    return true;
	}
    return false;
    }

void BuildCppVTablesPass::append_constant(SuifEnv *env,MultiValueBlock *mvb,GroupType *stype,
			    DataType *size_type,int value) {
    ExpressionValueBlock *evb = create_expression_value_block(
                env,create_int_constant(env,size_type,value));
    append_to_multi_value_block(mvb,evb);
    tb->add_symbol_to_group(stype,next_field_name(),
                        tb->get_qualified_type(size_type));
    }


void BuildCppVTablesPass::append_symbol_address(SuifEnv *env,MultiValueBlock *mvb,GroupType *stype,Symbol *sym) {

    PointerType *pt = tb->get_pointer_type(sym->get_type());
    tb->add_symbol_to_group(stype,next_field_name(),
        tb->get_qualified_type(pt));
    ExpressionValueBlock *evb = create_expression_value_block(
                env,
                create_symbol_address_expression(
                        env,pt,sym));
    append_to_multi_value_block(mvb,evb);
    }

static bool get_adjustment(int& adjust,int current_displacement,CppClassType *cct,InstanceMethodSymbol *sym) {
    int offset = 0;
    CppClassType *owner = to<CppClassType>(sym->get_owning_class());
    if (owner == cct) {
	adjust = - current_displacement;
	return true;
	}

    for (Iter<InheritanceLink*> piter = cct->get_parent_classe_iterator();
            piter.is_valid();piter.next()) {
        InheritanceLink* il = piter.current();
        CppClassType *p = to<CppClassType>(il->get_parent_class_type());

	if (owner == p) {
	    adjust = offset - current_displacement;
	    return true;
	    }
	if(get_adjustment(adjust,current_displacement - offset,p,sym))
	    return true;
	offset += p->get_bit_size().c_int()/BITSPERBYTE;
	}
    return false;
    }


//	Build the object lattice for a class type. All its direct 
// 	base classes will already have valid object lists
CppObject * BuildCppVTablesPass::build_object_lattice( CppClassType *cct) {
    // The top object is the class itself

    SuifEnv *env = get_suif_env();
    CppObject * base_object = create_cpp_object(env,cct,false);
    
    for (Iter<InheritanceLink*> piter = cct->get_parent_classe_iterator();
            piter.is_valid();piter.next()) {
        CppInheritanceLink* il = to<CppInheritanceLink>(piter.current());
        CppClassType *p = to<CppClassType>(il->get_parent_class_type());
	CppObject * p_object = 0;
	IInteger offset;
	if (il->get_is_virtual()) {
	    suif_hash_map<CppClassType *,CppObject *>::iterator iter = 
		_virtual_objects->find(p);
	    if (iter == _virtual_objects->end()) {
	        offset = _parent->get_size_of_virtuals();
            	_parent->set_size_of_virtuals(offset + p->get_size_without_virtuals());
		p_object = build_object_lattice(p);
		_virtual_objects->enter_value(p,p_object);
		p_object->set_is_virtual(true);
		}
	    else {
		// it has already been entered - nothing to do
		}
	    }
	else {
	    offset = _parent->get_size_without_virtuals();
	    _parent->set_size_without_virtuals(offset + p->get_size_without_virtuals());
	    p_object = build_object_lattice(p);
	    }

	if (p_object) {
	    p_object->set_offset(offset);
            _parent->append_object(p_object);

	    base_object->append_sub_object(p_object);
	    if (!base_object->has_sub_object_closure_member(p_object)) {
	        base_object->append_sub_object_closure(p_object);
	        }
	    int soc_count = p_object->get_sub_object_closure_count();
  	    for (int soc=0;soc < soc_count;soc ++) {
	        CppObject * object = p_object->get_sub_object_closure(soc);
	        if (!base_object->has_sub_object_closure_member(object)) {
                    base_object->append_sub_object_closure(object);
		    }
		}
            }
	}
    return base_object;
    }

//	We use a method def to keep track of what object contains a given definition
class MethodDef {
	CppInstanceMethodSymbol * _method;
	bool _override;		// overrides a base class method
	CppObject *_object;	// object with which it is associated
	int _next_ambiguity;	// non-zero if this method is ambiguous
    public:
	CppInstanceMethodSymbol * get_method() {return _method;}
        bool get_override() {return _override;}
        CppObject *get_object() {return _object;}
        void set_method(CppInstanceMethodSymbol * method) {
		_method = method;}
        void set_override(bool override) {_override = override;}
        void set_object(CppObject *object) {
		_object = object;}
	MethodDef(CppInstanceMethodSymbol * method,CppObject *object) :
		_method(method),_override(false),_object(object),_next_ambiguity(-1) {}
	MethodDef() : _method(0),_override(false),_object(0),_next_ambiguity(-1) {}
	void set_next_ambiguity(int i) {_next_ambiguity = i;}
	int get_next_ambiguity() {return _next_ambiguity;}

        };

class MethodDefList : public suif_vector<MethodDef> {
    suif_vector<int>ambiguity_heads;
    suif_vector<int>ambiguity_tails;
    int _new_method_count;
    VariableSymbol *_rtti_variable;
    CppVTableType *_vtable;
    MultiValueBlock *_vtable_mvb;


    bool dominates(int i,int j) {
	CppObject *i_obj = (*this)[i].get_object();
	CppObject *j_obj = (*this)[j].get_object();
	return i_obj->has_sub_object_closure_member(j_obj);
	}
  public:

    int get_new_method_count() {return _new_method_count;}
    void set_new_method_count(int i) {_new_method_count = i;}
    int get_method_count() {return ambiguity_heads.size();}

    int find_override(InstanceMethodSymbol *method,CppObject *method_object) {
        for (unsigned int i = 0;i < size();i++) {
            InstanceMethodSymbol *candidate = (*this)[i].get_method();
            if (candidate->get_name() != method->get_name())
                continue;
            if (candidate->overrides(method))
                return i;
	    CppObject *c_object = (*this)[i].get_object();

            if (((c_object == method_object)
		 || c_object->has_sub_object_closure_member(method_object))
		&& method->is_equivalent(candidate)) {
                return i;
                }
            }
        return -1;
        }

    // add an ambiguity. old_index must be the index of the head of the ambiguity list
    void add_ambiguity(int old_index,CppInstanceMethodSymbol *new_sym,CppObject *new_obj) {
	int new_pos = size();
	push_back(MethodDef(new_sym,new_obj));
	unsigned int i = 0;
	while ((i < ambiguity_heads.size()) && (ambiguity_heads[i] != old_index)) 
	    i ++;
	if (i >= ambiguity_heads.size()) {
	    i = ambiguity_heads.size();
	    ambiguity_heads.push_back(old_index);
	    ambiguity_tails.push_back(new_pos);
	    (*this)[old_index].set_next_ambiguity(new_pos);
	    }
	else {
	    int j = ambiguity_tails[i];
	    (*this)[j].set_next_ambiguity(new_pos);
	    }
	ambiguity_tails[i] = new_pos;
	}

    // resolve the ambiguities. Leave the lists for any which cannot be resolved
    // because we store a more dominant entry into the first entry, this is the only
    // entry which can dominate the whole graph since dominance is transitive

    void resolve_ambiguities() {
	for (unsigned int i = 0;i < ambiguity_heads.size(); i++) {
	    int head = ambiguity_heads[i];
	    int next = (*this)[i].get_next_ambiguity();
	    while ((next >= 0) && dominates(head,next)) {
		next = (*this)[next].get_next_ambiguity();
		}
	    if (next < 0) {
		(*this)[head].set_next_ambiguity(-1);
		}
	    else { // unresolvable ambiguities exist - prune list
		next = head;
		int last = -1;
		while (next >= 0) {
		    int dom = head;
		    while ((dom >= 0) && ((dom == next) || !dominates(dom,next)))
			dom = (*this)[dom].get_next_ambiguity();
		    if (dom < 0) { // no dominator exists
			if (last < 0) {
			    ambiguity_heads[i] = next;
			    }
			else {
			    (*this)[last].set_next_ambiguity(next);
			    }
			last = next;
			ambiguity_tails[i] = next;
			}
		    next = (*this)[next].get_next_ambiguity();
		    }
		(*this)[ambiguity_tails[i]].set_next_ambiguity(-1);
		}
	    }
	}

    void enter_symbols(CppObject *object,bool top_level) {
	CppClassType *cct = object->get_type();
        InstanceMethodSymbolTable * methods_table = 
		cct->get_instance_method_symbol_table();
        Iter<SymbolTableObject*> iter = 
		methods_table->get_symbol_table_object_iterator();
        while (iter.is_valid()) {
            CppInstanceMethodSymbol *method = 
		to<CppInstanceMethodSymbol>(iter.current());
            if (method->get_is_dispatched()) {
		int i = -1;
		if (!top_level) {
		    i = find_override(method,object);
		    }
		if (i < 0) {
                    push_back(MethodDef(method,object));
		    }
                else if (i >= _new_method_count) {
                    // already entered from another parent object. One of these
                    // needs to dominate the other, or both need to be dominated
                    // in turn.
                    CppObject *old_obj = (*this)[i].get_object();
                    if (old_obj->has_sub_object_closure_member(object)) {
                         // the old one dominates the new one - do nothing
                         }
                    else if (object->has_sub_object_closure_member(old_obj)) {
                         // the new one dominates the old one. Keep the new 
                         // object as the table entry
                         (*this)[i].set_object(object);
                         (*this)[i].set_method(method);
                         }
                    else { 
			// none of the above. The symbol may be ambiguous
                        // we resolve these later
                        add_ambiguity(i,method,object);
                        }
                    }
                }
            iter.next();
	    }
        }
      void set_rtti_variable(VariableSymbol *rtti_variable) {
                _rtti_variable = rtti_variable;
                }

        VariableSymbol *get_rtti_variable() {
                return _rtti_variable;
                }

        void set_vtable(CppVTableType *vtable) {
                _vtable = vtable;
                }

        CppVTableType *get_vtable() {
                return _vtable;
                }

        void set_vtable_mvb(MultiValueBlock *vtable_mvb) {
                _vtable_mvb = vtable_mvb;
                }

        MultiValueBlock *get_vtable_mvb() {
                return _vtable_mvb;
                }


    };

CppObject *BuildCppVTablesPass::get_object_lattice(CppClassType *cct) {
    CppObject *obj = cct->get_object_lattice();
    if (!obj) {
        cct->set_size_of_virtuals(0);
        cct->set_size_without_virtuals(0);
        _virtual_objects = new suif_hash_map<CppClassType *,CppObject *>;
        _parent = cct;
        obj = build_object_lattice(cct);
        delete _virtual_objects;
        obj->set_offset(0);
        cct->append_object(obj);

        cct->set_object_lattice(obj);
        // Update the positions of the virtual objects by adding the
        // size of the non-virtuals
        IInteger nvs = cct->get_size_without_virtuals();
        // add in the total size of the fields
        nvs += cct->get_bit_size()/BITSPERBYTE;
        cct->set_size_without_virtuals(nvs);

        // The positions of the virtuals have all been based on zero. So, now we
        // must correct them
        for (int i = 0; i < cct->get_object_count();i++) {
            CppObject *obj = cct->get_object(i);
            if (obj->get_is_virtual())
                obj->set_offset(obj->get_offset() + nvs);
            }
	}
    return obj;
    }

VariableSymbol *BuildCppVTablesPass::get_rtti_symbol(CppClassType *cct) {
    SuifEnv *env = get_suif_env();
    VariableSymbol *rtti_var = cct->get_rtti_variable();
    if (!rtti_var) {
	FileSetBlock *fsb = env->get_file_set_block();
    	suif_assert_message(fsb->get_file_block_count() == 1,
		("File is ambiguous"));
    	FileBlock *fb = fsb->get_file_block(0);
        BasicSymbolTable *symtab = to<BasicSymbolTable>(fb->get_symbol_table());
    	GroupType *RTTI_struct = create_group_type(env,0,0);

    	symtab->append_symbol_table_object(RTTI_struct);
    	RTTI_struct->set_is_complete(true);

    	LString name = cct->get_name();

    	MultiValueBlock *RTTI_mvb = create_multi_value_block(env,RTTI_struct);

    	rtti_var = build_initialized_variable(
                env,String("__RTTI_")+name,RTTI_struct,RTTI_mvb);
	VariableSymbol *cn_var = build_string_constant_variable(env,
                (const char *)name);
	append_symbol_address(env,RTTI_mvb,RTTI_struct,cn_var);

	cct->set_rtti_variable(rtti_var);
   	// make sure we have the object list
	for (int i = 0; i < cct->get_object_count();i++) {
            CppObject *obj = cct->get_object(i);
            CppClassType *p = obj->get_type();

            VariableSymbol *p_var = get_rtti_symbol(p);
            append_symbol_address(env,RTTI_mvb,RTTI_struct,p_var);
	    }
        // finish the table with a zero
        append_constant(env,RTTI_mvb,RTTI_struct,_pointer_sized_int,0);
	}
    return rtti_var;
    }

void BuildCppVTablesPass::build_vtable(CppObject *object,MethodDefList &methods) {
    // The first entry in the table is the pointer to the RTTI info
    SuifEnv *env = get_suif_env();

    CppVTableType * vtable = methods.get_vtable();
    MultiValueBlock *vtable_mvb = methods.get_vtable_mvb();
    VariableSymbol *RTTI_variable = methods.get_rtti_variable();

    CppVTableEntry *cve = create_cpp_v_table_entry(env,0);
    cve->append_symbol(RTTI_variable);
    cve->append_object(object);
    vtable->append_table_entry(cve);
    append_constant(env,vtable_mvb,vtable,_pointer_sized_int,0);
    append_symbol_address(env,vtable_mvb,vtable,RTTI_variable);

    // Make sure vtables are built for all parent classes, and add them to
    // our vtable. A vtable looks like this:
    //
    //		0
    //		offset to RTTI info
    //		first parents "local" vtable
    //          second parents "local" vtable
    //          ...
    //  	new entries not found in any parent vtable
    //
    //		Each entry consists of an offset and a pointer, for functions,

    // Iterate through all the direct subobjects and build their contained 
    // vtables

    int bo_count = object->get_sub_object_count();
    for (int bo_index = 0; bo_index < bo_count; bo_index++) {    
        CppObject *bo = object->get_sub_object(bo_index);
	build_vtable(bo,methods);
	}

    // now build the methods for this entry

    // Now add the remaining fields to the vtable
    int new_method_count = methods.get_new_method_count();
    for (int m=0;m < new_method_count;m++) {
        // ***********************************************************************
        // TODO - We should be able to share the table entries with the parent
        // entry in most cases. Commented out line is not quite right - we need
        // to make sure the offsets match too
        // ***********************************************************************
        // if (!methods[m].get_override()) {
            CppInstanceMethodSymbol *method = methods[m].get_method();
            //FieldSymbol *sym = tb->add_symbol_to_group(vtable,next_field_name(),
            //                    tb->get_qualified_type(
            //                        tb->get_pointer_type(method->get_type())));
            CppVTableEntry* c = create_cpp_v_table_entry(env,0);
            c->append_symbol(method);
            c->append_object(methods[m].get_object());
            if(m < new_method_count)
                method->set_vtable_slot_no(vtable->get_table_entry_count());
            vtable->append_table_entry(c);
            CppObject *f_object = methods[m].get_object();
            IInteger i_delta = f_object->get_offset();
            int delta = i_delta.c_int();
            append_constant(env,vtable_mvb,vtable,_pointer_sized_int,delta);
            append_symbol_address(env,vtable_mvb,vtable,method);
            //}
        }

    CppClassType *cct = object->get_type();
    InstanceMethodSymbolTable * methods_table =
                cct->get_instance_method_symbol_table();
    Iter<SymbolTableObject*> iter =
                methods_table->get_symbol_table_object_iterator();

    // TODO - we are entering symbols which are overrides twice. Should
    // probably be shared between parent and derived class. The entry
    // in the parent slot will be correct for both if its offset is zero.
    while (iter.is_valid()) {
        CppInstanceMethodSymbol *method = to<CppInstanceMethodSymbol>(iter.current());
        if (method->get_is_dispatched()) {
            int i = methods.find_override(method,object);
	    suif_assert_message(i>=0,
				("method %s not found in methods table",
				 method->get_name().c_str()));
            CppVTableEntry* c = create_cpp_v_table_entry(env,0);
	    method = methods[i].get_method();
            c->append_symbol(method);
            c->append_object(methods[i].get_object());
            vtable->append_table_entry(c);
            CppObject *f_object = methods[i].get_object();
            IInteger i_delta = f_object->get_offset();
            int delta = i_delta.c_int();
	    if ((i < new_method_count) && (i_delta == 0))
		method->set_vtable_slot_no(vtable->get_table_entry_count());
            append_constant(env,vtable_mvb,vtable,_pointer_sized_int,delta);
            append_symbol_address(env,vtable_mvb,vtable,method);
            }
	iter.next();
	}
    }

void BuildCppVTablesPass::build_vtable(CppClassType *cct) {

    SuifEnv *env = get_suif_env();

    // build the object list if it does not exist

    CppObject *objects = get_object_lattice(cct);

    // if it is not dispatched, we are done

    if (!is_dispatched(cct) && (cct->get_size_of_virtuals() == 0))
        return; // no vtable needed

    MethodDefList methods;

    // We now build a table of most derived methods. 
    // Each object has a set of methods some of which are overridden. 
    //
    // We start with entries which are new overrides

    methods.enter_symbols(objects,true);
    methods.set_new_method_count(methods.get_method_count());

    // now scan the base direct objects for other methods. These methods may
    // be overridden by those above. In that case, we do not enter such
    // methods. We enter new methods. If a method appears more than once, it may be 
    // ambiguous - we need to check this and resolve it if possible


    int bo_count = objects->get_sub_object_count();
    int bo_index;
    for (bo_index = 0; bo_index < bo_count; bo_index++) { 
	CppObject *bo = objects->get_sub_object(bo_index);
	methods.enter_symbols(bo,false);
	}

    // We now have a complete list of virtual methods, with some potential ambiguities which
    // we now resolve. 

    methods.resolve_ambiguities();

    FieldSymbol *field = 0;

    // Create a group type to hold the RTTI information. This will
    // have the form
    // Pointer to class name
    // pointer to parent class RTTI info
    // more parent pointers
    // 0 (of size void *)

    VariableSymbol *RTTI_variable = get_rtti_symbol(cct);
    methods.set_rtti_variable(RTTI_variable);


    // Create a type for the vtable and a multivalue block into which 
    // values will be placed
    CppVTableType *vtable = create_cpp_v_table_type(env,0,0);
    vtable->set_is_complete(true);
    FileSetBlock *fsb = env->get_file_set_block();
    suif_assert_message(fsb->get_file_block_count() == 1,
                ("File is ambiguous"));
    FileBlock *fb = fsb->get_file_block(0);
    BasicSymbolTable *symtab = to<BasicSymbolTable>(fb->get_symbol_table());

    LString name = cct->get_name();

    symtab->add_symbol(String("__VTABLE_") + name + String("_type"),vtable);

    methods.set_vtable(vtable);

    MultiValueBlock *vtable_mvb = create_multi_value_block(env,vtable);
    VariableSymbol *vtable_var = build_initialized_variable(
		env,String("__VTABLE_")+name,vtable,vtable_mvb);

    methods.set_vtable_mvb(vtable_mvb);

    build_vtable(objects,methods);

    // Add the space for the vtable to the class type and set the pointer fields
    field = tb->add_symbol_to_group(cct,next_field_name(),tb->get_qualified_type(tb->get_pointer_type(vtable)));	
    cct->set_vtable_type(vtable);
    cct->set_vtable_sym(vtable_var);
    cct->set_vtable_field(field);
    }

void BuildCppVTablesPass::do_file_set_block( FileSetBlock* fsb) {

    // start by building the vtables and creating the fields in the classes for them

    _pointer_sized_int = tb->get_integer_type(
              BITSPERBYTE*sizeof(void *),
                BITSPERBYTE*sizeof(void *),true);


    list<CppClassType *> *l = collect_objects<CppClassType>(fsb);
    for (list<CppClassType*>::iterator iter = l->begin();
       		iter != l->end(); iter++) {
    	CppClassType *cct = *iter;
	if (!cct->get_vtable_type())
	    build_vtable(cct);
	}

    delete l;    
    }




