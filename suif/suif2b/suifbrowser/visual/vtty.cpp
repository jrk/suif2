/*-------------------------------------------------------------------
 * vtty
 *
 */

#include "vtty.h"
#include "vtcl.h"
#include "vcommands.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>

/*-------------------------------------------------------------------
 * vtty::vtty
 */
vtty::vtty(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());
  text_pipe  = new vpipe;
}

/*-------------------------------------------------------------------
 * vtty::~vtty
 *
 */
vtty::~vtty(void)
{
  delete (text_pipe);
}

/*-------------------------------------------------------------------
 * vtty::destroy
 *
 */
void vtty::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vtty_destroy" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vtty::clear
 */
void vtty::clear(void)
{
  text_pipe->clear();
}
