/*----------------------------------------------------------------------
 * viewers.h
 *
 * Viewers for sbrowser header file
 *
 */

#ifndef VIEWERS_H
#define VIEWERS_H

extern char *application_help_text;

void init_viewers();
void enter_viewers(int *, char *[]);
void exit_viewers();
void start_viewers(int argc, char *argv[]);

#endif
