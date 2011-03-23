#include "suifkernel/suif_env.h"
#include "suifkernel/print_subsystem.h"
#include "suifpasses/passes.h"
#include "suifkernel/utilities.h"
#include "suifkernel/module_subsystem.h"
#include "suifnodes/suif.h"
#include "iputils_query.h"

extern "C" void init_iputils_extras(SuifEnv *s);

static struct PrintSpecClass defaultPrintStrings[] = {
  { "CallTargetAnnote", "[\"%**_name\": %**_targets]" }
};

#define PRINT_SIZE (sizeof(defaultPrintStrings)/sizeof(PrintSpecClass))

/**
    A pass to add "entry" annotation on procedures such as main.

    Currently doesn't support entry points of libraries.
*/
class MarkEntryPointsPass: public PipelinablePass {
public:
    MarkEntryPointsPass(SuifEnv *the_env) :
        PipelinablePass(the_env, "mark_entry_points"){};

    Module *clone() const { return (Module *)this;}

    void do_procedure_definition(ProcedureDefinition *proc_def){
        ProcedureSymbol* sym = proc_def->get_procedure_symbol();
        IPUtilsQuery::is_unique_entry_point(sym);
    }
};

extern "C" void init_iputils_extras(SuifEnv *s) {
  PrintSubSystem *print_sys = s->get_print_subsystem();
  PrintStringRepository *psub = 
    print_sys->retrieve_string_repository("print_suif");
  psub->set_print_string_by_table(defaultPrintStrings,
				  PRINT_SIZE);
  ModuleSubSystem* ms = s->get_module_subsystem();
  ms->register_module(new MarkEntryPointsPass(s));
}
