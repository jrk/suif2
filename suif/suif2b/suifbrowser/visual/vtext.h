/*--------------------------------------------------------------------
 * vtext.h
 *
 */

#ifndef VTEXT_H
#define VTEXT_H

#include "tcltk_calling_convention.h"
#include "vwidget.h"
#include "event.h"
#include "vtcl.h"
#include "vtagman.h"
#include "vtty.h"
#include "vprop.h"

class vnode;
class binding;
class vprop;

typedef int column_id;

/*----------------------------------------------------------------------
 * text widget
 */

class vtext : public vtty {

private:
  vtagman *tagman;		// tags mananger

  vnode *current_sel;		// currently selected object
  tag_node *current_tag;	// currently selected tag

  void add_indicator(int row, int col, bool on);

  void expand_node_helper(tag_node *tag);
  void collapse_node_helper(tag_node *tag);

  /* misc */
  char *read_text(text_coord &begin, text_coord &end);
  void delete_text(tag_node *node, text_coord &begin, text_coord &end);
  void insert_text(tag_node *root, bool overwrite);	// insert text

  void preselect_node_at(int row, int col);

public:
  vtext(vwidget *par);
  ~vtext(void);
  void destroy(void);
  virtual int kind(void) { return WIDGET_TEXT; }

  /* configure */
  void set_text_wrap(bool wrap);

  /* additional columns */
  column_id add_column(int width);
  void remove_column(column_id);
  void set_column_text(column_id col, int line_num, char *text);

  /* text I/O */
  int insert_file(char *filename, bool add_line_num = false);
  virtual void update(void);
  virtual void clear(void);

  /* attributes */
  virtual void tag_style(text_style style);

  /* vnode tags */
  virtual void *tag_begin(vnode *obj);
  virtual void *tag_begin(vnode *obj, print_fn fn, int d,
			  void *client_data);
  virtual void tag_end(vnode *obj);
  tag_node *root_tag(void) { return tagman->get_root(); }
  tag_node *find_tag(vnode *obj);
  tag_node_list *find_tags(vnode *obj);

  /* properties */
  void update_props(void);
  void update_prop(vprop *p);

  /* viewing */
  void view(int row, int col);
  void view(tag_node *tag);
  void view(vnode *vn);
  int get_top_row(void);

  /* selection */
  virtual vnode *get_selection(void) { return current_sel; }

  void select_clear(void);
  void select_node_at(int row, int col, int scope = 0);
  void select_line(int row);
  void select(tag_node *node, bool add = false);
  void select(vnode *vn, bool add = false);
  void select_expand(void);

  /* toggle nodes */
  void expand_node(tag_node *node);
  void collapse_node(tag_node *node);
  void collapse_all(void);
  void expand_all(void);

  /* interface with tcl/tk */
  static int TCLTK_CALLING_CONVENTION vtext_cmd(ClientData clientData, Tcl_Interp *interp, int argc,
			 char *argv[]);

};

#endif
