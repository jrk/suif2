/*-------------------------------------------------------------------
 * info_viewer
 *
 */

#include "info_viewer.h"
#include "suif_event.h"
#include "suif_vnode.h"
#include "suif_menu.h"
#include "code_tree.h"
#include "suif_print.h"
#include "common/suif_list.h"
//#include "basic_type_utils.h"
#include <stdlib.h>
//#include "ion.h"
//#include <zot.h>

/*--------------------------------------------------------------------
 * info_viewer::info_viewer
 */
info_viewer::info_viewer() {
}

/*--------------------------------------------------------------------
 * info_viewer::~info_viewer
 *
 */
info_viewer::~info_viewer() {
}

/*--------------------------------------------------------------------
 * info_viewer::create_window
 */
void info_viewer::create_window() {
  inherited::create_window();

  text->set_text_wrap(true);
  clear();
  view( vman->get_selection() );
}


/*--------------------------------------------------------------------
 * info_viewer::refresh
 */
void info_viewer::refresh() {
  /* set up menus */
  menu->remove( ROOT_MENU );
  create_obj_menu();
  create_edit_menu();
  add_std_go_menu( menu );

  view( vman->get_selection() );
}


/*--------------------------------------------------------------------
 * info_viewer::create_obj_menu
 */
void info_viewer::create_obj_menu() {
  menu->clear( "Object" );
  menu->add_separator( "Object" );
  add_close_command( menu, "Object" );
}


/*--------------------------------------------------------------------
 * info_viewer::create_edit_menu
 */
void info_viewer::create_edit_menu() {
  //@@@  menu->clear("Edit");
  //@@@  add_std_edit_menu(menu, "Edit");
}

/*--------------------------------------------------------------------
 * info_viewer::do_show_obj_cmd
 */
void info_viewer::do_show_obj_cmd(event& ,
                                  info_viewer*,
                                  vnode* vn ) {
  post_event( event( vn, SELECTION ) );
}

/*--------------------------------------------------------------------
 * event handler
 *
 */
void info_viewer::handle_event( event& e ) {
  inherited::handle_event(e);

  switch (e.kind()) {
  case SELECTION:
    {
      if ( e.get_source() == text ) {
	return;	// ignore local selection
      }
      view( e.get_object() );
    }
    break;
  case INVOCATION:
    {
      if ( e.get_source() == text ) {
	/* local event */
	view( e.get_object() );
      }
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
 * info_viewer::clear
 *
 */
void info_viewer::clear() {
  text->clear();
  text->fout() << "No object selected\n";
  text->update();

  /* set up menus */
  menu->remove(ROOT_MENU);
  create_obj_menu();
  create_edit_menu();
  add_std_go_menu( menu );
}

/*--------------------------------------------------------------------
 * info_viewer::view
 *
 */
void info_viewer::view(vnode* vn)
{
  if ( !vn ) return;

  text->clear();
  text->tag_begin( vn );
  fstream& fout = text->fout();
  char *tag = vn->get_tag();

  text->tag_style(BOLD_BEGIN);
  fout << "Object: 0x" << (void*)vn->get_object() << '(' << tag << ")\n";
  text->tag_style(BOLD_END);

  /* properties */
  text->tag_style( BOLD_BEGIN );
  fout << "Properties:\n";
  text->tag_style( BOLD_END );

  list<vprop *> *plist = vn->get_prop_list();
  if ( !plist || plist->empty() ) {
    fout << "<No properties defined>\n";
  } else {
    for ( s_count_t i = 0; i < plist->size(); i++ ) {
      vprop* p = (*plist)[i];
      if ( p->name() ) {
	  char* desc = p->description();
	  fout << '[' << p->name() << "]: " << (desc ? desc : "") << endl;
      }
    }
  }
  fout << endl;

  SuifObject* obj;
  if ( tag == tag_suif_object ) {
    obj = (SuifObject*) vn->get_object();
  } else if ( tag == tag_code_fragment ) {
      code_fragment* f = (code_fragment*) vn->get_object();
      obj = (SuifObject*) f->node();
  } else {
      suif_assert_message(false, ("Unknown tag"));
  }

  text->tag_style( BOLD_BEGIN );
  const MetaClass *m = obj->get_meta_class();
  fout << "Suif object type: " << m->get_class_name().c_str() << "\n\n";
  text->tag_style( BOLD_END );

  // if obj is a file_set_block => print the whole file_set_block
  //   in all other cases just print the obj
  formater f(suif_env, text, -1 ); //is_file_set_block( obj ) ? -1 : 1 );

  if ( is_kind_of<FileSetBlock>( obj ) ) {
    // print only the annotes
    FileSetBlock *fb = to<FileSetBlock>(obj);
    Iter<Annote*> annote_iter = fb->get_annote_iterator();
    for ( int i = 0; i < (int)fb->get_annote_count(); i++ ) {
      Annote* ann = annote_iter.current();
      annote_iter.next();
      vn = create_vnode( ann );
      text->tag_begin( vn );
      f.print_zot( ann, fout );
      text->tag_end( vn );
      fout << endl;
    }
  } else {
    f.print_zot( obj, fout );
    fout << endl;
  } 

  text->tag_end( vn );
  text->update();
}









