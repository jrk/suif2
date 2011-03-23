/*--------------------------------------------------------------------
 * vmodule.h
 *
 * A "module" class allows an external component to be easily written
 * and plugged into the visual system. External modules should be
 * registered in the visual manager.
 *
 */

#ifndef MODULE_H
#define MODULE_H

#include "common/suif_list.h"
class binding;
class virtual_menu;
class vmenu;
class event;

/*----------------------------------------------------------------------
 * module
 */

class module {

private:
  binding *eb;

protected:
  virtual_menu *menu;

public:
  module(void);
  virtual ~module(void);

  virtual char *class_name(void) { return "Module"; }
  virtual void handle_event(event &) {};

  virtual void attach_menu(vmenu *par_menu, char *menu_path);
  virtual void create_menu(void) {};
};


typedef list<module*> module_list;

#endif

