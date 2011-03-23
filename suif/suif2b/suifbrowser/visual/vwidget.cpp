/*-------------------------------------------------------------------
 * vwidget.cc
 *
 */

#include "vwidget.h"
#include "vtoplevel.h"
#include "window.h"

/*----------------------------------------------------------------------
 * class_name
 */

window *vwidget::win(void)
{
  window *the_window = 0;

  if (parent) {
    vwidget *w;
    for (w = parent; w->parent; w = w->parent);
    if (w->kind() == WIDGET_TOPLEVEL) {
      the_window = ((vtoplevel *) w)->win();
    }
  }

  return (the_window);
}
