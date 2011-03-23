/*-------------------------------------------------------------------
 * vgraph
 *
 */

#include "vtcl.h"
#include "vgraph.h"
#include "vcommands.h"
#include "event.h"
#include "binding.h"
#include "vmisc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include "graph_layout.h"

static char *dir_str[] = {"none", "last", "first", "both"};

enum layout_method {
  DEFAULT = 0, TREE, DOT
};

#define DOT_SCALE 72


/*-------------------------------------------------------------------
 * vgraph::vgraph
 *
 */
vgraph::vgraph(vwidget *par) : vwidget(par)
{
  strcpy(wpath, par->path());

  nodes = new (gnode_list);
  edges = new (gedge_list);
  root_node = 0;
  inv_binding = 0;
  current_sel = 0;
  current_layout = TREE;

  tcl << "vgraph_create" << wpath << this << tcl_end;
}

/*-------------------------------------------------------------------
 * vgraph::~vgraph
 *
 */
vgraph::~vgraph(void)
{

  while (!nodes->empty()) {
    delete nodes->front();
    nodes->pop_front();
  }

  while (!edges->empty()) {
    delete edges->front(); 
    edges->pop_front();
  }

  delete(nodes);
  delete(edges);
}

/*-------------------------------------------------------------------
 * vgraph::destroy
 *
 */
void vgraph::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vgraph_destroy" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vgraph::clear
 *
 */
void vgraph::clear(void)
{
 while (!nodes->empty()) {
    delete nodes->front(); 
    nodes->pop_front();
  }

  while (!edges->empty()) {
    delete edges->front();
    edges->pop_front();
  }

  root_node = 0;
  current_sel = 0;

  tcl << "vgraph_clear" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vgraph::add_node
 *
 */
gnode *vgraph::add_node(char *text, vnode *obj, int pos_x, int pos_y)
{
  gnode *n = new gnode(text);
  n->object = obj;
  nodes->push_back(n);

  if (pos_x == -1) {
    new_node_pos(pos_x, pos_y);
  }
  tcl << "vgraph_add_node" << wpath << text << pos_x <<
    pos_y << tcl_end;
  tcl >> n->canvas_id;

  return(n);
}

/*-------------------------------------------------------------------
 * vgraph::add_edge
 *
 */
gedge *vgraph::add_edge(gnode *node1, gnode *node2, arrow_dir dir,
			  vnode *obj)
{
  gedge *e = new gedge(node1, node2, dir);
  e->object = obj;
  edges->push_back(e);

  tcl << "vgraph_add_edge" << wpath << node1->canvas_id <<
    node2->canvas_id << dir_str[dir] << tcl_end;
  tcl >> e->canvas_id;

  return (e);
}

/*-------------------------------------------------------------------
 * vgraph::new_node_pos
 *
 */
void vgraph::new_node_pos(int &x, int &y)
{
  x = -100;
  y = -100;
}

/*-------------------------------------------------------------------
 * vgraph::get_node_geometry
 *
 */
item_geom vgraph::get_node_geometry(gnode *n)
{
  item_geom geom;

  tcl << "vgraph_get_bbox" << wpath << n->canvas_id << tcl_end;
  sscanf(tcl.result(), "%d %d %d %d",
	 &geom.x1, &geom.y1, &geom.x2, &geom.y2);
  return geom;
}

/*-------------------------------------------------------------------
 * vgraph::select
 *
 */
void vgraph::select(vnode *vn)
{
  gnode *n = get_node(vn);
  if (n) {
    tcl << "vgraph_select" << wpath << n->canvas_id << tcl_end;
  } else {
    select_clear();
  }
}

/*-------------------------------------------------------------------
 * vgraph::select_clear
 *
 */
void vgraph::select_clear(void)
{
  tcl << "vgraph_select_clear" << wpath << tcl_end;
}

/*--------------------------------------------------------------------*/
/*
 * parse string
 *
 */
static int parse_string(char *token_array[], char *str)
{
  for (int i = 0; token_array[i]; i++) {
    if (strcmp(str, token_array[i]) == 0) {
      return (i);
    }
  }
  return (-1);
}

/*-------------------------------------------------------------------
 * vgraph::layout
 *
 */
void vgraph::layout(char *methodname)
{
    tcl << "vgraph_layout" << wpath << this << methodname << tcl_end;
}

// This is an internal function
void vgraph::layout_graph(char *methodname)
{
  char *methods[] =
    {"default", "tree", "dot", 0};

  int method = parse_string(methods, methodname);

  if (method == DEFAULT) {
    method = current_layout;
  }

  switch (method) {

  case TREE:
    {
      graph_layout *layout = new graph_layout;
      s_count_t _cnt;

      /* add nodes and edges */
      for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
	gnode *n = (*nodes)[_cnt];

	item_geom geom = get_node_geometry(n);

	layout_geometry node_geom;
	node_geom.x = geom.x1;
	node_geom.y = geom.y1;
	node_geom.width = geom.x2 - geom.x1;
	node_geom.height = geom.y2 - geom.y1;
	layout->add_node(n, node_geom);
      }

      for ( _cnt=0;  _cnt<edges->size(); _cnt++ ) {
	gedge *e = (*edges)[_cnt];
	layout->add_edge(e->node1, e->node2);
      }

      if (root_node) {
	layout->set_root(root_node);
      }

      /* configure */
      layout_config config = {20, 10, 20, 20};
      layout->configure(config);
      layout->layout_tree();

      /* update node positions */
      for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
	gnode *n = (*nodes)[_cnt];

	layout_geometry node_geom = layout->get_node_bbox(n);
	place_node(n, node_geom.x, node_geom.y);
      }

      delete layout;
    }
    break;

  case DOT:
    {
      /*
       * Export the graph to a temporary .dot file, call 'dot' to
       * output a layout file, read it in, and lay out the graph.
       */

      char dotfile[50], outfile[50];
      sprintf(dotfile, "/tmp/~vgraph_dot_%05lu", (unsigned long)getpid());
      sprintf(outfile, "/tmp/~vgraph_out_%05lu", (unsigned long)getpid());

      FILE *fd = fopen(dotfile, "w");
      if (!fd) {
	display_message(win(), "Error: Cannot create temporary .dot file");
	break;
      }
      export_dot(fd);
      fclose(fd);

      char command_buffer[100];
      sprintf(command_buffer,
	      "dot -Tplain -Nshape=box -Gsize=50,50 %s > %s;",
	      dotfile,
	      outfile);

      int result = system(command_buffer);

      if (result != 0) {
	display_message(win(),
			"Cannot find `dot' command, or its execution failed");
	break;
      }

      layout_dot(outfile);
      unlink(dotfile);
      unlink(outfile);
    }
    break;
  default:
    {
      /* error */
      v_warning("Unknown layout method.");
      return;
    }
  }

  current_layout = method;
}

