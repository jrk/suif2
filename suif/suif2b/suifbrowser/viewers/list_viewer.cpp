/*-------------------------------------------------------------------
 * list_viewer
 *
 */

#include "suifnodes/suif.h"
#include "list_viewer.h"
#include "suif_vnode.h"
#include "suif_event.h"
#include "suif_utils.h"


/*--------------------------------------------------------------------
 * proc_list::proc_list
 *
 */
proc_list::proc_list( char* text ) : 
  list_base_viewer() { 
  set_title( text ); 
}

/*--------------------------------------------------------------------
 * proc_list::create_window
 *
 */
void proc_list::create_window() {
  inherited::create_window();

  add_close_button( button_bar );
  init();
}

/*--------------------------------------------------------------------
 * proc_list::class_name
 *
 */
char *proc_list::class_name() { 
  return "Procedure List"; 
}

/*--------------------------------------------------------------------
 * proc_list::init
 *
 */
void proc_list::init() {
  //slist_tos<procedure_symbol*>* procs = get_procedure_list
  //( get_default_file_set() );
  list<ProcedureSymbol*>* procs =
                  get_procedure_list(suif_env->get_file_set_block() );
  
  /* add them to the listbox */
  for ( s_count_t nr=0; nr<procs->size(); nr++ ) {
    ProcedureSymbol* current = (*procs)[nr];
    vnode* vn = create_vnode( current );
    listbox->add( 0, vn, (char*)current->get_name().c_str() );
  }
}

/*--------------------------------------------------------------------
 * proc_list::handle_event
 *
 */
void proc_list::handle_event( event& e ) {
  inherited::handle_event( e );

  switch ( e.kind() ) {
  case NEW_FILESET:
    {
      listbox->clear();
      init();
    }
    break;
  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * proc_list::constructor
 *
 */
window* proc_list::constructor() {
  return ( new proc_list( "Procedures:" ) );
}








