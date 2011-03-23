/*-------------------------------------------------------------------
 * base_viewer.cc
 *
 */

#include "base_viewer.h"
#include "suif_types.h"
#include "suif_event.h"
#include "basicnodes/basic_factory.h"
#include <stdlib.h>

/*--------------------------------------------------------------------
 */
static void event_helper( event& e, base_viewer* viewer ) {
  viewer->handle_event( e );
}

/*--------------------------------------------------------------------
 * base_viewer::base_viewer
 */
base_viewer::base_viewer() {
  // @@@ should this be moved to create_window ??
  event_binding = new binding( (bfun) &event_helper, this );
  add_event_binding( event_binding, VISUAL_EVENTS | X_EVENTS | SUIF_EVENTS );
}

/*--------------------------------------------------------------------
 * base_viewer::~base_viewer
 *
 */
base_viewer::~base_viewer() {
  // @@@ should this be moved to destroyed ??
  remove_event_binding( event_binding );
  delete event_binding;
}

/*--------------------------------------------------------------------
 * base_viewer::add_close_command
 *
 */
void base_viewer::add_close_command( vmenu* menu, char* parent_menu ) {
  binding* b = new binding( (bfun) &do_close_command, this );
  menu->add_command( b, parent_menu, "Close" );
}

/*--------------------------------------------------------------------
 * base_viewer::add_close_button
 *
 */
void base_viewer::add_close_button( vbuttonbar* button_bar ) {
  binding* b = new binding( (bfun) &do_close_command, this );
  button_bar->add_button( b, "Close" );
}

/*--------------------------------------------------------------------
 * base_viewer::do_close_command
 *
 */
void base_viewer::do_close_command( event& , base_viewer* viewer ) {
  viewer->destroy();
}

/*--------------------------------------------------------------------
 * text_base_viewer::text_base_viewer
 */
text_base_viewer::text_base_viewer() {
}

/*--------------------------------------------------------------------
 * text_base_viewer::~text_base_viewer
 *
 */
text_base_viewer::~text_base_viewer() {
  delete menu;
  delete frame;
  delete text;
}

/*--------------------------------------------------------------------
 * text_base_viewer::create_window
 */
void text_base_viewer::create_window() {
  inherited::create_window();

  menu  = new vmenu(toplevel);
  frame = new vframe(toplevel);
  text  = new vtext(frame);
}

/*--------------------------------------------------------------------
 * text_base_viewer::handle_event
 */
void text_base_viewer::handle_event( event& e ) {
  inherited::handle_event(e);

  switch (e.kind()) {
  case PROP_CHANGE:
    {
//@@@ this is not yet ported
//@@@      vprop *p = (vprop *) e.get_param();
//@@@      text->update_prop(p);
    }
    break;

  default:
    break;
  }
}

/*--------------------------------------------------------------------
 * graph_base_viewer::graph_base_viewer
 */
graph_base_viewer::graph_base_viewer() {
}

/*--------------------------------------------------------------------
 * graph_base_viewer::~graph_base_viewer
 *
 */
graph_base_viewer::~graph_base_viewer() {
  delete menu;
  delete frame;
  delete graph_wdgt;
}

/*--------------------------------------------------------------------
 * graph_base_viewer::create_window
 */
void graph_base_viewer::create_window() {
  inherited::create_window();

  menu       = new vmenu(toplevel);
  frame      = new vframe(toplevel);
  graph_wdgt = new vgraph(frame);
}

/*--------------------------------------------------------------------
 * list_base_viewer::list_base_viewer
 */
list_base_viewer::list_base_viewer() {
  list_title = 0;
}

/*--------------------------------------------------------------------
 * list_base_viewer::~list_base_viewer
 *
 */
list_base_viewer::~list_base_viewer() {
  delete listbox;
}

/*--------------------------------------------------------------------
 * list_base_viewer::set_title
 */
void list_base_viewer::set_title(char *title) {
  list_title = title;
}

/*--------------------------------------------------------------------
 * list_base_viewer::create_window
 */
void list_base_viewer::create_window() {
  inherited::create_window();

  listbox_frame = new vframe(toplevel);
  button_frame  = new vframe(toplevel, false );
  listbox       = new vlistbox(listbox_frame, list_title, true );
  button_bar    = new vbuttonbar(button_frame);
}

/*--------------------------------------------------------------------
 * form_base_viewer::form_base_viewer
 */
form_base_viewer::form_base_viewer() {
}

/*--------------------------------------------------------------------
 * form_base_viewer::~form_base_viewer
 *
 */
form_base_viewer::~form_base_viewer() {
  delete form;
}

/*--------------------------------------------------------------------
 * form_base_viewer::create_window
 */
void form_base_viewer::create_window() {
  inherited::create_window();

  menu         = new vmenu(toplevel);
  form_frame   = new vframe(toplevel);
  button_frame = new vframe(toplevel);
  form         = new vform(form_frame);
  info_bar     = new vmessage(toplevel);
  button_bar   = new vbuttonbar(button_frame);
}

