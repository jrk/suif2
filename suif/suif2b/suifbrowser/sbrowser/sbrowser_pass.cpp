/*
 * sbrowser.cc
 *
 *
 */

// Careful! There is a visual/module.h in this package too!
#include <stdlib.h>
#include <stdio.h>
#include "viewers/viewers.h"
#include "visual/visual.h"
#include "suifkernel/suif_env.h"
#include "suifnodes/suif.h"
#include "suifkernel/module.h"
#include "suifkernel/module_subsystem.h"
#include "suifprinter/suifprinter.h"
#include <unistd.h>

#include "common/lstring.h"

SuifEnv *suif_env = 0;
SuifPrinterModule *spm = 0;

extern char *sbrowser_help_text;

// On initialize, this pass will
// fork off a new process for visualization.
class SBrowserModule : public Module {
  bool _live; // TRUE if this is the Pass in the active thread.
  pid_t _child;
public:
  SBrowserModule(SuifEnv *env, const LString &name = "sbrowser") :
    Module(env, name) {
  }
  ~SBrowserModule() {};
  Module *clone() const { return (Module*)this; }

  void initialize() {
    Module::initialize();
  }
  void execute() {
    //    _child = vfork();
    //    if (_child != 0)
    //      return;
    // Otherwise, we are the sbrowser.  fire us up.
    int argc = 1;
    char *argv[] = { "sbrowser", "\n" };

    // This is the worst hack EVER...
    suif_env = get_suif_env();
    enter_visual(&argc, argv);
    enter_viewers(&argc, argv);
  
    /* set application help text */
    application_help_text = sbrowser_help_text;

    /* start viewers */
    start_viewers(argc, argv);

    visual_mainloop();
  }
};


extern "C" void init_sbrowser(SuifEnv *suif_env)
{
  suif_env->require_module("suifnodes");
  suif_env->require_module("suifprinter");

  ModuleSubSystem *mSubSystem = suif_env->get_module_subsystem();
  spm = (SuifPrinterModule*) mSubSystem->retrieve_module("print_suif");
  if (!spm)
    cerr << "Unable to find module - SuifPrinter\n";
  mSubSystem->register_module(new SBrowserModule(suif_env));
}
  
