/*-------------------------------------------------------------------
 * graph_layout.cc
 *
 * graph layout module
 */


#include "graph_layout.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <tcl.h>
#include "suifkernel/suifkernel_forwarders.h"


#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))


#define DPRIORITY 32		// a heuristic
#define FAN_SPACE_FACTOR 2

/*-----------------------------------------------------------------
 * is_member function that finds out if an element is present in a
 * list.
 */
//template <class T1>
bool is_member(layout_node_list *l, layout_node *key)
{
  list<layout_node*>::iterator current = l->begin();
  list<layout_node*>::iterator last = l->end();
  int count = 0;

  while(current != last)
  {
   if(*current == key)
     return true;
   current++;
   count++;
  }
  return false;
}

/*-------------------------------------------------------------------
 * layout_node
 */
layout_node::layout_node(void *node_id)
{
  client_id = node_id;
  succs = new layout_node_list;
  preds = new layout_node_list;
}

layout_node::~layout_node(void)
{
  delete succs;
  delete preds;
}

/*------------------------------------------------------------------
 * Find how connected this node is, this is a heuristic..
 *
 */
int layout_node::find_connectivity(layout_node_list *visited)
{
  s_count_t i;

  //if (visited->is_member(this)) return (0);
  if (is_member(visited, this)) return (0);
  visited->push_back(this);

  int num = 0;
  
#if 1
  for ( i=0; i<succs->size(); i++ ) {
    layout_node *succ = (*succs)[i] ;
    if (succ->parent == this) {
      num += succ->find_connectivity(visited);
    } else {
      num++;
    }
  }
#endif

  for ( i=0; i<preds->size(); i++ ) {
    layout_node *pred = (*preds)[i];
    if (pred != parent) {
      num++;
    }
  }
  return (num);
}

/*-------------------------------------------------------------------
 * graph_layout
 */
graph_layout::graph_layout(void)
{
  nodes = new layout_node_list;
  root = 0;
  config.xspacing = 20;
  config.yspacing = 20;
  config.xoffset = 20;
  config.yoffset = 20;

  num_nodes = 0;
}

graph_layout::~graph_layout(void)
{
  while (!nodes->empty()) {
    delete nodes->front();
    nodes->pop_front();
  }
  delete nodes;
}

/*-------------------------------------------------------------------
 * add node
 */
void graph_layout::add_node(void *node_id, layout_geometry &geom)
{
  layout_node *new_node = new layout_node(node_id);
  new_node->geom = geom;

  nodes->push_back(new_node);
  num_nodes++;
}

/*-------------------------------------------------------------------
 * find node given the client id
 */
layout_node *graph_layout::find_node(void *client_id)
{

  for ( s_count_t i=0; i<nodes->size(); i++ ) {
    layout_node *n = (*nodes)[i];
    if (n->client_id == client_id) {
      return (n);
    }
  }
  return (0);
}

/*-------------------------------------------------------------------
 * add edge
 */
void graph_layout::add_edge(void *node_id1, void *node_id2)
{
  layout_node *node1 = find_node(node_id1);
  layout_node *node2 = find_node(node_id2);
  node1->succs->push_back(node2);
  node2->preds->push_back(node1);
}

/*-------------------------------------------------------------------
 * set root of the graph
 */
void graph_layout::set_root(void *node_id)
{
  root = find_node(node_id);
}

/*-------------------------------------------------------------------
 * get bounding box of node
 */
layout_geometry &graph_layout::get_node_bbox(void *node_id)
{
  layout_node *n = find_node(node_id);
  return (n->geom);
}

/*-------------------------------------------------------------------
 * layout in a tree structure
 *
 * Algorithm:
 * 1) Do a topological sort, determine the depth of each node
 * 2) set the y position of each node, do it in a post-order traversal
 * 3) set the x position of each node according to its depth
 *
 */

int graph_layout::layout_tree(void)
{
  s_count_t _cnt;

  /* topological sort */
  toposort();

  /* initialize layout priorities */
 
  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];
    layout_node_list visited;

    // this is just a heuristic
    n->priority = - n->find_connectivity(&visited);

#if 0
    gnode *gn = (gnode *) n->client_id;
    printf("%s : %d\n", gn->get_text(), n->priority);
