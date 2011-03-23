/*-------------------------------------------------------------------
 * prof_viewer
 *
 */

#include "prof_viewer.h"
#include "suif_event.h"
#include <stdlib.h>

/*--------------------------------------------------------------------
 * prof_viewer::prof_viewer
 */
prof_viewer::prof_viewer() {
  //@@@  p_table = NULL;
}

prof_viewer::~prof_viewer() {
  //@@@  delete p_table;
}

/*--------------------------------------------------------------------
 * prof_viewer::create_window
 *
 */
void prof_viewer::create_window() {
  inherited::create_window();

  /* menu */
  binding* b = new binding( (bfun) &do_open_runtime_file, this );
  menu->add_command( b, "File", "Open Runtime Stats.." );

  add_close_command( menu, "File" );

  init();

  show( vman->get_selection() );
}

/*--------------------------------------------------------------------
 * prof_viewer::clear
 *
 */
void prof_viewer::clear() {
  //@@@  delete p_table;
  //@@@  p_table = NULL;
  
  text->clear();
}

/*--------------------------------------------------------------------
 * prof_viewer::init
 *
 */
void prof_viewer::init() {
  //@@@  delete p_table;
  
  post_progress( this, "Scanning for profile information", 0 );
  //@@@ p_table = new profile_table;
  //@@@ p_table->load_memprof_data();
  post_progress( this, "Scanning for profile information", 50 );

  print_profile();
  unpost_progress( this );
}

/*--------------------------------------------------------------------
 * prof_viewer::print_profile
 *
 */
void prof_viewer::print_profile() {
  text->clear();
  fstream& fout = text->fout();

  /*
   * loop nodes
   */

  text->tag_style( BOLD_BEGIN );
  fout << "LOOP/FOR nodes\n";
  text->tag_style( BOLD_END );
  fout << "    Proc      Type   Visits  Fallthroughs  Ave trip count\n"
	   "--------------------------------------------------------------\n";

  /* print out the profile information */
#ifdef CURRENTLY_NOT_IMPLEMENTED //@@@
  int count = 0;
  profile_data_list_iter iter(p_table->loop_profs);
  while (!iter.is_empty()) {
    loop_branch_profile *p = (loop_branch_profile *) iter.step();
    tree_node *tn = p->tn;
      
    char *desc;
    switch (tn->kind()) {
    case TREE_FOR:
      desc = "for";
      break;
    case TREE_LOOP:
      desc = "loop";
      break;
    default:
      desc = "???";
      break;
    }

    vnode *vn = create_vnode(tn);
    text->tag_begin(vn);
    count++;
    fprintf(fd, "%2d: %-10s %-8s %-8d %-10d %.3f\n",
	    count,
	    tn->proc()->name(),
	    desc,
	    p->visit_count, p->fallthrough_count, p->avg_trip_count);
    text->tag_end(vn);
  }
  if (count == 0) {
    fprintf(fd, "No profile information found\n");
  }
  
  /*
   * if nodes
   */
  
  text->tag_style(BOLD_BEGIN);
  fout << "\n\nIF nodes\n";
  text->tag_style(BOLD_END);
  fout << "    Proc      Visits    THEN count   Then ratio\n"
	  "--------------------------------------------------------------\n";
  
  /* print out the profile information */
  count = 0;
  iter.reset(p_table->if_profs);
  while (!iter.is_empty()) {
    if_branch_profile *p = (if_branch_profile *) iter.step();
    tree_node *tn = p->tn;
    
    vnode *vn = create_vnode(tn);
    text->tag_begin(vn);
    
    count++;
    fprintf(fd, "%2d: %-10s %-10d %-10d %.3f\n",
	    count,
	    tn->proc()->name(),
	    p->visit_count, p->then_count, p->then_ratio);
    text->tag_end(vn);
  }
  if (count == 0) {
    fout << "No profile information found\n";
  }

  /*
   * runtime profile
   */

  text->tag_style(BOLD_BEGIN);
  fout << "\n\nRuntime Stats\n";
  text->tag_style(BOLD_END);
  if (p_table->runtime_profile_string) {
    fout << p_table->runtime_profile_string << endl;
  }

  fout << "    Proc               Time  Doalls  Barriers Sync-Ngbs Locks Reductions\n"
	  "--------------------------------------------------------------\n";
  count = 0;
  profile_data_list_iter p_iter(p_table->runtime_profs);
  while (!p_iter.is_empty()) {
    proc_runtime_profile *p = (proc_runtime_profile *) p_iter.step();
    count++;

    vnode *vn;
    if (p->proc) {
      vn = create_vnode(p->proc);
      text->tag_begin(vn);
    }

    fprintf(fd, "%2d: %-18s %3.3f %6d %6d %6d %6d %6d\n",
	    count,
	    p->proc_name,
	    p->time,
	    p->doalls,
	    p->barriers,
	    p->sync_neighbors,
	    p->locks,
	    p->reductions);

    if (p->proc) {
      text->tag_end(vn);
    }
  }

  if ( count == 0 ) {
    fout << "Runtime data file not loaded.\n";
  }
#endif
  text->update();
}

/*--------------------------------------------------------------------
 * prof_viewer::do_open_runtime_file
 *
 */
void prof_viewer::do_open_runtime_file( event &, prof_viewer* viewer ) {
  char filename[1000];
  select_file(viewer, filename, "Load profile data:");
  if ( filename[0] ) {
#ifdef CURRENTLY_NOT_IMPLEMENTED //@@@
    if ( !viewer->p_table->load_runtime_data( filename ) ) {

      display_message( viewer, "Error loading runtime data: %s",
		       viewer->p_table->last_error ?
		       viewer->p_table->last_error : "(??)" );
      return;
    }
#endif
    viewer->print_profile();
  }
}

/*--------------------------------------------------------------------
 * prof_viewer::handle_event
 *
 */
void prof_viewer::handle_event( event& e ) {
  inherited::handle_event(e);

  switch (e.kind()) {
  case SELECTION:
    {
      vnode *vn = e.get_object();
      show(vn);
    }
    break;

  case CLOSE_FILESET:
    {
      clear();
    }
    break;

  case NEW_FILESET:
    {
      init();
    }
    break;

  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * prof_viewer::show
 *
 */
void prof_viewer::show( vnode* vn ) {
  if (!vn) return;

  /* call text widget to show it (if it exists) */
  text->view( vn );
  text->select( vn );
}
