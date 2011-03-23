/*----------------------------------------------------------------------
 * ann_form
 */


#include "ann_form.h"
#include "base_viewer.h"
#include "suif_event.h"
#include "suif_utils.h"
#include "suif_vnode.h"
#include "suif_types.h"
#include "suif_print.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic_factory.h"

extern SuifEnv*suif_env;

/*----------------------------------------------------------------------
 * ann_form::ann_form
 */
ann_form::ann_form( AnnotableObject *par, Annote *obj )
{
  new_annote = (obj == 0);
  if ( new_annote ) obj = create_annote(suif_env);
  current_obj = obj;
  parent = par;
}

/*----------------------------------------------------------------------
 * ann_form::~ann_form
 */
ann_form::~ann_form(void)
{
  //not part of the tree => delete
  if ( !current_obj->get_parent() )  delete current_obj; 
}

/*----------------------------------------------------------------------
 * ann_form::destroy
 */
void ann_form::destroy(void)
{
  inherited::destroy();
}

/*----------------------------------------------------------------------
 * ann_form::create_window
 */
void ann_form::create_window(void)
{

  inherited::create_window();

  /* set title */

  char string[200];
  sprintf(string, "Annote: (0x%p) Object: (0x%p) %s",
	  current_obj, current_obj,
	  current_obj->get_meta_class()->get_class_name().c_str());
  set_info_bar(string);

  form->add_field("Annote name", type_BRICK_STRING,
             (char *)current_obj->get_meta_class()->get_class_name().c_str() );

  add_bricks( current_obj );

  /* menu */
  binding *b;

  static char *immed_types[] = {
    "BRICK_NULL", 
    "BRICK_OWNED_ZOT", 
    "BRICK_ZOT_REF", 
    "BRICK_INTEGER", 
    "BRICK_STRING",
    "BRICK_BIT_BLOCK",
     0
  };

  for (char **t = immed_types; *t; t++) {
    b = new binding((bfun2) &do_insert, this,(char*) LString(*t).c_str() );
    menu->add_command(b, "Edit/Insert Field", *t);
  }

  b = new binding((bfun) &do_delete, this);
  menu->add_command(b, "Edit", "Delete field");

  /* buttons */
  b = new binding((bfun) &do_update, this);
  button_bar->add_button(b, "Ok");

  b = new binding((bfun) &do_cancel, this);
  button_bar->add_button(b, "Cancel");
}

/*----------------------------------------------------------------------
 */
void ann_form::do_update(event &, ann_form *win)
{

  char *annote_name = win->form->get_field_data(0);
  (void) annote_name;
#ifdef AG
  annote_def *adef = lookup_annote(annote_name);
  if (!adef) {
    char buffer[200];
    sprintf(buffer, "The annote name `%s' is not registered. "
	    "Do you want to register the annotation and continue?",
	    annote_name);
    int result = display_dialog(win, buffer, "Yes No", 0);
    if (result != 0) {
      return;
    }

    adef = new annote_def(annote_name, true);
    register_annote(adef);
  }
#endif

  BrickList *bricklist = win->get_brick_list( win->current_obj, 0);
  if (!bricklist) return;

//AG  win->current_obj->set_name( annote_name );

#if TO_BE_SETTLED
  for ( s_count_t _cnt=0; _cnt<bricklist->size(); _cnt++ ) {
    win->current_obj->set_component( _cnt+1, (*bricklist)[_cnt] );
  }
  if (win->new_annote) {
    /* add annote to the object's annote list */
    win->parent->append_annote(win->current_obj);
    win->new_annote = false;
  }
#endif // TO_BE_SETTLED

  /* post event */
  ProcedureDefinition *proc = get_procedure_definition( win->current_obj );
  if ( proc ) {
    post_event( event( create_vnode( proc ), PROC_MODIFIED ) );
  } else {   
    FileBlock *fb = get_file_block( win->current_obj );
    if ( fb ) {
	    post_event(event( create_vnode( fb ), FSE_MODIFIED));
    } else {
	    assert(false);		// this should not happen..
    }
  }
  win->destroy();
}

/*----------------------------------------------------------------------
 */
void ann_form::do_cancel(event &, ann_form *win)
{
  win->destroy();
}

/*----------------------------------------------------------------------
 * Insert a field
 */
void ann_form::do_insert(event &, ann_form *win, char *type)
{
  int f = win->form->get_current_field();

  int n = win->form->num_fields();
  char **field_data = new char*[n];
  char **field_type = new char*[n];
  int i;
  for (i = 0; i < n; i++) {
    field_data[i] = strdup(win->form->get_field_data(i));
    field_type[i] = strdup(win->form->get_field_type(i));
  }

  win->form->clear();
  win->form->add_field("Annote name", type_BRICK_STRING, field_data[0]); 

  int j = 1;
  for (i = 1; i < n+1; i++) {
    char field_name[50];
    sprintf(field_name, "Field %d", i);

    if (i == f+1) {
      win->form->add_field(field_name, type, "");
    } else {
      win->form->add_field(field_name, field_type[j], field_data[j]);
      j++;
    }
  }

  win->form->focus_field(f+1);

  for (i = 0; i < n; i++) {
    delete field_data[i];
    delete field_type[i];
  }
  delete field_data;
  delete field_type;
}

/*----------------------------------------------------------------------
 * Delete a field
 */
void ann_form::do_delete(event &, ann_form *win)
{
  int f = win->form->get_current_field();

  if (f == 0) {
    display_message(win, "Cannot delete annote name.");
    return;
  }

  int n = win->form->num_fields();
  char **field_data = new char*[n];
  char **field_type = new char*[n];
  int i;
  for (i = 0; i < n; i++) {
    field_data[i] = strdup(win->form->get_field_data(i));
    field_type[i] = strdup(win->form->get_field_type(i));
  }

  win->form->clear();
  win->form->add_field("Annote name", type_BRICK_STRING, field_data[0]); 

  int j = 1;
  for (i = 1; i < n; i++) {
    char field_name[50];
    sprintf(field_name, "Field %d", j);

    if (i != f) {
      win->form->add_field(field_name, field_type[i], field_data[i]);
      j++;
    }
  }

  win->form->focus_field(f);

  for (i = 0; i < n; i++) {
    delete field_data[i];
    delete field_type[i];
  }
  delete field_data;
  delete field_type;
}


/*----------------------------------------------------------------------
 * handle_event
 */
void ann_form::handle_event(event &e)
{
  inherited::handle_event(e);

  switch (e.kind()) {
  case CLOSE_FILESET:
    {
      destroy();		// destroy this form
    }
    break;
   case VNODE_DESTROY:
    {
      SuifObject *z = (SuifObject*)e.get_object();
      if ( ( z == parent ) || ( z == current_obj ) ) {
        destroy();
      }
    }
    break;
  default:
    break;
  }

}

