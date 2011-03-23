#ifndef SOURCE_LANG_SUPPORT
#define SOURCE_LANG_SUPPORT

#include <suifpasses/suifpasses.h>
#include <suifkernel/utilities.h>
#include <common/lstring.h>

extern DLLIMPORT const LString k_source_language;

extern DLLIMPORT const LString k_c_lang;
extern DLLIMPORT const LString k_fortran_lang;
extern DLLIMPORT const LString k_c_plus_plus_lang;
extern DLLIMPORT const LString k_java_lang;
extern DLLIMPORT const LString k_undef_lang;

/**
    Set the source language for \a proc_sym to \a lang.
*/
void set_source_lang(ProcedureSymbol* proc_sym, LString lang);

/**
    Get the source language for \a proc_sym. If none is 
    recorded, k_undef_lang is returned.
*/
LString get_source_lang(ProcedureSymbol* proc_sym);

/**
    Set the language for all procedure symbols in \a fsb.
*/
void set_source_language(FileSetBlock* fsb, LString lang);

/**
    Convert the program into Fortran form.
*/
class ConvertToFortranForm : public PipelinablePass {
public:
    ConvertToFortranForm(SuifEnv *the_env) :
        PipelinablePass(the_env, "convert_to_fortran_form"){};

    Module *clone() const { return (Module *)this;}

	void initialize();

    /**
	    This converts 
        LoadExpressions     -> LoadVariableExpressions
          and 
        StoreExpressions    -> StoreVariableExpressions.

        Newly created expressions are marked with 
        k_fortran_form_artifact annote.

        In addition to that, it replaces the type of the 
        used or modified variables to remove one pointer
        dereference. It may also remove qualifiers on the 
        type. The old type is saved in k_old_fortran_type
        annote and is retrieved by the "undo" pass. The
        change of variable type also updates the procedure
        signature type accordingly.

        The pass also checks to make sure the procedure is 
        not in in the Fortran form yet (k_proc_in_fortran_form 
        annote is placed on the procedure symbol to indicate 
        that). If it is, a warning is issued and nothing happens.
    */
    void do_procedure_definition(ProcedureDefinition *proc_def);

	void fortranize_type(VariableSymbol* var);
};

/**
    Unconvert the program back from Fortran form.
*/
class UnconvertFromFortranForm : public PipelinablePass {
public:
    UnconvertFromFortranForm(SuifEnv *the_env) :
        PipelinablePass(the_env, "unconvert_from_fortran_form"){};

    Module *clone() const { return (Module *)this;}

	void initialize();

    /**
        Undoes what convert_to_fortran_form did.

	    It converts marked
        LoadVariableExpressions -> LoadExpressions
          and 
        StoreVariableExpressions -> StoreExpressions.

	    Types of variables are also updated to the old
        ones. Annotes on procedure symbols are removed.

        If k_proc_in_fortran_form annote is not found, 
        a warning is issued.
    */
    void do_procedure_definition(ProcedureDefinition *proc_def);

	void unfortranize_type(VariableSymbol* var);
};

#endif /* SOURCE_LANG_SUPPORT */