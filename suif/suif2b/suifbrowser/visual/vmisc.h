/*--------------------------------------------------------------------
 * vmisc.h
 *
 */

#ifndef VMISC_H
#define VMISC_H

//#include <sty.h>

class window;

/*
 * String functions
 */
char *strdup(char *s);

/*
 * Misc functions
 */
bool widget_exists(char *path);
void select_file(window *parent, char *filename, char *text = "Select file:",
		 char *default_filename = "");
char *select_fileset(window *parent, char *text = "Select file:",
		     char *fileset = 0);
void display_message(window *parent, char *mesg ...);
int display_dialog(window *parent, char *mesg, char *options, 
		   int default_option = 0);
void display_query(window *parent, char *mesg, char *result);

/*
 * Progress indicator
 */
void post_progress(window *parent, char *message, float percent_completed);
void unpost_progress(window *parent);

#endif