/*----------------------------------------------------------------------
 * vgraph::layout_dot
 *
 * layout according to a `dot' output
 */

void vgraph::layout_dot(char *filename)
{
  static char *sep = " \n";

  FILE *fd = fopen(filename, "r");
  if (!fd) return;

  float scale, xsize, ysize;
  fscanf(fd, "graph %f %f %f\n", &scale, &xsize, &ysize);

  bool parse_ok = false;
  while (1) {
    char buffer[1001];
    if (!fgets(buffer, 1000, fd)) break;
    char *p = strtok(buffer, sep);
    if (strcmp(p, "node") == 0) {
      /* a node */
      gnode *node;
      float x1, y1, width, height;

      p = strtok(0, sep);
      if (sscanf(p, "n%p", &node) != 1) break;
      p = strtok(0, sep);

      if (sscanf(p, "%f", &x1) != 1) break;
      p = strtok(0, sep);
      if (sscanf(p, "%f", &y1) != 1) break;
      p = strtok(0, sep);
      if (sscanf(p, "%f", &width) != 1) break;
      p = strtok(0, sep);
      if (sscanf(p, "%f", &height) != 1) break;

      node->x = (int)(x1 * DOT_SCALE);
      node->y = -(int)(y1 * DOT_SCALE);

      place_node(node, node->x, node->y, false);
      set_node_size(node, (int) (width * DOT_SCALE) - 1,
		    (int) (height * DOT_SCALE) - 1, false);

    } else if (strcmp(p, "edge") == 0) {
      /* an edge */
      gnode *node1, *node2;
      int num_points;

      p = strtok(0, sep);
      if (sscanf(p, "n%p", &node1) != 1) break;
      p = strtok(0, sep);
      if (sscanf(p, "n%p", &node2) != 1) break;

      /* find the edge */
      gedge *edge = 0;
      bool found_edge = false;

      for ( s_count_t _cnt=0; _cnt<edges->size(); _cnt++ ) {
	edge = (*edges)[_cnt];
	if (edge->arrow == ARROW_FORWARD &&
	    edge->node1 == node1 &&
	    edge->node2 == node2) {
	  found_edge = true;
	  break;
	}
	if ((edge->arrow == ARROW_BOTH)
	    && edge->node2 == node1 && edge->node1 == node2) {
	  found_edge = true;
	  break;
	}
      }
      if (!found_edge) {
	/* cannot find the edge */
	break;
      }

      p = strtok(0, sep);
      if (sscanf(p, "%d", &num_points) != 1) break;
      int *point_x = new int[num_points];
      int *point_y = new int[num_points];

      int i;
      for (i = 0; i < num_points; i++) {
	float px, py;

	p = strtok(0, sep);
	if (sscanf(p, "%f", &px) != 1) break;
	p = strtok(0, sep);
	if (sscanf(p, "%f", &py) != 1) break;
	
	point_x[i] = (int) (px * DOT_SCALE);
	point_y[i] = -(int) (py * DOT_SCALE);
      }
      if (i < num_points) {
	delete point_x;
	delete point_y;
	break;
      }

      /*
       * Check the direction!
       * For some reason, the plain layout file doesn't have direction
       * information.
       */

      int last = num_points -1;
      if (((point_x[0] - node1->x) * (point_x[0] - node1->x) +
	   (point_y[0] - node1->y) * (point_y[0] - node1->y)) >
	  ((point_x[last] - node1->x) * (point_x[last] - node1->x) +
	   (point_y[last] - node1->y) * (point_y[last] - node1->y))) {

	/* reverse the order of points */
	int j = last;
	for (i = 0; i < j; i++, j--) {
	  int tmp = point_x[i]; point_x[i] = point_x[j]; point_x[j] = tmp;
	  tmp = point_y[i]; point_y[i] = point_y[j]; point_y[j] = tmp;
	}
      }

      place_edge(edge, num_points, point_x, point_y);

      delete point_x;
      delete point_y;

    } else if (strcmp(p, "stop") == 0) {
      parse_ok = true;
      break;
    }
  }

  if (!parse_ok) {
    display_message(win(), "Cannot parse layout file `%s'", filename);
  }
}

