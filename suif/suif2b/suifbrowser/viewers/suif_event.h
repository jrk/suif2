/*-------------------------------------------------------------------
 * suif_event.h
 *
 */

#ifndef SUIF_EVENT_H
#define SUIF_EVENT_H


enum suif_event_kinds {
  NULL_SUIF_EVENT = VISUAL_USER_EVENT,

  REFRESH,			// Redraw window

  NEW_FILESET,			// A new fileset is loaded.
  CLOSE_FILESET,		// The current fileset is closed.

  PROC_MODIFIED,		// A proc has been modified
  FSE_MODIFIED,			// A file set has been modified

  SUIF_USER_EVENT = VISUAL_USER_EVENT + 256
};

#define SUIF_EVENTS  (NULL_SUIF_EVENT)

#endif
