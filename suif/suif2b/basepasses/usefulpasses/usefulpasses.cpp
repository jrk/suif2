#include "common/system_specific.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"

#include "time_module.h"
#include "unowned.h"
#include "type_checker.h"
#include "unused_passes.h"
#include "count_suif_object_pass.h"
#include "gc_symbol_table_pass.h"
#include "validate_suif_pass.h"
#include "exec_pass.h"
#include "source_lang_support.h"
#include "strip_annotes_pass.h"

extern "C" void init_suifnodes(SuifEnv *);
extern "C" void init_cfenodes(SuifEnv *);

extern "C" void init_usefulpasses( SuifEnv* suif_env ) {
  ModuleSubSystem* module_subsystem = suif_env->get_module_subsystem();

  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("utils");

  module_subsystem -> register_module( new TimeModule( suif_env ) );
  module_subsystem -> register_module( new UnownedPass( suif_env ) );
  module_subsystem -> register_module( new TypeCheckerPass(suif_env));
  module_subsystem -> register_module( new RemoveTrashPass(suif_env));
  module_subsystem -> register_module( new StripAnnotesPass(suif_env) );
  module_subsystem -> test_and_register_module(new CountSuifObjectPass(suif_env),
					       true);
  module_subsystem -> test_and_register_module(new GCSymbolTablePass(suif_env),
					       true);
  module_subsystem -> test_and_register_module(new ValidateSuifPass(suif_env),
					       true);
  module_subsystem -> test_and_register_module(new ExecPass(suif_env), true);

  module_subsystem -> test_and_register_module(new ConvertToFortranForm(suif_env), true);
  module_subsystem -> test_and_register_module(new UnconvertFromFortranForm(suif_env), true);
}
