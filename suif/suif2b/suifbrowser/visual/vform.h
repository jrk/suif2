/*--------------------------------------------------------------------
 * vform.h
 *
 */

#ifndef VFORM_H
#define VFORM_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"
#include <stdarg.h>

class field_info;
class binding;

//typedef slist_tos<field_info*> field_info_list;
typedef list<field_info*> field_info_list;


class vform : public vwidget {

protected:

  binding *dialog_binding;
  field_info_list *fields;

  int filter(char *title, char *variable);

public:

  vform(vwidget *par);
  ~vform(void);
  void destroy(void);

  void add_field(char *field_name, char *type, char*val);
  void insert_field(int field_num, char *field_name, char *type,
		    char *val);
  void remove_field(int field_num);
  void clear(void);

  char *get_field_data(int field_num);
  char *get_field_type(int field_num);
  char *get_field_name(int field_num);
  int num_fields(void);

  int get_current_field(void);
  void set_current_field(int field_num);
  void focus_field(int field_num);

  void set_dialog_binding(binding *b) {
    dialog_binding = b;
  }

  virtual int kind(void) { return WIDGET_FORM; }

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vform_cmd(ClientData, Tcl_Interp *interp, int argc,
		       char *argv[]);

};

#endif