/*----------------------------------------------------------------------
 * vgraph::place_edge
 *
 * place an edge, giving (x1,y1)-(x2,y2) coords
 */

void vgraph::place_edge(gedge *e, int x1, int y1, int x2, int y2)
{
  tcl << "vgraph_place_edge" << wpath << e->canvas_id << x1 << y1 <<
    x2 << y2 << tcl_end;
}

void vgraph::place_edge(gedge *e, int num_points, int *xarray,
			int *yarray)
{
  tcl << "vgraph_place_edge_spline" << wpath << e->canvas_id;
  for (int i = 0; i < num_points; i++) {
    tcl << xarray[i] << yarray[i];
  }
  tcl << tcl_end;
}

/*----------------------------------------------------------------------
 * vgraph::set_node_size
 *
 */
void vgraph::set_node_size(gnode *n, int width, int height,
			   bool update_edges)
{
  tcl << "vgraph_set_node_size" << wpath << n->canvas_id << width
    << height << (update_edges ? 1 : 0) << tcl_end;
}

/*----------------------------------------------------------------------
 * vgraph::place_node
 *
 * place node, giving (x,y) coords of the center
 */
void vgraph::place_node(gnode *n, int x, int y, bool update_edges)
{
  tcl << "vgraph_place_node" << wpath << n->canvas_id << x << y <<
    (update_edges ? 1 : 0) << tcl_end;
}

/*-------------------------------------------------------------------
 * vgraph::get_selection
 *
 */
vnode *vgraph::get_selection(void)
{
  return (current_sel);
}

/*-------------------------------------------------------------------
 * vgraph::get_node
 *
 */
gnode *vgraph::get_node(int canvas_id)
{
  for ( s_count_t i=0; i<nodes->size(); i++ ) {
    gnode *n = (*nodes)[i];
    if (n->canvas_id == canvas_id) {
      return (n);
    }
  }
  return (0);
}

