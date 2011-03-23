/*--------------------------------------------------------------------
 * vprop.h
 *
 * The vprop class allows a visual properties to be defined on
 * vnodes. Each vprop object contains a list of the vnodes that have
 * that particular property. The vprop object defines the visual
 * attributes such as foreground, background colors. The vnodes also
 * contain back-pointers to the vprop objects.
 *
 * The "add_node" and "remove_node" methods modify the vnode list.
 * The "update" method must be called to update the visual attributes.
 * This method post a global PROP_CHANGE event, so that all widgets
 * can update the display.
 *
 */

#ifndef VPROP_H
#define VPROP_H

#include "vnode.h"
#include "common/suif_list.h"

#define DEFAULT_FOREGROUND "black"
#define DEFAULT_BACKGROUND "#F0C0C0"


class vwidget;
class vprop;
typedef list<vprop*> vprop_list;

class vprop {
private:
  friend class vwidget;
  friend class vnode;

  char *nm;
  char *desc;

  /* display attributes */
  char *foreground;
  char *background;

  /* misc information */
  vnode_list *nodes;		// the nodes that has this property
  void *client_data;

public:
  vprop(char *name = "no-name", void *data = 0);
  ~vprop(void);

  void set_description(char *s) { desc = s; }

  char *name(void) { return nm; }
  char *description(void) { return desc; }

  /* display attributes */
  char *get_foreground(void) { return foreground; }
  char *get_background(void) { return background; }

  void set_foreground(char *color) { foreground = color; }
  void set_background(char *color) { background = color; }

  /* node list */
  vnode_list *get_node_list(void) { return nodes; }

  /* add/remove nodes */
  void erase(void);
  void add_node(vnode *vn);
  void remove_node(vnode *vn);

  /* update */
  void update(void);

  /* get client data */
  void set_client_data(void *dat) { client_data = dat; };
  void *get_client_data(void) { return client_data; };
};



#endif
