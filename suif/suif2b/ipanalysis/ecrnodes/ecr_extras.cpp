#include "suifkernel/suif_env.h"
#include "suifkernel/print_subsystem.h"

extern "C" void init_ecr_extras(SuifEnv *s);

static struct PrintSpecClass defaultPrintStrings[] = {
 { "EcrRefAnnote", "[\"%**_name\": %**_ecr_id %**_ecr_set]" },
 { "EcrSetTauObject", "%**_id->{%**_points_to_id, %**_points_to_procedure_id}\n" },
 { "EcrSetLambdaObject", "%**_id={%LC%**_ecr_procs}\n" },
 { "EcrManagerAnnote", "[\"%**_name\": %**_ecr_sets]\n" }
};

#define PRINT_SIZE (sizeof(defaultPrintStrings)/sizeof(PrintSpecClass))

static PrintSpecClass defaultPrintRefStrings[] = {
 { "EcrRefAnnote", "[\"%**_name\": %*_ecr_id %**_ecr_set]" },
 { "EcrSetTauObject", "%**_id->{%**_points_to_id, %**_points_to_procedure_id}" },
 { "EcrSetLambdaObject", "%**_id={%LC%**_ecr_procs}" },
};

#define PRINT_REF_SIZE (sizeof(defaultPrintRefStrings)/sizeof(PrintSpecClass))


extern "C" void init_ecr_extras(SuifEnv *s) {
  s->require_module("iputils");
  PrintSubSystem *print_sys = s->get_print_subsystem();
  PrintStringRepository *psub = 
    print_sys->retrieve_string_repository("print_suif");
  psub->set_print_string_by_table(defaultPrintStrings,
				  PRINT_SIZE);
  psub->set_print_ref_string_by_table(defaultPrintRefStrings,
				      PRINT_REF_SIZE);
}
