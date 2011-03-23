/*-------------------------------------------------------------------
 * vmenu.cc
 *
 */

#include "vmenu.h"
//#include <sty.h>
#include "vtcl.h"
#include "vcommands.h"
#include "binding.h"
#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include "suifkernel/suifkernel_forwarders.h"

/*-------------------------------------------------------------------
 * vmenu::vmenu
 *
 */
vmenu::vmenu(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());

  /* create menu widget */
  tcl << "vmenu_create" << wpath << this << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::~vmenu
 *
 */
vmenu::~vmenu(void)
{
}

/*-------------------------------------------------------------------
 * vmenu::destroy
 *
 */
void vmenu::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vmenu_destroy" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vmenu::add_command
 *
 */
void vmenu::add_command(binding *b, const char *menupath, const char *text,
			const char *accelerator)
{
  tcl << "vmenu_add_command" << wpath << this << menupath << accelerator <<
    text << b << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::add_radio
 *
 * Add a radio button
 */
void vmenu::add_radio(binding *b, char *menupath, char *text, bool on)
{
  tcl << "vmenu_add_radio" << wpath << this << menupath << "" << text <<
    b << on << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::add_check
 *
 * Add a check box
 */
void vmenu::add_check(binding *b, char *menupath, char *text, bool on)
{
  tcl << "vmenu_add_check" << wpath << this << menupath << "" << text
    << b << on << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::add_separator
 *
 */
void vmenu::add_separator(char *menupath)
{
  tcl << "vmenu_add_separator" << wpath << this << menupath << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::add_menu
 *
 */
void vmenu::add_menu(binding *b, char *menupath)
{
  tcl << "vmenu_add_menu" << wpath << this << menupath <<
    b << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::clear
 *
 */
void vmenu::clear(char *menupath)
{
  tcl << "vmenu_clear" << wpath << menupath << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::remove
 *
 */
void vmenu::remove(char *menupath)
{
  tcl << "vmenu_remove" << wpath << menupath << tcl_end;
}

/*-------------------------------------------------------------------
 * vmenu::invoke
 *
 */
void vmenu::invoke(char *menupath, char *item_name)
{
  tcl << "vmenu_invoke" << wpath << menupath << item_name <<tcl_end;
}

/*--------------------------------------------------------------------*/
/* tk command:
 * vmenu invoke <vmenu> <binding>
 * vmenu destroy <vmenu>
 */
int TCLTK_CALLING_CONVENTION vmenu::vmenu_cmd(ClientData, Tcl_Interp *interp, int argc,
			 char *argv[])
{
  static char *firstargs[] = {"invoke", "destroy", "destroy_item", 0};
  enum { INVOKE = 0, DESTROY, DESTROY_ITEM };

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  switch (a) {

  case INVOKE:
    {
      if (argc != 4) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }

      vmenu *menu;
      binding *b;
      sscanf(argv[2], "%p", &menu);
      sscanf(argv[3], "%p", &b);

      if (b) {
	b->invoke(event(0, INVOCATION, menu));
      }
    }
    break;

  case DESTROY:
    {
      /* menu bar is destroyed */

      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vmenu *menu;
      sscanf(argv[2], "%p", &menu);
      menu->destroy();
    }
    break;

  case DESTROY_ITEM:
    {
      /* menu item is destroyed, now delete the associated binding */

      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      binding *b;
      sscanf(argv[2], "%p", &b);
      if (b) {
	delete (b);
      }
    }
    break;

  default:
    break;
  }

  return (TCL_OK);
}

/*-------------------------------------------------------------------
 * virtual_menu::virtual_menu
 *
 */
virtual_menu::virtual_menu(void) {
  par_menu = 0;
  root_path = 0;
}

/*-------------------------------------------------------------------
 * virtual_menu::attach
 *
 * attach to a real menu
 */
void virtual_menu::attach(vmenu *parent_menu, char *path) {
  par_menu = parent_menu;
  root_path = (char*)LString(path).c_str();
}

/*-------------------------------------------------------------------
 * virtual_menu::get_menu_path
 *
 */
char *virtual_menu::get_menu_path(const char *menu_str)
{
  char *menupath = new char[strlen(menu_str) + strlen(root_path) +1];
  sprintf(menupath, "%s/%s", root_path, menu_str);

  return (menupath);
}

/*-------------------------------------------------------------------
 * virtual_menu::add_command
 *
 */
void virtual_menu::add_command(binding *b, const char *menu, const char *text,
			   const char *accelerator)
{
  if (par_menu) {
    char *menupath = get_menu_path(menu);
    par_menu->add_command(b, menupath, text, accelerator);
    delete (menupath);
  }
}

/*-------------------------------------------------------------------
 * virtual_menu::add_radio
 *
 */
void virtual_menu::add_radio(binding *b, char *menu, char *text)
{
  if (par_menu) {
    char *menupath = get_menu_path(menu);
    par_menu->add_radio(b, menupath, text);
    delete (menupath);
  }
}

/*-------------------------------------------------------------------
 * vmenu::add_check
 *
 */
void virtual_menu::add_check(binding *b, char *menu, char *text, bool on)
{
  if (par_menu) {
    char *menupath = get_menu_path(menu);
    par_menu->add_check(b, menupath, text, on);
    delete (menupath);
  }
}

/*-------------------------------------------------------------------
 * virtual_menu::add_separator
 *
 */
void virtual_menu::add_separator(char *menu)
{
  if (par_menu) {
    char *menupath = get_menu_path(menu);
    par_menu->add_separator(menupath);
    delete (menupath);
  }
}

/*-------------------------------------------------------------------
 * virtual_menu::clear
 *
 */
void virtual_menu::clear(char *menu)
{
  if (par_menu) {
    char *menupath = get_menu_path(menu);
    par_menu->clear(menupath);
    delete (menupath);
  }
}
