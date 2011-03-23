/*-------------------------------------------------------------------
 * cg_viewer
 *
 */

#include "cg_viewer.h"
#include "code_tree.h"
#include "suif_vnode.h"
#include "suif_event.h"
#include "suif_menu.h"
#include "suif_utils.h"
#include <stdlib.h>
#include "suifnodes/suif.h"
#include "iokernel/cast.h"

static char* init_msg = "Initializing call graph ...";

/*--------------------------------------------------------------------
 * cg_viewer::cg_viewer
 */
cg_viewer::cg_viewer() {
  nodes = 0;
  current_cg = 0;
  show_external_procs = true;
}

/*--------------------------------------------------------------------
 * cg_viewer::~cg_viewer
 *
 */
cg_viewer::~cg_viewer() {
  delete nodes;
  delete current_cg;
}

/*--------------------------------------------------------------------
 * cg_viewer::create_window
 */
void cg_viewer::create_window() {
  inherited::create_window();

  create_proc_menu();
  create_view_menu();
  init();
  show ( vman->get_selection() );
}

/*--------------------------------------------------------------------
 * cg_viewer::clear
 */
void cg_viewer::clear() {
  delete nodes;
  nodes = 0;

  graph_wdgt->clear();
}

/*--------------------------------------------------------------------
 * cg_viewer::init
 */
void
cg_viewer::init()
{
  // return if no file_set_block exists
  if ( !(suif_env->get_file_set_block()) ) return;

  /* === initialize call graph === */
  init_sbrowser_cg( 0, 0 );
  current_cg = new cg( true );

  post_progress( this, init_msg, 50 );

  show_cg( current_cg );

  exit_sbrowser_cg();

  unpost_progress(this);
}

/*--------------------------------------------------------------------
 * cg_viewer::show_cg
 */
void
cg_viewer::show_cg(cg* callgraph)
{
  int i;

  num_nodes = callgraph->num_nodes();
  nodes = new gnode*[num_nodes];

  /* add nodes */
  for (i = 0; i < num_nodes; i++ ) {
    cg_node* node = callgraph->get_node(i);
    ProcedureSymbol* psym = node->get_procedure_symbol();
    ProcedureDefinition* pdef = psym->get_definition();

    if ( show_external_procs || pdef ) {
      vnode* vn = create_vnode( psym );
      gnode* graph_node = graph_wdgt->add_node((char *)psym->get_name().c_str(), vn );
      nodes[i] = graph_node;

      if ( node == callgraph->main_node() ) {
	      graph_wdgt->set_root_node( graph_node );
      }
    } else {
      // don't show this node
      nodes[i] = 0;
    }
  }

  /* add edges */
  for (i = 0; i < num_nodes; i++ ) {
    cg_node* caller;
    // cg_node* callee;

    caller = callgraph->get_node( i );
    cg_node_list* callees = caller->succs();
    for ( int j = 0; j < (int)callees->size(); j++ ) {
      cg_node* callee = (*callees)[j];
      int caller_num = caller->number();
      int callee_num = callee->number();

      arrow_dir arrow;
      arrow = ARROW_FORWARD;

      if ( nodes[caller_num] && nodes[callee_num] ) {
	  graph_wdgt->add_edge( nodes[caller_num], nodes[callee_num], arrow );
      }
    }
    visual_yield();
  }
  graph_wdgt->layout();
}

/*--------------------------------------------------------------------
 * cg_viewer::create_proc_menu
 */
void cg_viewer::create_proc_menu() {
  menu->clear( "Procedure" );

  add_std_proc_menu( menu, "Procedure" );
  menu->add_separator( "Procedure" );
  add_close_command( menu, "Procedure" );
}

/*--------------------------------------------------------------------
 * cg_viewer::create_view_menu
 */
void cg_viewer::create_view_menu() {
  menu->clear("View");

  binding* b = new binding((bfun) &do_show_external_procs, this );
  menu->add_check( b, "View/Options", "Show external procedures", true );
}

/*--------------------------------------------------------------------
 */
void cg_viewer::set_binding( binding* b ) {
  graph_wdgt->set_binding( b );
}

/*--------------------------------------------------------------------
 */
void cg_viewer::select(ProcedureDefinition* proc ) {
  vnode* vn = vman->find_vnode( proc );
  graph_wdgt->select(vn);
}

/*--------------------------------------------------------------------
 */
void cg_viewer::do_show_external_procs(event&, cg_viewer* viewer ) {
  viewer->show_external_procs = !viewer->show_external_procs;

  if (viewer->current_cg) {
    viewer->clear();
    viewer->show_cg(viewer->current_cg);
  }
}

/*--------------------------------------------------------------------
 * event handler
 *
 * when a event occurs, this function is invoked
 */
void cg_viewer::handle_event(event& e) {
  inherited::handle_event(e);

  switch (e.kind()) {
  case SELECTION:
    {
      void* event_source = e.get_source();
      if ( event_source == graph_wdgt ) return; // ignore local event
      if ( !current_cg ) init();
      show( e.get_object() );
    }
    break;

  case CLOSE_FILESET:
    {
      clear();
    }
    break;

  case NEW_FILESET:
    {
      create_proc_menu();
      create_view_menu();
      init();
    }
    break;

  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * cg_viewer::show
 *
 */
void
cg_viewer::show(vnode* vn)
{
  if ( !vn ) return;

  ProcedureDefinition* proc = 0;
  char* tag = vn->get_tag();

  if ( tag == tag_suif_object ) {
    SuifObject* obj = (SuifObject*) vn->get_object();
  
    if (is_kind_of<ProcedureSymbol>(obj)) {
      proc = to<ProcedureSymbol>(obj)->get_definition();
    } else if (is_kind_of<ProcedureDefinition>(obj)) {
      proc = to<ProcedureDefinition>(obj);
    }

    if ( proc ) select( proc );

  } else if (tag == tag_code_fragment) {
    code_fragment* f = (code_fragment *) vn->get_object();
    if (f->node() && is_kind_of<ProcedureDefinition>(f->node())) {
      select(to<ProcedureDefinition>(f->node()));
    }
  }
}
