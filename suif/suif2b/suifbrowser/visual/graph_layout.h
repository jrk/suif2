/*
 * graph_layout.h
 *
 * graph layout module
 *
 */

#ifndef G_LAYOUT_H
#define G_LAYOUT_H

#include "common/suif_list.h"

#define MAX_DEPTH 100

struct layout_geometry {
  int x;			// top left corner coordinate
  int y;			// top left corner coordinate
  int width;
  int height;
};

struct layout_config {
  int xspacing;
  int yspacing;
  int xoffset;
  int yoffset;
};

class layout_node;
typedef list<layout_node*> layout_node_list;

class layout_node {

private:
  friend class graph_layout;

  layout_node *parent;
  void *client_id;

  layout_geometry geom;
  layout_node_list *succs;
  layout_node_list *preds;

  //boolean visited;		// for misc purposes
  bool visited;	        	// for misc purposes
  int depth;			// depth in the tree (used in layout_tree)
  int priority;			// priority level (used in layout_tree)

public:
  layout_node(void *node_id);
  ~layout_node(void);

  int find_connectivity(layout_node_list *visited);
};




class graph_layout {

private:
  int num_nodes;

  layout_node_list *nodes;
  layout_node *root;
  layout_config config;

  int max_ypos[MAX_DEPTH];	// used for layout_tree

  void reset_visited(void);
  void reset_depth(void);
  void reset_priority(void);
  layout_node *find_node(void *node_id);

  void toposort(void);
  void topo_node_sort(layout_node *n, layout_node *parent, int depth);

  void layout_tree_y(layout_node *n, int yoffset);
  void layout_tree_x(int xoffset);

  int find_space(layout_node *n);
  int find_max_depth(layout_node *n);
  //boolean cycle_exists(layout_node *n, layout_node *child);
  bool cycle_exists(layout_node *n, layout_node *child);
  static void increase_forest_priority(layout_node *n, int delta,
				       layout_node_list &visited);

public:
  graph_layout(void);
  ~graph_layout(void);

  void add_node(void *node_id, layout_geometry &geom);
  void add_edge(void *node_id, void *node_id2);

  void set_root(void *node_id);
  void configure(layout_config &c) {
    config = c;
  }

  int layout_tree(void);

  layout_geometry &get_node_bbox(void *node_id);
};

#endif
