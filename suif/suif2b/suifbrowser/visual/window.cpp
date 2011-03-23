/*-------------------------------------------------------------------
 * window.cc
 * 
 */

#include "window.h"
#include "binding.h"
#include "event.h"
#include "vtoplevel.h"
#include "vman.h"

/*-------------------------------------------------------------------
 * window::window
 *
 */
window::window(void)
{
  bindings = new binding_list;
  toplevel = 0;
}

/*-------------------------------------------------------------------
 * window::~window
 *
 */
window::~window(void)
{
  delete (toplevel);
  delete_bindings(bindings);
  // delete_bindings calls delete_list_and_elements which deletes bindings as
  // well.
  //delete (bindings);
}

/*-------------------------------------------------------------------
 * window::create_window
 *
 */
void window::create_window(void)
{
  post_event(event(0, WINDOW_CREATE, 0, this));
  toplevel = new vtoplevel(class_name(), this);
}

/*-------------------------------------------------------------------
 * window::destroy
 *
 */
void window::destroy(void)
{
  if (toplevel) {
    toplevel->destroy();
  }
}

/*-------------------------------------------------------------------
 * window::destroyed
 *
 */
void window::destroyed(void)
{
  post_event(event(0, WINDOW_CLOSE, 0, this));
  delete (this);
}

