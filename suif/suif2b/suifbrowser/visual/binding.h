/*--------------------------------------------------------------------
 * binding.h
 *
 * A "binding" is a function callback that is attached to a user event.
 * It can be attached to a menu item, user selection, invocation, etc.
 *
 * The callback function can be in several forms - tcl/tk command,
 * "bfun" or "bfun2" functions (defined in binding.h). The function
 * parameters are a "event" object, and one or two client data. The
 * "event" object contains context information of the event that invoked
 * the binding.
 *
 */

#ifndef BINDING_H
#define BINDING_H

#include "event.h"
//#include <sty.h>
#include "common/suif_list.h"

class vnode;

typedef void (*bfun)(const event &e, void *client_data);
typedef void (*bfun2)(const event &e, void *client_data1, void *client_data2);

class binding {
private:
  char *tcl_command;
  bfun function;

  int num_client_data;
  void *client_data1;
  void *client_data2;

public:
  binding(void);
  binding(bfun f, void *data = 0);
  binding(bfun2 f, void *data1, void *data2);

  ~binding(void);

  void set_tcl_command(char *s, void *data = 0) {
    tcl_command = s;
    client_data1 = data;
    num_client_data = 1;
  }
  void set_tcl_command(char *s, void *data1, void *data2) {
    tcl_command = s;
    client_data1 = data1;
    client_data2 = data2;
    num_client_data = 2;
  }
  void set_function(bfun f, void *data = 0) { 
    function = f;
    client_data1 = data;
    num_client_data = 1;
  };
  void set_function(bfun2 f, void *data1, void *data2) { 
    function = (bfun) f;
    client_data1 = data1;
    client_data2 = data2;
    num_client_data = 2;
  };

  /* invoke */
  void invoke(const event &e);
};


//typedef slist_tos<binding*> binding_list;
typedef list<binding*> binding_list;

void delete_bindings(binding_list *bindings);

#endif
