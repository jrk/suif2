/*--------------------------------------------------------------------
 * vmenu.h
 *
 * This class allows menus to be created easily.
 * Menus and submenus are represented by a menu path, in a form similar
 * to a file path name, separated by '/'s, starting from the root menu.
 * e.g. "a/b/c" represents the menu with menubutton labeled "a", and
 * cascade menus "b", and then "c".
 *
 * Cascade menus are automatically created. So for instance, a call to
 * add_command(b, "a/b/c", "test") will create the menubutton "a", and
 * submenus "a/b" and "a/b/c".
 *
 */

#ifndef VMENU_H
#define VMENU_H

#include "tcltk_calling_convention.h"
//#include <sty.h>
#include "vwidget.h"
#include "vtcl.h"
#include "binding.h"

#define ROOT_MENU ""		// The root of the menu.

/*----------------------------------------------------------------------
 * menu class
 */
class vmenu : public vwidget {

public:
  vmenu(vwidget *par);
  ~vmenu(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_MENU; }

  /* add a menu */
  void add_menu(binding *b, char *menu);

  /* add a menu command/radio button/checkbox/separator */
  void add_command(binding *b, const char *menu, const char *text,
		   const char *accelerator = "");
  void add_radio(binding *b, char *menu, char *text, bool on = false);
  void add_check(binding *b, char *menu, char *text, bool on = false);
  void add_separator(char *menu);

  /* clear the menu */
  void clear(char *menu);	/* remove all items in the menu */
  void remove(char *menu);	/* remove the menu */

  /* invoke a menu item */
  void invoke(char *menu, char *item_name);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vmenu_cmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[]);
};


/*----------------------------------------------------------------------
 * virtual menu class
 */
class virtual_menu {

protected:
  vmenu *par_menu;
  char *root_path;

  char *get_menu_path(const char *menu_str);

public:
  virtual_menu(void);
  void attach(vmenu *parent_menu, char *path);

  void add_command(binding *b, const char *menupath, const char *text,
		   const char *accelerator = "");
  void add_radio(binding *b, char *menupath, char *text);
  void add_check(binding *b, char *menupath, char *text, bool on);
  void add_separator(char *menupath);

  void clear(char *menu);
};

#endif
