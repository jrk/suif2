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

extern SuifEnv *suif_env;
extern SuifPrinterModule *spm;

static char *help_mesg =
"Suif Browser\n"
"\n"
"Syntax: sbrowser [options] [suif file(s)]\n"
"\n"
"options:\n"
"	-help		   Show this help message.\n"
"	-sync		   Use synchronous mode for display server.\n"
"\n";

extern char *sbrowser_help_text;

void
start_suif(int *argc, char *argv[])
{
  suif_env = create_suif_env();
  suif_env->require_module("sbrowser");
  /* this will actually register the browser. */
  /* Now we should really execute the pass */
  /* However, the pass doesn't handle arguments well yet */
}
/*----------------------------------------------------------------------
 * main
 *
 */

int main(int argc, char * argv[])
{
  /* parse command line */
  for (int i = 1; i < argc; i++) {
    char *s = argv[i];
    if (s[0] == '-') {
      if (strcmp(&s[1], "help") == 0) {
	fprintf( stderr, help_mesg );
	return 1;
      }
    }  
  }
  
  start_suif(&argc, argv);
  enter_visual(&argc, argv);
  enter_viewers(&argc, argv);
  
  /* set application help text */
  application_help_text = sbrowser_help_text;

  /* start viewers */
  start_viewers(argc, argv);

  visual_mainloop();

  return 0;
}
