/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <string.h>

#include <basicnodes/basic_factory.h>
#include <typebuilder/type_builder.h>
#include <utils/trash_utils.h>
#include <utils/type_utils.h>
#include <utils/expression_utils.h>
#include <basicnodes/basic_factory.h>
#include <suifkernel/command_line_parsing.h>
#include <utils/node_builder.h>
#include <utils/cloning_utils.h>
#include <typebuilder/type_builder.h>
#include <suifkernel/utilities.h>

#include "source_lang_support.h"

const LString k_source_language("source_language");

const LString k_c_lang("c");
const LString k_fortran_lang("fortran");
const LString k_c_plus_plus_lang("c_plus_plus");
const LString k_java_lang("java");
const LString k_undef_lang("undef source lang");

LString lang_names[] = {
    k_c_lang,
    k_fortran_lang,
    k_c_plus_plus_lang,
    k_java_lang,
    k_undef_lang
};

void set_source_lang(ProcedureSymbol* proc_sym, LString lang){
    SuifEnv* suif_env = proc_sym->get_suif_env();
    if(proc_sym->take_annote(k_source_language)){
        suif_warning("%s already has a source language "
            "associated with it, trying to overwrite",
            proc_sym->get_name().c_str());
    }

    bool found = false;
    for(size_t i=0; i<sizeof(lang_names)/sizeof(const char*); i++){
        if(!strcmp(lang_names[i], lang)){
            found = true;
            break;
        }
    }

    suif_assert_message(found, ("Invalid source language %s", lang.c_str()));

    BrickAnnote* ba = create_brick_annote(suif_env, k_source_language);
    ba->append_brick(create_string_brick(suif_env, lang));
    proc_sym->append_annote(ba);
};

LString get_source_lang(ProcedureSymbol* proc_sym){
    BrickAnnote* ba = to<BrickAnnote>(
        proc_sym->peek_annote(k_source_language));

    if(ba){
        StringBrick* brick = to<StringBrick>(ba->get_brick(0));
        return LString(brick->get_value());
    }else{
        return k_undef_lang;
    }
};

void set_source_language(FileSetBlock* fsb, LString lang){
    list<ProcedureSymbol*>* to_be_set = 
        collect_objects<ProcedureSymbol>(fsb);

    {for (list<ProcedureSymbol*>::iterator iter = to_be_set->begin();
            iter != to_be_set->end(); iter++)
    {
        ProcedureSymbol* proc_sym= *iter;
        set_source_lang(proc_sym, lang);
    }}

    delete to_be_set;
};

// annotes for the fortran form
static const LString k_proc_in_fortran_form("proc_in_fortran_form");
static const LString k_fortran_form_artifact("fortran_form_artifact");
static const LString k_old_fortran_type("old_fortran_type");

void ConvertToFortranForm::initialize() {
  PipelinablePass::initialize();
  _command_line->set_description("Converts to Fortran form.");
}

void ConvertToFortranForm::do_procedure_definition(ProcedureDefinition *proc_def){
    SuifEnv* suif_env = proc_def->get_suif_env();
    NodeBuilder nb(proc_def->get_symbol_table());
    ProcedureSymbol* proc_sym = proc_def->get_procedure_symbol();

    {// put the fortran form annote on proc_sym
        if(proc_sym->peek_annote(k_proc_in_fortran_form)){
            suif_warning("%s is already in Fortran form, proceeding", 
                proc_sym->get_name().c_str());
            return;
        }else{
            proc_sym->append_annote(create_general_annote(
                suif_env, k_proc_in_fortran_form));
        }
    }
    
	{
	list<LoadExpression*>* to_be_converted = 
        collect_objects<LoadExpression>(proc_def);

	{for (list<LoadExpression*>::iterator iter = to_be_converted->begin();
            iter!=to_be_converted->end(); iter++)
    {
		LoadExpression* load = *iter;
		Expression* source_address = load->get_source_address();
		if(is_kind_of<LoadVariableExpression>(source_address)){
            LoadVariableExpression* load_var = 
                to<LoadVariableExpression>(source_address);
            VariableSymbol* var = load_var->get_source();
            fortranize_type(var);
            LoadVariableExpression* new_load_var = nb.load_var(var);

            new_load_var->append_annote(create_general_annote(
                suif_env, k_fortran_form_artifact));

			load->get_parent()->replace(load, new_load_var);
            trash_it(suif_env, load);
		}
	}}

	delete to_be_converted;
	}

	{
	list<StoreStatement*>* to_be_converted = 
        collect_objects<StoreStatement>(proc_def);

	{for (list<StoreStatement*>::iterator iter = to_be_converted->begin();
            iter!=to_be_converted->end(); iter++)
    {
		StoreStatement* store = *iter;
		Expression* destination_address = store->get_destination_address();
		Expression* value = store->get_value();
		if(is_kind_of<LoadVariableExpression>(destination_address)){
			LoadVariableExpression* load_var = to<LoadVariableExpression>(destination_address);

			remove_suif_object(load_var);
			store->set_destination_address(NULL);
			remove_suif_object(value);
			store->set_value(NULL);
			
			VariableSymbol* sym = load_var->get_source();
            fortranize_type(sym);
			// create a new StoreVariableStatement
			StoreVariableStatement* store_var = 
				nb.store_var(sym, value);
            // mark it with an annote
            store_var->append_annote(create_general_annote(
                    suif_env, k_fortran_form_artifact));
			// change the parent
			store->get_parent()->replace(store, store_var);
            trash_it(suif_env, store);
		}
	}}

	delete to_be_converted;
	}
};

