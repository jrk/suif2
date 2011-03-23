/*-------------------------------------------------------------------
 * vlistbox
 *
 */

#include "vtcl.h"
#include "vlistbox.h"
#include "vcommands.h"
#include "binding.h"
#include "event.h"
//#include <sty.h>
#include <stdio.h>
#include <stdlib.h>
#include "iokernel/helper.h"

/*-------------------------------------------------------------------
 * vlistbox::vlistbox
 *
 */
vlistbox::vlistbox(vwidget *par, char *title, bool horizontal_slider ) : vwidget(par)
{
  strcpy(wpath, par->path());
  tcl << "vlistbox_create" << wpath << title << this << 
         (int)horizontal_slider<< tcl_end;
}

/*-------------------------------------------------------------------
 * vlistbox::~vlistbox
 *
 */
vlistbox::~vlistbox(void)
{
  int n = entries.size();
    for (int i = 0; i < n; i++) {
      listbox_entry *entry = entries[i];
      delete entry;
    }
}

 /*-------------------------------------------------------------------
 * vlistbox::destroy
 *
 */
void vlistbox::destroy(void)
{
  tcl << "vlistbox_destroy" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vlistbox::add
 *
 */
void vlistbox::add(binding *b, vnode *object, char *text)
{
  tcl << "vlistbox_add" << wpath << text << tcl_end;

  listbox_entry *entry = new listbox_entry(object, b);
  entries.push_back( entry );
}

/*-------------------------------------------------------------------
 * vlistbox::clear
 *
 */
void vlistbox::clear(void)
{
 int n = entries.size();
  for (int i = 0; i < n; i++) {
    listbox_entry *entry = entries[i];
    delete entry;
  }
  for (int i = 0; i < n; i++) 
    entries.pop_front();

  tcl << "vlistbox_clear" << wpath << tcl_end;
}

/*--------------------------------------------------------------------*/
/* tk command
 */
int TCLTK_CALLING_CONVENTION vlistbox::vlistbox_cmd(ClientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
  static char *firstargs[] = {"select", "destroy", 0};
  enum { SELECT = 0, DESTROY};

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }

  vlistbox *listbox;
  sscanf(argv[2], "%p", &listbox);

  switch (a) {
  case SELECT:
    {
      if (argc < 4) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      int index;
      sscanf(argv[3], "%d", &index);

      if (index < (int)listbox->entries.size()) {

	listbox_entry *entry = (listbox->entries)[index];
	vnode *vn = entry->object;
	binding *b = entry->bd;
	event e = event(vn, SELECTION, listbox);
	if (b) {
	  b->invoke(e);
	}
	post_event(e);
      }
    }
    break;

  case DESTROY:
    {
      listbox->destroy();
    }
    break;

  default:
    break;
  }
  return (TCL_OK);
}
