/*-------------------------------------------------------------------
 * suif_viewer
 *
 */

#include "suif_viewer.h"
#include "suif_vnode.h"
#include "suif_event.h"
#include "suif_menu.h"
#include "suif_print.h"
#include "suif_utils.h"
#include "suifnodes/suif.h"
#include <stdlib.h>

/*---------------------------------------------------------------*/
bool tag_prefix_match(LString tag, char *prefix)
  {
    const char *tag_chars = tag.c_str();
    if ((tag_chars == 0) && (prefix == 0))
        return true;
    if ((tag_chars == 0) || (prefix == 0))
        return false;
    const char *prefix_chars = prefix;
    while (*prefix_chars != 0)
      {
        if (*tag_chars != *prefix_chars)
            return false;
        ++tag_chars;
        ++prefix_chars;
      }
    return ((*tag_chars == '/') || (*tag_chars == 0));
  }

/*-----------------------------------------------------------------
 * is_member function that finds out if an element is present in a
 * list.
 */
template <class T1, class T2>
bool is_member(T1 *l, T2 key)
{
  typename T1::iterator current = l->begin();
  typename T1::iterator last = l->end();
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
/*-----------------------------------------------------------------*/
class collect_annotations_visitor : public stopping_suif_visitor {
  string_list* list;
public:
  collect_annotations_visitor(SuifEnv *suif):stopping_suif_visitor(suif) { list = new string_list; }

  enum Walker::ApplyStatus operator() (void *zot, MetaClass* type) {
     if (type->get_class_name() == Annote::get_class_name())
        handle_annote((Annote*) zot);
     return Continue;
  }
  virtual void handle_annote( Annote* the_annote ) {  
    if ( !is_member( list, the_annote->get_name())  ) {
      String name = String( "Annote: [" ) + the_annote->get_class_name() + String( "]" );
      list->push_back( name );
      list->push_back( the_annote->get_name() );
    }
  }

  virtual string_list *get_string_list() { string_list *l=list;list=0;return l;}

  virtual ~collect_annotations_visitor() { delete list; }
};

class find_instruction_visitor : public stopping_suif_visitor {
  s_count_t number;
  SuifObject* result;
public:
  find_instruction_visitor(SuifEnv *suif, s_count_t num ) :
  stopping_suif_visitor(suif), number( num ), result( 0 ) {}

  virtual void handle_zot(SuifObject* the_zot )  {
#if TO_BE_SETTLED
    if ( the_zot->id_number() == number ) {
      result = the_zot;
      stop = true;
    }
#endif // TO_BE_SETTLED
  }
  enum Walker::ApplyStatus operator() (void *zot, MetaClass* type) {
     handle_zot((SuifObject*) zot);
     return Continue;
  }

  SuifObject *get_zot() { return result; }
};

class find_tag_visitor : public stopping_suif_visitor {
  char *tag;
  SuifObject *result;
public:
  find_tag_visitor(SuifEnv *suif, char *t ) : stopping_suif_visitor(suif), tag( t ), result( 0 ) {}

  virtual void handle_zot( Annote* the_zot )  {
    if  ( tag_prefix_match( the_zot->get_name(), tag ) ) {
      result = the_zot;
      stop = true;
    }
  }
  enum Walker::ApplyStatus operator() (void *zot, MetaClass* type) {
     handle_zot((Annote*) zot);
     return Continue;
  }

  SuifObject *get_zot() { return result; }
};


/*--------------------------------------------------------------------
 * suif_viewer::suif_viewer
 */
suif_viewer::suif_viewer() {
  annotation_names=0;
  current_proc = 0;
  show_mark_instructions = false;
}


/*--------------------------------------------------------------------
 * suif_viewer::~suif_viewer
 *
 */
suif_viewer::~suif_viewer() {
  delete info_bar;
  delete annotation_names;
}


/*--------------------------------------------------------------------
 * suif_viewer::create_window
 */

void suif_viewer::create_window() {
  inherited::create_window();
  info_bar = new vmessage( toplevel );

  refresh();

  select_node( vman->get_selection() );
}

/*--------------------------------------------------------------------
 * suif_viewer::clear
 *
 */
void suif_viewer::clear() {
  current_proc = 0;

  refresh();
}

/*--------------------------------------------------------------------
 * suif_viewer::refresh
 */
void suif_viewer::refresh_menu() {
  menu->remove(ROOT_MENU);
  create_proc_menu();
  //@@@ editing is currently not supported create_edit_menu();
  create_view_menu();
  create_find_menu();
  add_std_go_menu(menu);

  update_info_bar();
}


/*--------------------------------------------------------------------
 * suif_viewer::refresh
 */
void suif_viewer::refresh() {
  text->clear();

  refresh_menu();

  ProcedureDefinition* proc = current_proc;
  current_proc = 0;	// to force a redraw
  view( proc );

  select_node( vman->get_selection() );
}


/*--------------------------------------------------------------------
 * suif_viewer::show_node
 *
 * to make sure a node is expanded, visible
 */
void suif_viewer::show_node( SuifObject* node ) {
  if ( !is_kind_of<ProcedureDefinition>(node) ) { 
    /*  make sure its parent is expanded */
    node = node->get_parent();
    if ( node ) {
      show_node( node );
      vnode* vn = create_vnode( node );
      tag_node* par_tag = text->find_tag(vn);
      if (par_tag && ( !par_tag->is_expanded() ) ) {
	text->expand_node(par_tag);
      }
    }
  }
}

/*--------------------------------------------------------------------
 * suif_viewer::view
 *
 * view a procedure
 */
void
suif_viewer::view(SuifObject* node)
{
  if ( !node ) return;

  ProcedureDefinition* proc = get_procedure_definition( node );
						  
  if ( !proc ) return; // if "node" is not contained in a procedure we just
	               //   display the current contents
  if ( current_proc != proc ) {
    current_proc = proc;
    text->clear();

    /* print procedure */
    formater f(suif_env, text );
    f.set_current_filter(show_mark_instructions ? f.null_filter :f.mark_filter);
    f.print_zot(proc, text->fout());
    text->update();
    
    /* update menu and info bar */
    update_info_bar();
  }
  if ( !is_same_procedure( proc, node )  ) { 
    // view a node in the procedure and not the procedure
    show_node( node );
    text->view( create_vnode( node ) );
  }
}


/*--------------------------------------------------------------------
 * suif_viewer::view
 *
 * view a code fragment
 */
void suif_viewer::view( code_fragment* f ) {
  assert( f );
  view( f->node() );
}


/*--------------------------------------------------------------------
 * suif_viewer::select
 *
 * select a code fragment
 */
void suif_viewer::select( code_fragment* f ) {
  assert( f );
  text->select_clear();
  f->select( text, true );
  //  text->select( create_vnode( f->node() ), true );
}


void suif_viewer::select( SuifObject* node ) {
  text->select_clear();
  text->select( create_vnode( node ) );
}

/*---------------------------------------------------------------------
 * show
 */
void
suif_viewer::select_node(vnode* vn)
{
  if ( !vn ) return;
  char *tag = vn->get_tag();

  if (tag == tag_suif_object) {
    SuifObject* node = (SuifObject*) vn->get_object();
    view( node );
    select( node );    
  } else if ( tag == tag_code_fragment ) {
    code_fragment* f = (code_fragment*) vn->get_object();
    view( f );
    select( f );
  }
}

/*--------------------------------------------------------------------
 * handle_event
 *
 * when a event occurs, this function is invoked
 */
void
suif_viewer::handle_event(event& e)
{
  inherited::handle_event(e);

  switch (e.kind()) {
  case SELECTION:
    {
      void *event_source = e.get_source();
      vnode* vn = e.get_object();
      SuifObject* z = suif_utils::get_zot( vn );
      if ((event_source == text) &&
          !(z && is_kind_of<ProcedureSymbol>(z)))
              return;	// ignore local event which is not a procedure_symbol
      select_node( e.get_object() );
      refresh_menu();
    }
    break;

  case CLOSE_FILESET:
  case NEW_FILESET:
    {
      clear();
    }
    break;

  case REFRESH:
  case PROC_MODIFIED:
  case FSE_MODIFIED:
    {
      refresh();
    }
    break;

  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * suif_viewer::update_info_bar
 *
 */
void
suif_viewer::update_info_bar()
{
  String proc_name = current_proc ? ( String("Procedure: ")+
			current_proc->get_procedure_symbol()->get_name())	
                        : String("<No procedure selected>");
  info_bar->set_message((char *) proc_name.c_str());
}

/*--------------------------------------------------------------------
 * suif_viewer::create_proc_menu
 *
 */
void suif_viewer::create_proc_menu() {
  menu->clear("Procedure");
  add_std_proc_menu(menu, "Procedure");
  menu->add_separator("Procedure");
  add_close_command(menu, "Procedure");
}

/*--------------------------------------------------------------------
 * suif_viewer::create_edit_menu
 *
 */
void suif_viewer::create_edit_menu() {
  menu->clear("Edit");
  add_std_edit_menu(menu, "Edit");
}

/*--------------------------------------------------------------------
 * suif_viewer::create_view_menu
 *
 */
void suif_viewer::create_view_menu() {
  menu->clear("View");

  binding *b = new binding((bfun) &expand_all_cmd, this);
  menu->add_command(b, "View", "Expand all");

  b = new binding((bfun) &collapse_all_cmd, this);
  menu->add_command(b, "View", "Collapse all");

  menu->add_separator("View");
  b = new binding((bfun) &filter_mrk_cmd, this);
  menu->add_check(b, "View/Filter", "mrk", !show_mark_instructions );
}


/*--------------------------------------------------------------------
 * suif_viewer::filter_mrk_cmd
 *
 */
void suif_viewer::filter_mrk_cmd( event&, suif_viewer* viewer ) {
  viewer->show_mark_instructions = !viewer->show_mark_instructions;
  viewer->refresh();
}

/*--------------------------------------------------------------------
 * suif_viewer::create_find_menu
 *
 */
void
suif_viewer::create_find_menu()
{
  menu->clear("Find");

  if ( !current_proc ) return;

#define STATEMENT_TAG_CHARS "/stmt"
#define SCOPE_STATEMENT_TAG_CHARS "/scope"
#define WHILE_STATEMENT_TAG_CHARS "/while"
#define FOR_STATEMENT_TAG_CHARS "/for"
#define IF_STATEMENT_TAG_CHARS "/if"
  /* find node */
  find_info.tag = STATEMENT_TAG_CHARS;

  binding *b;
  
  b = new binding((bfun2) &find_node_cmd, this, (char*)STATEMENT_TAG_CHARS );
  menu->add_radio(b, "Find/Item", "Statement node");

  b = new binding((bfun2) &find_node_cmd, this, (char*)SCOPE_STATEMENT_TAG_CHARS );
  menu->add_radio(b, "Find/Item", "Scope node");

  b = new binding((bfun2) &find_node_cmd, this, (char*) WHILE_STATEMENT_TAG_CHARS );
  menu->add_radio(b, "Find/Item", "While node");

  b = new binding((bfun2) &find_node_cmd, this, (char*)FOR_STATEMENT_TAG_CHARS );
  menu->add_radio(b, "Find/Item", "For node");

  b = new binding((bfun2) &find_node_cmd, this, (char*)IF_STATEMENT_TAG_CHARS );
  menu->add_radio(b, "Find/Item", "If node");
  
  /* annotation items */
  collect_annotations_visitor ann_vis(suif_env);
  iterate_over( current_proc, &ann_vis);

  delete annotation_names;
  annotation_names = ann_vis.get_string_list();
  for ( s_count_t _cnt=0; _cnt<annotation_names->size(); _cnt++ ) {
    String name = (*annotation_names)[_cnt];
    String tag  = (*annotation_names)[++_cnt];
    b = new binding((bfun2) &find_node_cmd, this, (char *)tag.c_str() );
    menu->add_radio(b, "Find/Item", (char *)name.c_str());
    
  }
  menu->add_separator("Find");

  /* "find prev" command */
  b = new binding((bfun2) &find_cmd, this, (void *) false);
  menu->add_command(b, "Find", "Previous", "p");
  
  /* "find next" command */
  b = new binding((bfun2) &find_cmd, this, (void *) true);
  menu->add_command(b, "Find", "Next", "n");

  menu->add_separator("Find");

  /* find instruction command */
  b = new binding((bfun) & find_instr_cmd, this);
  menu->add_command(b, "Find", "Instruction..");
}

/*--------------------------------------------------------------------
 * suif_viewer::expand_all
 *
 */
void suif_viewer::expand_all_cmd( event&, suif_viewer* viewer ) {
  viewer->text->expand_all();
}

/*--------------------------------------------------------------------
 * suif_viewer::collapse_all
 *
 */
void suif_viewer::collapse_all_cmd(event&, suif_viewer* viewer ) {
  viewer->text->collapse_all();
}

/*--------------------------------------------------------------------
 * find instruction
 *
 */
void suif_viewer::find_instr_cmd( event&, suif_viewer* viewer ) {
  char buffer[200];
  display_query(viewer, "Enter instruction number:", buffer);

  int inum;
  if (sscanf(buffer, "%d", &inum) == 1 &&
      viewer->current_proc) {
    SuifObject *target;
    /* find the instruction */
    find_instruction_visitor  vis(suif_env, inum );
    iterate_over( viewer->current_proc, &vis );
    target = vis.get_zot();
    if ( target  ) {
      vnode *vn = create_vnode( target );
      viewer->view( target );
      viewer->select( target );
      post_event(event(vn, SELECTION, viewer->text));
    } else {
      display_message(viewer, "Cannot find instruction numbered %d.", inum);
    }
  }
}




/*--------------------------------------------------------------------
 * find node
 *
 */
void suif_viewer::find_node_cmd( event&, suif_viewer* viewer, char* tag ) {
  viewer->find_info.tag= (char*)tag;
  find_cmd(event(), viewer ,(void *) true);
}


/*--------------------------------------------------------------------
 * find next/prev
 *
 */
void suif_viewer::find_cmd(const event&, suif_viewer* viewer,
				void* search_forward) {
 assert( viewer );
 viewer->find_helper( viewer->find_info.tag, (bool) search_forward );
}

/*--------------------------------------------------------------------
 * find next helper
 *
 */
void suif_viewer::find_helper( void* client_data, bool search_forward ) {
  bool found;
  SuifObject* node = 0;
  vnode *vn = text->get_selection();

  text->select_clear();

  if (vn && vn->get_tag() == tag_suif_object) {
    SuifObject *obj = (SuifObject *) vn->get_object(); 
    find_tag_visitor vis(suif_env, (char*)client_data );
    if (search_forward) {
      found = iterate_next( obj, &vis, current_proc );
      if (!found) // wrap around
        found = iterate_next( current_proc, &vis, current_proc );
    } else {
      found = iterate_previous( obj, &vis, current_proc );
      if (!found) // wrap around
        found = iterate_previous( last_zot(current_proc), &vis, current_proc );
    }
    node = vis.get_zot();
  }

  if ( node ) {
      view( node );
      select( node );
      post_event(event( create_vnode( node ), SELECTION, text ) );
    }
}
