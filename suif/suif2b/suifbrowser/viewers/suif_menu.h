#ifndef SUIF_MENU_H
#define SUIF_MENU_H

#include "visual/visual.h"

/*
 * Standard menus
 */

void add_std_fse_menu(  vmenu* root_menu, char* parent_menu );
void add_std_proc_menu( vmenu* root_menu, char* parent_menu );
void add_std_edit_menu( vmenu* root_menu, char* parent_menu );
void add_std_go_menu(   vmenu* root_menu );

#endif
