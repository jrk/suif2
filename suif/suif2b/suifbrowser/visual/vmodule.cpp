/*-------------------------------------------------------------------
 * module.cc
 *
 */

#include "vmodule.h"
#include "vmenu.h"
#include "binding.h"
#include <stdlib.h>

/*--------------------------------------------------------------------
 */
static void event_helper(event &e, module *m)
{
  m->handle_event(e);
}

/*--------------------------------------------------------------------
 * module::module
 */
module::module(void)
{
  eb = new binding((bfun) &event_helper, this);
  add_event_binding(eb, VISUAL_EVENTS | X_EVENTS | VISUAL_USER_EVENTS);

  menu = new virtual_menu;
}

/*--------------------------------------------------------------------
 * module::module
 *
 */
module::~module(void)
{
  remove_event_binding(eb);
  delete (eb);
}

/*--------------------------------------------------------------------
 * module::attach_menu
 *
 */
void module::attach_menu(vmenu *par_menu, char *menu_path)
{
  menu->attach(par_menu, menu_path);
}
