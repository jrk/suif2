/*--------------------------------------------------------------------
 * vgraph.h
 *
 */

#ifndef VGRAPH_H
#define VGRAPH_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"
#include <string.h>
#include "common/suif_list.h"
class binding;

#define NODE_TEXT_LEN 40

enum arrow_dir {
  ARROW_NONE = 0,
  ARROW_FORWARD,
  ARROW_BOTH
};

// layout methods
#define LAYOUT_DEFAULT  "default"
#define LAYOUT_TREE     "tree"
#define LAYOUT_DOT      "dot"

struct item_geom {
  int x1, y1;			/* top left corner */
  int x2, y2;			/* bottom right corner */
};

/*
 * gnode
 */
class gnode {
private:
  friend class vgraph;

  int canvas_id;
  char text[NODE_TEXT_LEN + 1];

  gnode(char *_text) {
    strncpy(text, _text, NODE_TEXT_LEN);
    text[NODE_TEXT_LEN] = '\0';
  }

  int x;			/* for layout */
  int y;

public:
  vnode *object;
  char *get_text(void) { return text; }

};

/*
 * gedge
 */
class gedge {
private:
  friend class vgraph;

  int canvas_id;
  gnode *node1;
  gnode *node2;
  arrow_dir arrow;

  gedge(gnode *_node1, gnode *_node2, arrow_dir _arrow) {
    node1 = _node1;
    node2 = _node2;
    arrow = _arrow;
  }
public:
  vnode *object;
  gnode *get_node1(void) const { return node1; }
  gnode *get_node2(void) const { return node2; }
  arrow_dir get_arrow_dir(void) const { return arrow; }
};

//DECLARE_LIST_CLASS(gnode_list, gnode *);
//typedef slist_tos<gnode*> gnode_list;
//typedef slist_tos<gedge*> gedge_list;
typedef list<gnode*> gnode_list;
typedef list<gedge*> gedge_list;
//DECLARE_LIST_CLASS(gedge_list, gedge *);


/*
 * vgraph class
 *
 */

class vgraph : public vwidget {

protected:
  gnode_list *nodes;
  gedge_list *edges;
  gnode *root_node;

  binding *inv_binding;
  vnode *current_sel;

  int current_layout;

  void new_node_pos(int &x, int &y);
  void layout_graph(char *method_name);
  gnode *get_node(int canvas_id);
  void layout_dot(char *filename);

public:
  vgraph(vwidget *par);
  ~vgraph(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_GRAPH; }

  void clear(void);

  /* graph construction methods */
  gnode *add_node(char *text, vnode *obj,
		  int pos_x = -100, int pos_y = -100);
  gedge *add_edge(gnode *node1, gnode *node2, arrow_dir arrow,
		  vnode *obj = 0);

  gnode *get_root_node(void) { return root_node; }
  void set_root_node(gnode *node) { root_node = node; }

  /* nodes, edges */
  gnode *get_node(vnode *obj);
  gedge *get_edge(vnode *obj);

  /* layout */
  void layout(char *method_name = LAYOUT_DEFAULT);
  item_geom get_node_geometry(gnode *n);

  void set_node_size(gnode *n, int width, int height, bool update_edges);
  void place_node(gnode *n, int x, int y, bool update_edges = true);
  void place_edge(gedge *e, int x1, int y1, int x2, int y2);
  void place_edge(gedge *e, int num_points, int *xarray,
		  int *yarray);

  /* bindings */
  void set_binding(binding *b);
  void invoke(vnode *vn);

  /* view */
  void view(gnode *node);

  /* selection */
  virtual vnode *get_selection(void);
  void select(vnode *vn);
  void select_clear(void);

  /* export */
  void export_dot(FILE *fp);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vgraph_cmd(ClientData clientData, Tcl_Interp *interp, int argc,
			char *argv[]);

};

#endif
