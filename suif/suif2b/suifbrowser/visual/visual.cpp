/*----------------------------------------------------------------------
 * visual.cc
 */

#include "vtcl.h"
//#include <sty.h>
#include "vcommands.h"
#include "vman.h"
#include "event.h"
#include <tcl.h>
#include <tk.h>
#include <stdlib.h>
#include <unistd.h>
//#include <io.h>

#define VISUAL_TCL_LIB_INTERFACE_VER  "1.0"

/* globals */
Tcl_Interp *v_interp;
tcl_command tcl;
tcl_terminator *tcl_end;

/*--------------------------------------------------------------------*/
/* local */

static void Tkmain(int argc, char *argv[]);
// static void Prompt(Tcl_Interp *interp, int partial);
// static void StdinProc(ClientData clientData, int mask);

// static Tcl_DString command;	/* Used to assemble lines of terminal input
//				 * into Tcl commands. */

/*--------------------------------------------------------------------*/
/*
 * main initialization routine
 */

void enter_visual(int *argc, char *argv[])
{
  static bool init_flag = false;
  if (init_flag) return;
  init_flag = true;

  Tkmain(*argc, argv);
  tcl.set_interp(v_interp);	// init interpreter

  /* initialize runtime library */

  char tcl_command1[] = 
    "if {[catch {set p $env(VISUAL_TCL)}] == 1} {\n"
    "  set env(VISUAL_TCL) \"$env(SUIFHOME)/tcl\"\n"
    "}\n";
  tcl.eval(tcl_command1);

  char tcl_command2[] = "source \"$env(VISUAL_TCL)/visual.tcl\"";
  int result = tcl.eval(tcl_command2);
  if (result != TCL_OK) {
    fprintf(stderr, 
	    "Error(s) encountered while initializing the visual system: %s\n",
	    tcl.result());
    exit(1);
  }
  if (strcmp(tcl.result(), "Error") == 0) {
    fprintf(stderr, "Fatal error(s) encountered.\n");
    exit(1);
  }
  if (strcmp(tcl.result(), VISUAL_TCL_LIB_INTERFACE_VER) != 0) {
    fprintf(stderr, 
	    "Wrong visual_tcl_lib tcl runtime library interface version!\n"
	    "This program requres visual_tcl_lib interface version `%s'.\n"
	    "Please make sure that the environment variable `VISUAL_TCL'\n"
	    "is set to the correct runtime tcl directory.",
	    VISUAL_TCL_LIB_INTERFACE_VER
	    );
    exit(1);
  }

  /* create tcl commands */
  v_create_commands(v_commands);

  /* event manager */
  init_eman();

  /* visual manager */
  init_vman();
}

/*----------------------------------------------------------------------
 * exit visual
 */

void exit_visual(void)
{
  exit_vman();
  exit_eman();
}

/*----------------------------------------------------------------------
 * visual main event loop
 */

void visual_mainloop(void)
{
  Tk_MainLoop();
}

void visual_do_one_event(void)
{
  Tk_DoOneEvent(0);
}

void visual_yield(int yield_count)
{
  for (int i = 0; i < yield_count; i++) {
    if (!Tk_DoOneEvent(TK_DONT_WAIT | TK_IDLE_EVENTS)) {
      break;
    }
  }
}

/*----------------------------------------------------------------------
 * Make visual interactive, expose tcl prompt to user
 */
#ifdef AG
void visual_prompt(void)
{
  /*
   * Set the "tcl_interactive" variable.
   */
  Tcl_SetVar(v_interp, "tcl_interactive", "1", TCL_GLOBAL_ONLY);

  Tcl_DStringInit(&command);
  Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);

  Prompt(v_interp, 0);
}
#endif
/*--------------------------------------------------------------------*/
/*
 * this is modified from tk 3.6
 *
 */

// static Tk_Window mainWindow;	/* The main window for the application.  If
// 				 * NULL then the application no longer
// 				 * exists. */

static int synchronize = 0;
// static char *display = 0;

static Tk_ArgvInfo argTable[] = {
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
	"Use synchronous mode for display server"},
    {(char *) 0, TK_ARGV_END, (char *) 0, (char *) 0,
	(char *) 0}
};

