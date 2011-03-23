#include <iostream.h>
#include "common/suif_vector.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/object_factory.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/module.h"
#include "suifkernel/token_stream.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"
#include "utils/print_utils.h"

#include "tcl.h"
#include "suif_utils.h"
#include "tcl_token_stream.h"

/*
#include "naf/naf_problem.h"
#include "naf/query.h"
*/

/** Remember to execute "suif import cfenodes" before "suif load ...".
  * Otherwise you will get very funny seg fault (like to_id_string() suddenly
  * called Token:: method.)
  */



/** Pre condition: argv has "children ..."
  */
int
SuifChildren_Cmd(ClientData senv, Tcl_Interp *interp,
		 int objc, Tcl_Obj * const objv[])
{
  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "obj_id");
    return TCL_ERROR;
  }
  SuifEnv *suif_env = (SuifEnv*)senv;
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  SuifObject* obj = suif_get_object_with_test(Tcl_GetStringFromObj(objv[1], 0),
					      suif_env, interp);
  if (obj == 0) return TCL_ERROR;
  suif_vector<SuifObject*> children;
  unsigned ccnt = suif_get_children(obj, &children);
  for (unsigned i=0; i<ccnt; i++) {
    Tcl_AppendElement(interp, to_id_string(children[i]));
  }
  return TCL_OK;
}



/** Pre condition: argv has "field ...".
  * returns to tclsh three lists {field_names field_values field_types}
  */
int SuifFields_Cmd(ClientData senv, Tcl_Interp* interp,
		   int objc, Tcl_Obj * const objv[])
{
  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "obj_id");
    return TCL_ERROR;
  }
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  SuifObject* root = suif_get_object_with_test(Tcl_GetStringFromObj(objv[1], 0), suif_env, interp);
  if (root == 0) return TCL_ERROR;
  const MetaClass *mc = root->get_meta_class();
  Tcl_DString fields;
  Tcl_DStringInit(&fields);
  suif_get_field_names(root, mc, &fields);
  Tcl_AppendElement(interp, Tcl_DStringValue(&fields));
  Tcl_DStringFree(&fields);

  suif_get_field_values(root, mc, &fields);
  Tcl_AppendElement(interp, Tcl_DStringValue(&fields));
  Tcl_DStringFree(&fields);

  suif_get_field_types(root, mc, &fields);
  Tcl_AppendElement(interp, Tcl_DStringValue(&fields));
  Tcl_DStringFree(&fields);

  return TCL_OK;
}



    
/** Pre condition: argv has "?module_name? ..."
  */
int
SuifModule_Cmd(ClientData senv, Tcl_Interp *interp,
	       int objc, Tcl_Obj * const objv[])
{
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  ModuleSubSystem* msys = suif_env->get_module_subsystem();
  TclTokenStream tokstrm(objc, objv);
  try {
    msys->execute(Tcl_GetStringFromObj(objv[0], 0), &tokstrm);
  } catch (SuifException &exp) {
    Tcl_SetResult(interp, (char*)exp.get_message().c_str(), TCL_VOLATILE);
    return TCL_ERROR;
  } catch (...) {
    Tcl_SetResult(interp, "Runaway exception intercepted.", TCL_STATIC);
    return TCL_ERROR;
  }
  return TCL_OK;
}

/** Pre-condition: argv has "fsb ..."
  */
int SuifFsb_Cmd(ClientData senv, Tcl_Interp *interp,
		int objc, Tcl_Obj * const objv[])
{
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  if (objc != 1) {
    Tcl_WrongNumArgs(interp, 1, objv, "");
    return TCL_ERROR;
  }
  FileSetBlock* fsb = suif_env->get_file_set_block();
  if (fsb == 0) return TCL_OK;
  Tcl_SetResult(interp, (char*)(to_id_string(fsb).c_str()), TCL_VOLATILE);
  return TCL_OK;
}

int Suif_Cmd(ClientData cdata, Tcl_Interp *interp,
	     int objc, Tcl_Obj* const objv[])
{
  return SuifModule_Cmd(cdata, interp, objc-1, objv+1);
}


    
int
SuifListModules_Cmd(ClientData senv, Tcl_Interp *interp,
	       int objc, Tcl_Obj * const objv[])
{
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (objc != 1) {
    Tcl_WrongNumArgs(interp, 1, objv, "");
    return TCL_ERROR;
  }
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  ModuleSubSystem* msys = suif_env->get_module_subsystem();
  list<LString> mnames;
  msys->get_module_list(LString(), mnames);
  for (list<LString>::iterator iter = mnames.begin();
       iter != mnames.end();
       iter++) {
    Tcl_AppendElement(interp, (*iter));
  }
  return TCL_OK;
}


int
SuifDescribeModule_Cmd(ClientData senv, Tcl_Interp *interp,
		       int objc, Tcl_Obj * const objv[])
{
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "module_name");
    return TCL_ERROR;
  }
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  ModuleSubSystem* msys = suif_env->get_module_subsystem();
  const char* mod_name = Tcl_GetStringFromObj(objv[1], 0);
  Module* m = msys->retrieve_module(LString(mod_name));
  if (m == 0) {
    Tcl_AppendResult(interp, "No module named <", mod_name, (char*)0);
    return TCL_ERROR;
  }
  Tcl_SetResult(interp, const_cast<char*>(m->get_description().c_str()),
		TCL_VOLATILE);
  return TCL_OK;
}







/** naf_summary <query> <region>
  * returns: the string value of the summary
  */
/*
int SuifNafValue_Cmd(ClientData senv, Tcl_Interp *interp,
		     int objc, Tcl_Obj* const objv[])
{
  if (objc != 3) {
    Tcl_WrongNumArgs(interp, 2, objv, "?query? ?region?");
    return TCL_ERROR;
  }
  SuifEnv* suif_env = (SuifEnv*)senv;
  if (suif_env == 0) {
    Tcl_SetResult(interp, "SuifEnv not initialized.", TCL_STATIC);
    return TCL_ERROR;
  }
  const char* qname = Tcl_GetStringFromObj(objv[1], 0);
  const char* rname = Tcl_GetStringFromObj(objv[2], 0);
  SuifObject* r = suif_get_object_with_test(rname, suif_env, interp);
  if (!is_kind_of<Region>(r)) {
    Tcl_AppendResult(interp, rname, " is not a region.", 0);
    return TCL_ERROR;
  }
  try {
    const LatticeValue* v = Query::get_value(qname, to<Region>(r));
    Tcl_SetResult(interp, const_cast<char*>(v->to_string().c_str()),
		  TCL_STATIC);
    return TCL_OK;
  } catch (SuifException& exp) {
    Tcl_SetResult(interp, const_cast<char*>(exp.get_message().c_str()),
		  TCL_STATIC);
    return TCL_ERROR;
  }
}
*/
