/*-------------------------------------------------------------------
 * text_viewer
 *
 */

#include "suif_viewer.h"
#include "suif_vnode.h"
#include "suif_event.h"
#include "suif_menu.h"
#include "suif_print.h"
#include "suif_utils.h"

#include "text_viewer.h"
#include <stdlib.h>

/*--------------------------------------------------------------------
 * text_viewer::text_viewer
 */
text_viewer::text_viewer() {
}

/*--------------------------------------------------------------------
 * text_viewer::~text_viewer
 *
 */
text_viewer::~text_viewer() {
}

/*--------------------------------------------------------------------
 * text_viewer::create_window
 */
void text_viewer::create_window() {
  inherited::create_window();

  binding *b = new binding((bfun) &do_open, this);
  menu->add_command(b, "File", "Open..");

  b = new binding((bfun) &do_close, this);
  menu->add_command(b, "File", "Close");
}

/*--------------------------------------------------------------------
 * text_viewer::do_open
 *
 */
void text_viewer::do_open( event &, text_viewer* viewer ) {
  char filename[1000];
  select_file(viewer, filename);
  if (filename[0] == 0) {
    return;
  }
  viewer->open(filename);
}

/*--------------------------------------------------------------------
 * text_viewer::do_close
 *
 */
void text_viewer::do_close( event &, text_viewer* viewer ) {
  viewer->destroy();
}


/*--------------------------------------------------------------------
 * text_viewer::open
 *
 */
void text_viewer::open( char* filename ) {
  text->clear();

  if (text->insert_file(filename) == -1) {
    text->fout() << "Cannot open file " << filename << endl;
    text->update();
  }
}


/*--------------------------------------------------------------------
 * text_viewer::clear
 *
 */
void text_viewer::clear() {
  text->clear();
}


/*--------------------------------------------------------------------
 * text_viewer::insert_text
 *
 */
void text_viewer::insert_text( char* str ) {
  text->fout() << str;
  text->update();
}








