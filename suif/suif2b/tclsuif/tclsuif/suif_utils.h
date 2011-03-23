#ifndef _TCLSUIF_SUIF_UTIL_H_
#define _TCLSUIF_SUIF_UTIL_H_


#include "tcl.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/suif_env.h"




/** Find the SuifObject from the current file set block.
  * Search only via the owner link.
  * @param suif_env IN
  * @param id       IN name of the object searching for.
  */
SuifObject* suif_get_object(SuifEnv* suif_env, const char* id);



/** Find the SuifObject from the current file set block.
  * If not found, enter an error message to the tcl interpreter.
  * @param id   IN name of the object being sought for.
  * @param suif_env IN the suif env.
  * @param interp IN result will be set to an error message if the object
  *                  is not found.
  * @return the object or NULL if not found.
  */
SuifObject* suif_get_object_with_test(const char* id, SuifEnv *suif_env,
				      Tcl_Interp* interp);



/** Get all immediate children (owned objects).
  * @param root     IN  the parent.
  * @param children OUT into which the children are dumped.
  * @return number of immediate children.
  */
unsigned suif_get_children(SuifObject* root,
			   suif_vector<SuifObject*>* children);



/** Get all immediate referenced object.
  * @param root    IN 
  * @param referenced OUT into which the objects referenced from \a root are
  * dumped.
  * @return number of objects dumped into \a referenced.
  */
unsigned suif_get_referenced(SuifObject* root,
			     suif_vector<SuifObject*> *referenced);



/** Get the name of all fields in this object.
  * @param dstr OUT which hold the result.
  * @result TCL_OK or TCL_ERROR.
  */
int suif_get_field_names(const Address addr, const MetaClass* mc,
			 Tcl_DString* dstr);



/** Get the printed form of all field values in this object.
  * @param dstr OUT which hold the result.
  * @return TCL_OK or TCL_ERROR.
  */
int suif_get_field_values(const Address addr, const MetaClass* mc,
			  Tcl_DString* dstr);


/** Get the printed form of a field type.
  * result in one of :
  * {elementary classname}
  * {pointer basetype owned_or_referenced}
  * {list element_type}
  * {object classname}
  * {aggregate classname fieldnames fieldtypes}
  * {unknown metaclassname}
  *
  * @return TCL_OK or TCL_ERROR.
  */
int suif_get_field_types(const Address addr, const MetaClass* mc,
			 Tcl_DString* dstr);





#endif // _TCLSUIF_SUIF_UTIL_H_