#endif

  }

  /* layout in y direction */
  reset_visited();

  for (int i = 0; i < MAX_DEPTH; i++) {
    max_ypos[i] = config.yoffset;
  }

  if (root) {
    layout_tree_y(root, config.yoffset);
  }

  layout_node_list top_level_nodes;

  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];
    if (n->depth == 0) {
      top_level_nodes.push_back(n);
    }
  }

  while (1) {
    int max_priority = -10000;
    layout_node *next_node = 0;

    for ( _cnt=0; _cnt<top_level_nodes.size(); _cnt++ ) {
    //layout_node *n = top_level_nodes.elem( _cnt );
    layout_node *n = top_level_nodes[_cnt];
      
      if (!n->visited &&
	  n->priority > max_priority) {
	max_priority = n->priority;
	next_node = n;
      }
    }
    if (next_node == 0) break;
    
    layout_tree_y(next_node, config.yoffset);
  }
  
  /* layout in x direction */
  layout_tree_x(config.xoffset);

  return (0);
}

void graph_layout::layout_tree_y(layout_node *n, int yoffset)
{
  layout_node *node;

  if (n->visited == false) {
    n->visited = true;

    int ymin = INT_MAX;		// keep track of min and max y pos of succs
    int ymax = 0;
    int depth = n->depth;

    /* set the lower bound of y coordinate for this subtree */
    int yspace = find_space(n);
    yspace = MAX(yspace, yoffset);

    /* layout the children, in order of priority */
    while (1) {
      int max_priority = -10000;
      node = 0;

    for ( s_count_t i=0; i<n->succs->size(); i++ ) {
      layout_node *child = (*(n->succs))[i];
   
	if (!child->visited &&
	    child->parent == n &&
	    child->priority > max_priority) {
	  max_priority = child->priority;
	  node = child;
	}
      }
      if (node == 0) break;

      layout_tree_y(node, yspace);
      ymin = MIN(ymin, node->geom.y);
      ymax = MAX(ymax, node->geom.y + node->geom.height);
    }

    int extra_y_spacing = FAN_SPACE_FACTOR *
      (n->preds->size() + n->succs->size());

    /* now layout the node, set y-coord of the node */
    int ycenter;
    if (ymin == INT_MAX) {
      ycenter = 0;
    } else {
      ycenter = (ymin + ymax - n->geom.height)/2;
    }
    int ypos = MAX(max_ypos[depth] + extra_y_spacing, ycenter);
    ypos = MAX(yspace, ypos);
    n->geom.y = ypos;
    ypos += n->geom.height;

    max_ypos[depth] = ypos + config.yspacing + extra_y_spacing;

    /* increase priority of nodes that are connected to this node */
    layout_node_list visited;
    increase_forest_priority(n, DPRIORITY, visited);
  }
}

void graph_layout::layout_tree_x(int xoffset)
{
  s_count_t _cnt;
  int max_width = 0;

  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];
    max_width = MAX(max_width, n->geom.width);
  }

#if 0
   for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];
    n->geom.x = n->depth * (max_width + config.xspacing) + xoffset;
  }
#endif

#if 1
  int i;
  int max_depth = 0;

  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];
    max_depth = MAX(max_depth, n->depth);
  }

  layout_node_list **nodelists = new layout_node_list*[max_depth + 1];
  for (i = 0; i <= max_depth; i++) { 
    nodelists[i] = new layout_node_list;
  }
  int *xpos = new int[max_depth + 1];

  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt]; 
    nodelists[n->depth]->push_back(n);
  }

  int delta_x = (max_width + config.xspacing);

  for (i = 0; i <= max_depth; i++) {

    int x;

    /* find a good x position for nodes at depth i */

    if (i == 0) {
      x = xoffset;
    } else {

      int x0 = xpos[i-1] + delta_x;

      float max_gradient = 3;
      int max_gradient_xdiff = delta_x/3;

      for ( _cnt=0; _cnt<nodelists[i]->size(); _cnt++ ) {
        layout_node *n = (*nodelists[i])[_cnt];   
	
	for (int j = 0; j < 2; j++) {
          layout_node_list *the_list;
          the_list = (j==0) ? n->preds : n->succs;
          for ( s_count_t k=0; k<the_list->size(); k++ ) {
            layout_node *p = (*the_list)[k];	 

	    if (p->depth < i) {
	      int xdiff = x0 - xpos[p->depth];
	      float grad = (p->geom.y - n->geom.y)/xdiff;
	      if (grad < 0) grad = -grad;
	      if (grad > max_gradient) {
		max_gradient = grad;
		max_gradient_xdiff = xdiff;
	      }
	    }
	  }
	}
      }

      int dx = (int)(max_gradient * max_gradient_xdiff);
      int max_dx = max_width + 8 * config.xspacing;
      dx = MIN(dx, max_dx);
      x = xpos[i-1] + dx;
    }

    /* now set the x-coordinates of nodes at depth i */

    xpos[i] = x;

    for ( _cnt=0; _cnt<nodelists[i]->size(); _cnt++ ) {
      layout_node *n = (*nodelists[i])[_cnt];   
      n->geom.x = x;
    }

  }

  for (i = 0; i <= max_depth; i++) {
    delete nodelists[i];
  }
  delete nodelists;
  delete xpos;