void ConvertToFortranForm::fortranize_type(VariableSymbol* var){
    SuifEnv* suif_env = var->get_suif_env();
    TypeBuilder *tb = (TypeBuilder *)
         suif_env->get_object_factory(TypeBuilder::get_class_name());

    Type* new_type = tb->unqualify_type(var->get_type());
    
    if(is_kind_of<PointerType>(new_type)){
        Type* type = var->get_type();
        new_type = to<PointerType>(new_type)->get_reference_type();
        var->set_type(to<QualifiedType>(new_type));
        BrickAnnote* SBA = 
            create_brick_annote(suif_env, k_old_fortran_type);
        SBA->append_brick(create_suif_object_brick(suif_env, type));
        var->append_annote(SBA);
    }
};

void UnconvertFromFortranForm::initialize() {
    PipelinablePass::initialize();
    _command_line->set_description("Unconverts from Fortran form.");
};

void UnconvertFromFortranForm::do_procedure_definition(ProcedureDefinition *proc_def){
    NodeBuilder nb(proc_def->get_symbol_table());
    ProcedureSymbol* proc_sym = proc_def->get_procedure_symbol();

    {   // remove the fortran form annote from proc_sym
        if(!proc_sym->take_annote(k_proc_in_fortran_form)){
            suif_warning("%s is NOT in Fortran form, can't unconvert it, proceeding", 
                proc_sym->get_name().c_str());
            return;
        }
    }

	{
	list<LoadVariableExpression*>* to_be_unconverted = 
        collect_objects<LoadVariableExpression>(proc_def);

	{for (list<LoadVariableExpression*>::iterator iter = to_be_unconverted->begin();
            iter!=to_be_unconverted->end(); iter++)
    {
		LoadVariableExpression* load_var = *iter;
        // if one of the fortran form LVEs
		if(load_var->take_annote(k_fortran_form_artifact)){
            // note that take_annote should remove the annote
            VariableSymbol* var = load_var->get_source();
            unfortranize_type(var);
            // create new load
            LoadExpression* load = nb.load(nb.load_var(var));
            // replace the old load_var
            load_var->get_parent()->replace(load_var, load);
            trash_it(load_var);
		}
	}}

	delete to_be_unconverted;
	}

	{
	list<StoreVariableStatement*>* to_be_unconverted = 
        collect_objects<StoreVariableStatement>(proc_def);

	{for (list<StoreVariableStatement*>::iterator iter = to_be_unconverted->begin();
            iter!=to_be_unconverted->end(); iter++)
    {
		StoreVariableStatement* store_var = *iter;
        // if one of the fortran form SVSs
		if(store_var->take_annote(k_fortran_form_artifact)){
            VariableSymbol* dest = store_var->get_destination();
            Expression* value = store_var->get_value();

            // SuifObject* parent = store_var->get_parent();

            remove_suif_object(dest);
			store_var->set_destination(NULL);
			remove_suif_object(value);
			store_var->set_value(NULL);
            // adjust the type of dest
            unfortranize_type(dest);

			// create a new StoreStatement
			StoreStatement* store_st = 
                    nb.store(
                        nb.load_var(dest),
                        value);
            // replace store_var
            store_var->get_parent()->replace(store_var, store_st);
            trash_it(store_var);
		}
	}}

	delete to_be_unconverted;
	}
};

void UnconvertFromFortranForm::unfortranize_type(VariableSymbol* var){
    Type* type = var->get_type();
    // look at the annote and get the type from it
    BrickAnnote* ba = to<BrickAnnote>(
            var->take_annote(k_old_fortran_type));
    if(ba){
        SuifObjectBrick* type_brick = 
            to<SuifObjectBrick>(ba->get_brick(0));
        QualifiedType* old_type = to<QualifiedType>(type_brick->get_object());

        var->set_type(old_type);
        type->set_parent(NULL);
        trash_it(type);
    }
};