static void Tkmain(int argc, char *argv[])
{
  char *args;
  char buf[20];
  v_interp = Tcl_CreateInterp();
  Tcl_InitMemory(v_interp);
  /*
   * Parse command-line arguments.
   */
  if (Tk_ParseArgv(v_interp, (Tk_Window) 0, &argc, argv, argTable, 0)
      != TCL_OK) {
    fprintf(stderr, "%s\n", v_interp->result);
    exit(1);
  }
#ifdef AG
  /*
   * Initialize the Tk application.
   */
  mainWindow = Tk_CreateMainWindow(v_interp, display, argv[0], "Tk");
  if (mainWindow == 0) {
    fprintf(stderr, "%s\n", v_interp->result);
    exit(1);
  }
  if (synchronize) {
    XSynchronize(Tk_Display(mainWindow), true);
  }
  Tk_GeometryRequest(mainWindow, 200, 200);
#endif
  /*
   * Make command-line arguments available in the Tcl variables "argc"
   * and "argv".  Also set the "geometry" variable from the geometry
   * specified on the command line.
   */

  args = Tcl_Merge(argc-1, argv+1);
  Tcl_SetVar(v_interp, "argv", args, TCL_GLOBAL_ONLY);
  ckfree(args);
  sprintf(buf, "%d", argc-1);
  Tcl_SetVar(v_interp, "argc", buf, TCL_GLOBAL_ONLY);
  Tcl_SetVar(v_interp, "argv0", argv[0], TCL_GLOBAL_ONLY);

  /*
   * Set the "tcl_interactive" variable.
   */

  Tcl_SetVar(v_interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

  /*
   * Init Tcl
   */
  
  if (Tcl_Init(v_interp) == TCL_ERROR) {
    fprintf(stderr, "Tcl Error: %s", v_interp->result);
    exit(1);
  }
  /*
   * Init Tk
   */

  if (Tk_Init(v_interp) == TCL_ERROR) {
    fprintf(stderr, "Tk Error: %s", v_interp->result);
    exit(1);
  }
#ifdef TK4_1
  /*
   * Initialize the Tk application.
   */
  /* Tk4.1 removes the CreateMainWindow call */
  mainWindow = Tk_MainWindow(v_interp);
  if (mainWindow == 0) {
    fprintf(stderr, "%s\n", v_interp->result);
    exit(1);
  }
  if (synchronize) {
    XSynchronize(Tk_Display(mainWindow), true);
  }
  Tk_GeometryRequest(mainWindow, 200, 200);

#endif
}

/*
 *----------------------------------------------------------------------
 * Taken from tkMain.c (ver 3.6), modified
 *
 * StdinProc --
 *
 *	This procedure is invoked by the event dispatcher whenever
 *	standard input becomes readable.  It grabs the next line of
 *	input characters, adds them to a command being assembled, and
 *	executes the command if it's complete.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Could be almost arbitrary, depending on the command that's
 *	typed.
 *
 *----------------------------------------------------------------------
 */
#ifdef AG
static void
StdinProc(ClientData /* clientData */, /* Not used. */
	  int /* mask */)	 /* Not used. */
{
#undef BUFFER_SIZE
#define BUFFER_SIZE 4000
    char input[BUFFER_SIZE+1];
    static int gotPartial = 0;
    char *cmd;
    int code, count;


    count = read(fileno(stdin), input, BUFFER_SIZE);
    if (count <= 0) {
	if (!gotPartial) {
	  Tcl_Eval(v_interp, "exit");
	  exit(1);
	} else {
	  count = 0;
	}
    }
    cmd = Tcl_DStringAppend(&command, input, count);
    if (count != 0) {
	if ((input[count-1] != '\n') && (input[count-1] != ';')) {
	    gotPartial = 1;
	    goto prompt;
	}
	if (!Tcl_CommandComplete(cmd)) {
	    gotPartial = 1;
	    goto prompt;
	}
    }
    gotPartial = 0;

    /*
     * Disable the stdin file handler while evaluating the command;
     * otherwise if the command re-enters the event loop we might
     * process commands from stdin before the current command is
     * finished.  Among other things, this will trash the text of the
     * command being evaluated.
     */

    Tk_CreateFileHandler(0, 0, StdinProc, (ClientData) 0);
    code = Tcl_RecordAndEval(v_interp, cmd, 0);
    Tk_CreateFileHandler(0, TK_READABLE, StdinProc, (ClientData) 0);
    Tcl_DStringFree(&command);
    if (*v_interp->result != 0) {
      printf("%s\n", v_interp->result);
    }

    /*
     * Output a prompt.
     */

    prompt:
    Prompt(v_interp, gotPartial);
}
#endif
/*
 *----------------------------------------------------------------------
 * Taken from tkMain.c (ver 3.6)
 *
 * Prompt --
 *
 *	Issue a prompt on standard output, or invoke a script
 *	to issue the prompt.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A prompt gets output, and a Tcl script may be evaluated
 *	in interp.
 *
 *----------------------------------------------------------------------
 */
#ifdef AG
static void
Prompt(Tcl_Interp *interp,     /* Interpreter to use for prompting. */
       int partial)     /* Non-zero means there already
			 * exists a partial command, so use
			 * the secondary prompt. */

{
    char *promptCmd;
    int code;

    promptCmd = Tcl_GetVar(interp,
	partial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
    if (promptCmd == 0) {
    defaultPrompt:
	if (!partial) {
	    fputs("% ", stdout);
	}
    } else {
	code = Tcl_Eval(interp, promptCmd);
	if (code != TCL_OK) {
	    Tcl_AddErrorInfo(interp,
		    "\n    (script that generates prompt)");
	    fprintf(stderr, "%s\n", interp->result);
	    goto defaultPrompt;
	}
    }
    fflush(stdout);
}
#endif
