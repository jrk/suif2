#include "tcl.h"

#include "suifkernel/suif_env.h"
#include "suifpasses/suifpasses.h"

extern int Suif_Cmd(ClientData cdata, Tcl_Interp *interp,
		    int objc, Tcl_Obj* const objv[]);
extern int SuifFsb_Cmd(ClientData cdata, Tcl_Interp *interp,
		       int objc, Tcl_Obj* const objv[]);
extern int SuifChildren_Cmd(ClientData cdata, Tcl_Interp *interp,
			    int objc, Tcl_Obj* const objv[]);
extern int SuifFields_Cmd(ClientData cdata, Tcl_Interp *interp,
			 int objc, Tcl_Obj* const objv[]);
extern int SuifListModules_Cmd(ClientData cdata, Tcl_Interp *interp,
			       int objc, Tcl_Obj* const objv[]);
extern int SuifDescribeModule_Cmd(ClientData cdata, Tcl_Interp *interp,
				  int objc, Tcl_Obj* const objv[]);


extern int SuifNafValue_Cmd(ClientData cdata, Tcl_Interp *interp,
			    int objc, Tcl_Obj* const objv[]);


extern "C" {

int Tclsuif_Init(Tcl_Interp *interp)
{
  SuifEnv* suif_env = create_suif_env();
  init_suifpasses(suif_env);

  Tcl_PkgProvide(interp, "suif", "1.0");
  Tcl_CreateObjCommand(interp, "suif", Suif_Cmd, (ClientData)suif_env, 0);
  Tcl_CreateObjCommand(interp, "suif_fsb", SuifFsb_Cmd,
		       (ClientData)suif_env, 0);
  Tcl_CreateObjCommand(interp, "suif_children", SuifChildren_Cmd,
		       (ClientData)suif_env, 0);
  Tcl_CreateObjCommand(interp, "suif_fields", SuifFields_Cmd,
		       (ClientData)suif_env, 0);
  Tcl_CreateObjCommand(interp, "suif_list_modules", SuifListModules_Cmd,
		       (ClientData)suif_env, 0);
  Tcl_CreateObjCommand(interp, "suif_describe_module", SuifDescribeModule_Cmd,
		       (ClientData)suif_env, 0);
  /*
  Tcl_CreateObjCommand(interp, "naf_value", SuifNafValue_Cmd,
		       (ClientData)suif_env, 0);
		       */
  return TCL_OK;
};


}