gnode *vgraph::get_node(vnode *vn)
{
  for ( s_count_t i=0; i<nodes->size(); i++ ) {
    gnode *n = (*nodes)[i];
    if (n->object == vn) {
      return (n);
    }
  }
  return (0);
}

/*-------------------------------------------------------------------
 * vgraph::get_edge
 *
 */
gedge *vgraph::get_edge(vnode *vn)
{
  for ( s_count_t i=0; i<edges->size(); i++ ) {
    gedge *e = (*edges)[i];
    if (e->object == vn) {
      return (e);
    }
  }
  return (0);
}

/*-------------------------------------------------------------------
 * vgraph::set_binding
 *
 */
void vgraph::set_binding(binding *b)
{
  inv_binding = b;
}

/*-------------------------------------------------------------------
 * vgraph::invoke
 *
 */
void vgraph::invoke(vnode *vn)
{
  if (inv_binding) {
    inv_binding->invoke(event(vn, INVOCATION, this));
  }
}

/*-------------------------------------------------------------------
 * vgraph::view
 *
 */
void vgraph::view(gnode *node)
{
  tcl << "vgraph_view_node" << wpath << node->canvas_id << tcl_end;
}

/*-------------------------------------------------------------------
 * vgraph::export_dot
 *
 */
void vgraph::export_dot(FILE *fp)
{
  s_count_t _cnt;
  fprintf(fp, "digraph gen_graph {\nsize = \"8,10\";\n");

  // Print out nodes names and their labels
    for ( _cnt=0; _cnt<nodes->size(); _cnt++ ) {
      gnode *node = (*nodes)[_cnt];
      fprintf(fp, "n%p [label=\"%s\"];\n",
            node,
	    node->get_text());
  }

  // Print out each connecting edges
  for ( _cnt=0; _cnt<edges->size(); _cnt++ ) {
    gedge *edge = (*edges)[_cnt];
    gnode *node1 = edge->get_node1();
    gnode *node2 = edge->get_node2();

    switch (edge->get_arrow_dir()) {
    case ARROW_NONE:
      fprintf(stderr, "Warning: undirected edge ignored\n");
      break;
    case ARROW_FORWARD:
      fprintf(fp, "n%p -> n%p\n", node1, node2);
      break;
    case ARROW_BOTH:
      fprintf(fp, "n%p -> n%p\n", node1, node2);
      fprintf(fp, "n%p -> n%p\n", node2, node1);
      break;
    }
  }
  fprintf(fp, "}\n");
}

/*--------------------------------------------------------------------*/
/* tk command:
 * vgraph invoke <object address>
 * vgraph destroy <object address>
 * vgraph layout <object address> <method>
 */

int TCLTK_CALLING_CONVENTION vgraph::vgraph_cmd(ClientData, Tcl_Interp *interp,
			   int argc, char *argv[])
{
  static char *firstargs[] = {"select", "invoke", "destroy", "layout",
			      "export_dot", 0};
  enum { SELECT = 0, INVOKE, DESTROY, LAYOUT, EXPORT_DOT };

  int a = v_parse_firstarg(interp, argc, argv, firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }

  vgraph *graph;
  sscanf(argv[2], "%p", &graph);

  switch (a) {

  case SELECT:
    {
      if (argc < 4) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vnode *vn = graph->get_selection();
      (void) vn; // avoid warning
      int canvas_id;
      sscanf(argv[3], "%d", &canvas_id);

      gnode *n = graph->get_node(canvas_id);
      graph->current_sel = 0;
      if (n) {
	graph->current_sel = n->object;
	post_event(event(graph->current_sel, SELECTION, graph));
      }
    }
    break;

  case INVOKE:
    {
      graph->invoke(graph->current_sel);
    }
    break;

  case LAYOUT:
    {
      if (argc < 4) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      graph->layout_graph(argv[3]);
    }
    break;

  case DESTROY:
    {
      graph->destroy();
    }
    break;

  case EXPORT_DOT:
    {
      char filename[100];
      select_file(graph->win(), filename, "Output DOT file:");
      if (filename[0]) {
	FILE *fp = fopen(filename, "w");
	if (fp) {
	  graph->export_dot(fp);
	  fclose(fp);
	} else {
	  display_message(graph->win(),
			  "Cannot write to file `%s'", filename);
	}
      }
    }
    break;

  }
  return (TCL_OK);
}