#endif
}

/*
 * find y space for a subtree at node n, i.e.
 * find the max y of already-allocated nodes
 *
 */
int graph_layout::find_space(layout_node *n)
{
  int maxy = max_ypos[n->depth];

  for ( s_count_t _cnt=0; _cnt<n->succs->size(); _cnt++ ) {
    layout_node *child = (*n->succs)[_cnt];   

    /* for the adjacent successor */
    if (child->parent == n) {
      int child_maxy = find_space(child);
      maxy = MAX(maxy, child_maxy);
    }
    /* for succs that is higher up in the tree (i.e. there's a cycle) */
    if (child->depth < n->depth) {
      for (int d = child->depth; d < n->depth; d++) {
	maxy = MAX(maxy, max_ypos[d]);
      }
    }
  }
  return (maxy);
}

/*
 * find max depth of tree
 *
 */
int graph_layout::find_max_depth(layout_node *n)
{
  int max_depth = n->depth;

  for ( s_count_t _cnt=0; _cnt<n->succs->size(); _cnt++ ) {
    layout_node *child = (*n->succs)[_cnt];   
    if (child->depth > n->depth) {
      int child_max_depth = find_max_depth(child);
      max_depth = MAX(max_depth, child_max_depth);
    }
  }
  return (max_depth);
}

#if 0
/*
 * find number of descendents that have been visited
 *
 */
int graph_layout::find_num_visited_des(layout_node *n)
{
  int num = 0;
  for ( s_count_t _cnt=0; _cnt<n->succs->size(); _cnt++ ) {
    layout_node *child = (*n->succs)[_cnt];  
    if (child->parent == n) {
      if (!child->visited) {
	num += num_visited_des(child) + 1;
      }
    }
  }
  return (num);
}
#endif

/*
 * increase priority of nodes that are reachable from node n
 *
 */
void graph_layout::increase_forest_priority(layout_node *n, int delta,
					    layout_node_list &visited)
{
  //if (delta <= 0 || visited.is_member(n)) {
  if (delta <= 0 || is_member(&visited, n)) {
    return;
  }
  visited.push_back(n);

  layout_node_list *the_list;
 
  for (int j = 0; j < 2; j++) {
    the_list = (j == 0 ) ? n->succs : n->preds;
    for ( s_count_t _cnt=0; _cnt<the_list->size(); _cnt++ ) {
      layout_node *adj = (*the_list)[_cnt];  
   
      if (!adj->visited && adj->parent != n) {
	adj->priority += delta;
	increase_forest_priority(adj, delta - 2, visited);
      }
    }
  }
}

/*-------------------------------------------------------------------
 * topological sort
 *
 */

void graph_layout::toposort(void)
{
  s_count_t _cnt;

  reset_visited();
  reset_depth();
  reset_priority();

  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
      layout_node *n = (*nodes)[_cnt];  

    /* if this node has no predecessors */
    if (n->preds->empty()) {
      topo_node_sort(n, 0, 0);
    }
  }

  /* now, check those nodes that form a cycle and are not visited */


  for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];

    if (n->visited == false) {
      topo_node_sort(n, 0, 0);
    }
  }

}

void graph_layout::topo_node_sort(layout_node *n, layout_node *parent, 
				  int depth)
{
  if (n->visited == false ||
      n->depth < depth) {

    n->visited = true;
    n->depth = depth;
    n->parent = parent;

    /* visit successors */
    for ( s_count_t _cnt=0; _cnt<n->succs->size(); _cnt++ ) {
      layout_node *child = (*n->succs)[_cnt];  
      if (!cycle_exists(n, child)) {
	topo_node_sort(child, n, depth + 1);
      }
    }
  }
}

/*
 * check if a cycle exists
 * returns true if "child" is an ancestor of "n"
 */

bool graph_layout::cycle_exists(layout_node *n, layout_node *child)
{
  for (layout_node *node = n; node; node = node->parent) {
    if (node == child) {
      return (true);
    }
  }
  return (false);
}

/*-------------------------------------------------------------------
 * misc
 */

void graph_layout::reset_visited(void)
{
  for ( s_count_t _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];  
    n->visited = 0;
  }
}

void graph_layout::reset_depth(void)
{
  for ( s_count_t _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];  
    n->depth = 0;
  }
}

void graph_layout::reset_priority(void)
{
  for ( s_count_t _cnt=0; _cnt<nodes->size(); _cnt++ ) {
    layout_node *n = (*nodes)[_cnt];  
    n->priority = 0;
  }
}