/*--------------------------------------------------------------------
 * form_base_viewer::set_info_bar
 */
void form_base_viewer::set_info_bar( char* msg ) {
  info_bar->set_message(msg);
}

/*----------------------------------------------------------------------
 * form_base_viewer::add_bricks
 */
void form_base_viewer::add_bricks( SuifObject* obj ) {
#if TO_BE_SETTLED
    char field_string[20];

  //for ( s_count_t i=0; i<obj->num_components(); i++ )
  {
    //brick br = obj->component( i );
    SuifBrick br = obj->component( i );

    sprintf( field_string, "Field %d", (int)i+1 );
    add_brick( &br, field_string );
  }
#endif // TO_BE_SETTLED
}

/*----------------------------------------------------------------------
 * form_base_viewer::add_brick
 */
void
form_base_viewer::add_brick(SuifBrick *br, char* field_name)
{
  char *field_type;
  char *val_string = 0;

#ifdef TO_BE_SETTLED
  const int current_buffer_size=1000;

  char val_buffer[current_buffer_size];

  switch ( br.tag()) {

  case BRICK_NULL:
      val_string="<NULL>";
      field_type = type_BRICK_NULL;
      break;
  case BRICK_OWNED_ZOT:
      field_type = type_BRICK_OWNED_ZOT;
      sprintf(val_buffer, "%p", br.get_zot() );
      val_string = val_buffer;
      break;
  case BRICK_ZOT_REF:
      field_type = type_BRICK_ZOT_REF;
      sprintf(val_buffer, "%p", br.get_zot() );
      val_string = val_buffer;
      break;
  case BRICK_INTEGER:
      field_type = type_BRICK_INTEGER;
      assert( br.get_integer().written_length().c_int()<current_buffer_size );
      br.get_integer().write( val_buffer );
      val_string = val_buffer;
      break;
  case BRICK_STRING:
      field_type = type_BRICK_STRING;
      val_string = br.get_string().chars();
      break;
  case BRICK_BIT_BLOCK:
      field_type = type_BRICK_BIT_BLOCK;
      val_string =  (char*)br.get_bits();
      break;
  default:
    assert( 0, "Unknown brick tag");
  }
#else // TO_BE_SETTLED
  val_string="<NULL>";
  field_type = type_BRICK_NULL;
#endif // TO_BE_SETTLED
  form->add_field(field_name, field_type, (char*)(val_string ? val_string : ""));
}

/*----------------------------------------------------------------------
 * form_base_viewer::get_brick_list
 */
BrickList *
form_base_viewer::get_brick_list(SuifObject* obj, int first_field_num)
{
  BrickList *bricklist = new BrickList();
#if TO_BE_SETTLED
  BrickList *bricklist = create_brick_list(suif_env);

  int n = form->num_fields();
  for (int i = first_field_num; i < n; i++) {
    char *error_msg;

    SuifBrick *br = get_brick(obj, i, error_msg);
    if (error_msg) {
      display_message(this, "%s", error_msg);
      form->focus_field(i);
      delete bricklist;
      return 0;
    }

    bricklist->append_brick( br );
  }
#endif // TO_BE_SETTLED
  return bricklist;
}

/*----------------------------------------------------------------------
 * form_base_viewer::get_brick
 */
SuifBrick*
form_base_viewer::get_brick( SuifObject* obj, int field_num, char*& error_msg)
{
  error_msg = 0;
  char *type = form->get_field_type(field_num);
  char *val = form->get_field_data(field_num);

  assert (val && type);

  SuifBrick *br;

  if ( type == type_BRICK_NULL ) {
    br = create_suif_brick(suif_env);
  } else if ( type == type_BRICK_OWNED_ZOT ) {
      void *ptr;
      sscanf(val,"%p", &ptr );
      br = create_owned_suif_object_brick(suif_env, (SuifObject*)ptr );
  } else if ( type == type_BRICK_ZOT_REF ) {
#ifdef TO_BE_SETTLED
      void *ptr;
      sscanf(val,"%p", &ptr );
      br = ref_brick( (SuifObject*)ptr );
#else /* TO_BE_SETTLED */
     br = create_suif_brick(suif_env);
#endif /* TO_BE_SETTLED */
  } else if ( type == type_BRICK_BIT_BLOCK ) {
#ifdef TO_BE_SETTLED
     br = brick( (unsigned char*)val, strlen(val) );
#else /* TO_BE_SETTLED */
     br = create_suif_brick(suif_env);
#endif /* TO_BE_SETTLED */
  } else if ( type == type_BRICK_INTEGER ) {
      IInteger intval = val;
      if (intval.is_undetermined() ) {
        error_msg = "This is not a number";
      } else {
        br = create_integer_brick(suif_env, intval);
      }
  } else if ( type == type_BRICK_STRING ) {
    br = create_string_brick(suif_env, String(val));
  } else {
    /* unknown type */
    error_msg = "Cannot handle unknown type";
  }
  return br;
}
