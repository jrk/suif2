/*-------------------------------------------------------------------
 * vform
 *
 */

#include "vtcl.h"
#include "vform.h"
#include "vcommands.h"
#include "dynatype.h"
#include "vmisc.h"
//#include <sty.h>
#include "common/suif_list.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include "suifkernel/suifkernel_forwarders.h"

struct field_info {
  char *name;
  char *type;

  field_info() {};
  field_info(char *nm, char *t) {
    name = strdup(nm);
    type = (char *)LString(t).c_str();
  }
  ~field_info() {
    delete (name);
  }
  bool operator==(const field_info &f) {
    return (strcmp(f.name, name) == 0 && f.type == type);
  }
};


//typedef slist_tos<field_info*> field_info_list;
typedef list<field_info*> field_info_list;

/*-------------------------------------------------------------------
 * vform::vform
 *
 */
vform::vform(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());
  dialog_binding = 0;
  fields = new field_info_list;

  tcl << "vform_create" << wpath << this << tcl_end;
}

/*-------------------------------------------------------------------
 * vform::~vform
 *
 */
vform::~vform(void)
{
  while ( !fields->empty() ) {
    delete fields->front();
    fields->pop_front();
  }
  delete fields;
}

/*-------------------------------------------------------------------
 * vform::destroy
 *
 */
void vform::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vform_destroy" << wpath << tcl_end;
  }
}

/*--------------------------------------------------------------------
 * Clear the form
 */
void vform::clear(void)
{
  tcl << "vform_clear" << wpath << tcl_end;

  while ( !fields->empty() ) {
    delete fields->front();
    fields->pop_front();
  }
}

/*--------------------------------------------------------------------
 * Add a field to the form
 */
void vform::add_field(char *field_name, char *type, char *val)
{
  tcl << "vform_add_entry" << wpath << this << field_name << type <<
    val << tcl_end;

  field_info *f = new field_info(field_name, type);
  fields->push_back(f);
}

/*--------------------------------------------------------------------
 * Insert a field to the form
 */
void vform::insert_field(int field_num, char *field_name, char *type,
		    char *val)
{
  int n = fields->size();
  if (field_num <= n) {

    tcl << "vform_insert_entry" << wpath << this << field_name << type <<
      val << field_num << tcl_end;

    field_info *f = new field_info(field_name, type);
    if (field_num == 0) {
      fields->push_back(f);
    } else {
      int i = 0;
      for ( s_count_t j=0; j<fields->size(); j++ ) {
	if (++i == field_num) {
	  //fields->insert_after(fields->handle_for_num( j ), f );
	  fields->insert((fields->get_nth(j))++, f );
	  break;
	}
      }
    }
  }
}

/*--------------------------------------------------------------------
 * Remove a field
 */
void vform::remove_field(int field_num)
{
  int n = fields->size();
  if (field_num < n) {
    tcl << "vform_remove_entry" << wpath << this << field_num << tcl_end;

    field_info *fi = (*fields)[field_num];
    //fields->remove( field_num) );
    fields->erase( fields->get_nth(field_num) );
    delete fi;
  }
}

/*--------------------------------------------------------------------
 * Set current field, focus on the field entry
 *
 */
void vform::set_current_field(int field_num)
{
  tcl << "vform_set_current_field" << wpath << this << field_num << tcl_end;
}

/*--------------------------------------------------------------------
 * Get current field
 *
 */
int vform::get_current_field(void)
{
  tcl << "vform_get_current_field" << wpath << this << tcl_end;
  int field_num;
  tcl >> field_num;
  return field_num;
}

/*--------------------------------------------------------------------
 * Get number of fields
 *
 */
int vform::num_fields(void)
{
  return (fields->size());
}

/*--------------------------------------------------------------------
 * Filter and check a field
 */

int vform::filter(char *type_name, char *variable)
{
  char *val = Tcl_GetVar(v_interp, variable, TCL_GLOBAL_ONLY);
  if (!val) {
    display_message(win(), "Warning: Cannot read tcl/tk variable");
    return (-1);		// error!
  }

  char *error_msg = 0;
  char *newval = typeman.type_check(type_name, val, error_msg);

  if (!newval) {
    display_message(win(), (char*) (error_msg ? error_msg : "Unknown type error"));
    return (-1);
  }

  char *result = Tcl_SetVar(v_interp, variable, newval, TCL_GLOBAL_ONLY);
  if (!result) {
    v_warning("Cannot set variable.");
  }
  delete (newval);

  return (0);
}

/*--------------------------------------------------------------------
 * Get a field data
 */
char *vform::get_field_data(int field_num)
{
  int result = tcl << "vform_get_data" << wpath << field_num << tcl_end;
  if (result != TCL_OK) {
    return (0);
  }

  return tcl.result();
}

/*--------------------------------------------------------------------
 * Get field type
 */
char *vform::get_field_type(int field_num)
{
  if (field_num >= (int)fields->size()) {
    return 0;
  }
  //return fields->elem( (s_count_t)field_num )->type;
  return (*fields)[(s_count_t)field_num]->type;
}

/*--------------------------------------------------------------------
 * Get field name
 */
char *vform::get_field_name(int field_num)
{
  if (field_num >= (int)fields->size()) {
    return 0;
  }
  return (*fields)[(s_count_t)field_num]->name;
}

/*--------------------------------------------------------------------
 * Set focus on a field
 */
void vform::focus_field(int field_num)
{
  tcl << "vform_set_focus" << wpath << this << field_num << tcl_end;
}

/*--------------------------------------------------------------------*/
/* tk command: vform
 *
 */
int TCLTK_CALLING_CONVENTION vform::vform_cmd(ClientData, Tcl_Interp *interp, int argc,
		     char *argv[])
{
  static char *firstargs[] = {"select", "destroy", "filter", "ok",
			      "cancel", 0};
  enum { SELECT = 0, DESTROY, FILTER, OK, CANCEL };

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }
  vform *form;
  sscanf(argv[2], "%p", &form);

  switch (a) {
  case SELECT:
    {
    }
    break;

  case DESTROY:
    {
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      form->destroy();
    }
    break;

  case FILTER:
    {
      if (argc != 5) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      int result = form->filter(argv[3], argv[4]);
      if (result == -1) {
	return (TCL_ERROR);
      }
    }
    break;

  default:
    return (TCL_ERROR);
  }

  return (TCL_OK);
}
