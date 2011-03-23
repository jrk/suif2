/*-------------------------------------------------------------------
 * binding.cc
 *
 */

#include "binding.h"
#include "event.h"
#include "vtcl.h"
#include <stdio.h>
#include "iokernel/helper.h"

/*-------------------------------------------------------------------
 * binding::binding
 */
binding::binding(void)
{
  tcl_command = "";

  function = 0;
  num_client_data = 1;
  client_data1 = client_data2 = 0;
}

binding::binding(bfun f, void *data)
{
  tcl_command = "";

  function = f;
  client_data1 = data;
  num_client_data = 1;
};

binding::binding(bfun2 f, void *data1, void *data2)
{
  tcl_command = "";

  function = (bfun) f;
  client_data1 = data1;
  client_data2 = data2;
  num_client_data = 2;
};

/*-------------------------------------------------------------------
 * binding::~binding
 */
binding::~binding(void)
{
}

/*-------------------------------------------------------------------
 * binding::invoke
 */
void binding::invoke(const event &e)
{
  if (tcl_command[0]) {
    tcl << tcl_command;
    if (num_client_data > 0) {
      if (client_data1) {
	tcl << client_data1;
      }
      if (num_client_data > 1 && client_data2) {
	tcl << client_data2;
      }
    }

    tcl << e.get_object() << tcl_end;
  }

  if (function) {
    if (num_client_data == 1) {
      (*function)(e, client_data1);
    } else {
      (*(bfun2)function)(e, client_data1, client_data2);
    }
  }
}

/*-------------------------------------------------------------------
 * delete_bindings
 */
void delete_bindings(binding_list *bindings)
{
  //while (!bindings->is_empty()) {
    //delete bindings->pop();
  //}
  delete_list_and_elements(bindings);
}
