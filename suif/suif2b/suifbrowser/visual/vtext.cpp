/*-------------------------------------------------------------------
 * vtext
 *
 */

#include "vtext.h"
#include "vtcl.h"
#include "vtagman.h"
#include "vcommands.h"
#include "binding.h"
#include "vnode.h"
#include "vprop.h"
#include "event.h"
#include "vman.h"
#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------*/
/* internal data structures */

#define INDICATOR_SIZE 2	// size of expand/collapse indicator in
				// number of characters.

enum style_kinds {
  BOLD_STYLE,
  ITALIC_STYLE
};

class style_node {
private:
  style_kinds kind;
public:
  text_coord begin;
  text_coord end;

  style_node(style_kinds k) { kind = k; }
  bool is_bold() { return (kind == BOLD_STYLE); }
  bool is_italic() { return (kind == ITALIC_STYLE); }
};


typedef list<style_node*> style_node_list;

/* end internal data structures */
/*----------------------------------------------------------------------*/

/*-------------------------------------------------------------------
 * vtext::vtext
 *
 */
vtext::vtext(vwidget *par) : vtty(par)
{
  tagman = new vtagman;
  current_sel = 0;
  current_tag = 0;

  tcl << "vtext_create" << wpath << this << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::~vtext
 *
 */
vtext::~vtext(void)
{
  delete (tagman);

}

/*-------------------------------------------------------------------
 * vtext::destroy
 *
 */
void vtext::destroy(void)
{
  if (state != W_DESTROYED) {
    state = W_DESTROYED;
    tcl << "vtext_destroy" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vtext::set_text_wrap
 */
void vtext::set_text_wrap(bool wrap)
{
  tcl << "vtext_configure" << wpath <<
    (wrap ? "-wrap char" : "-wrap none") << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::set_top_row
 */
int vtext::get_top_row(void)
{
  tcl << "vtext_get_top_row" << wpath << tcl_end;
  int row;
  tcl >> row;
  return row;
}

/*-------------------------------------------------------------------
 * vtext::add_column
 */
column_id vtext::add_column(int width)
{
  tcl << "vtext_col_create" << wpath << width << tcl_end;
  int col_id;
  tcl >> col_id;

  return (col_id);
}

/*-------------------------------------------------------------------
 * vtext::remove_column
 */
void vtext::remove_column(column_id col)
{
  tcl << "vtext_col_destroy" << wpath << col << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::set_column_text
 */
void vtext::set_column_text(column_id col, int line_num, char *text)
{
  tcl << "vtext_col_set_line" << wpath << col << line_num <<
    text << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::insert_file
 * returns -1 if there is an error, 0 otherwise
 */

//static void print_numbered_line(FILE *fd, int line_num, char *str)
static void print_numbered_line(fstream& fout, int line_num, char *str)
{
  // Print a numbered line
  // Have to handle the tabs properly

#define OFFSET 7
  char buffer[200];

  sprintf(buffer, "%5d: ", line_num);
  char *buffptr = &buffer[OFFSET];

  int new_col = OFFSET;
  int orig_col = 0;

  for (char *ptr = str; *ptr; ptr++) {
    char c = *ptr;
    if (c == '\t') {		// handle tab
      orig_col += 8 - (orig_col & 7);
      int tmp_new_col = new_col + 8 - (new_col & 7);
      if (tmp_new_col == orig_col + OFFSET) {
	*buffptr++ = '\t';
	new_col = tmp_new_col;
      }
    } else {
      while (new_col != orig_col + OFFSET) { // pad spaces
	*buffptr++ = ' ';
	new_col++;
      }
      *buffptr++ = c;
      orig_col++;
      new_col++;
    }
  }
  *buffptr = 0;
  fout << buffer;
  //fprintf(fd, "%s", buffer);
}

int vtext::insert_file(char *filename, bool add_line_num)
{
  // check whether this is a valid input file
  FILE *fd = fopen(filename, "r");
  if (!fd) return (TCL_ERROR);
  int ch;
  while ( ( ch=getc(fd) )!= EOF ) if ( ch<4 ) return (TCL_ERROR);
  fclose(fd);

  if (!add_line_num) {
    update();
    tcl << "vtext_insert_file" << wpath << filename << tcl_end;
    int result;
    tcl >> result;
    return (result);

  } else {

    /* add line numbers */

    FILE *fd = fopen(filename, "r");
    if (!fd) return (TCL_ERROR);

    #undef BUFFER_SIZE
    #define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE + 1];
    int line_num = 1;
    //FILE *output = text_pipe->fd();
    fstream& output = text_pipe->fout();
    while (fgets(buffer, BUFFER_SIZE, fd)) {
      print_numbered_line(output, line_num++, buffer);
    }

    fclose(fd);
    update();
  }
  return (TCL_OK);
}

/*-------------------------------------------------------------------
 * vtext::clear
 */
void vtext::clear(void)
{
  tcl << "vtext_clear" << wpath << tcl_end;
  text_pipe->clear();
  tagman->clear();
  current_sel = 0;
  current_tag = 0;
}

/*-------------------------------------------------------------------
 * vtext::update
 *
 */
void vtext::update(void)
{
  insert_text(tagman->get_root(), false);
}

/*-------------------------------------------------------------------
 * vtext::insert_text
 *
 * This code is somewhat messy, and I believe buggy in certain situations.
 * Need to be cleaned up.
 *
 */
#define BUFFERSIZE 1024
void vtext::insert_text(tag_node *root_node, bool overwrite)
{
  enum { GET_CHAR, GET_OBJ, GET_STYLE};

  int current_state = GET_CHAR;
  unsigned long obj_addr;
  int obj_bytes;
  int style;
  int style_bytes;
  char b[BUFFERSIZE + 1];
  char output[BUFFERSIZE * 2];	// only an estimate of what is needed!

  /* link buffer variable to tcl/tk variable */
  char tcl_command[] = "catch {global v_tmp; unset v_tmp}";
  tcl.eval(tcl_command);
  char *tmp = output;
  if (tcl.link_var("v_tmp", (char *) &tmp,
		   TCL_LINK_STRING | TCL_LINK_READ_ONLY) != TCL_OK) {
    v_warning("Cannot link variable (%s)", tcl.result());
  }

  /*
   * Read from text pipe,
   * and insert into text window
   */
  tag_node_list tag_stack;
  tag_stack.push_back(root_node);
  text_coord end = root_node->get_end_coord();

  int current_row, current_col;
  if (overwrite) {
    text_coord begin = root_node->get_begin_coord();

    tcl << "vtext_delete_text" << wpath << begin.row << begin.col <<
      end.row << end.col << tcl_end;

    assert(begin.col == 0);
    current_row = begin.row;
    current_col = 0;

  } else {
    current_row = end.col ? (end.row + 1) : end.row;
    current_col = 0;
  }

  tag_node_list new_nodes;
  int hide_chars = 0;
  if (root_node->is_expandable()) {
    new_nodes.push_back(root_node);
    hide_chars = INDICATOR_SIZE;
  }

  style_node_list style_nodes;
  style_node *current_bold_node = 0;
  style_node *current_italic_node = 0;

  int result;
  while ((result = text_pipe->read(b, BUFFERSIZE)) > 0) {

    /*
     * state diagram:
     *
     *                    '\01'(tag begin)                v---\
     *  [state 1:GET_CHAR]   ------->      [state 2:GET_ADDR]-/
     *   ^      |     ^                        |
     *   |      |     +------------------------+
     *   |      |                          (when got the whole addr)
     *   \_____/
     *    '\02'(tag end)
     *
     *
     */

    int beg_row = current_row;
    int beg_col = current_col;

    /* print & set up links */
    char *output_ptr = output;
    const char *ptr = b;
    for (int i = 0; i < result; i++) {
      unsigned char c = *ptr++;

      if (current_state == GET_CHAR) {
	switch (c) {
	case '\n':
	  {
	    current_row++;
	    current_col = 0;
	    *output_ptr++ = '\n';
	    hide_chars = 0;
	  }
	  break;

	case '\01':
	  {
	    /*
	     * Beginning of a tag, following this is the address of the tag
	     */
	    current_state = GET_OBJ;
	    obj_addr = 0;
	    obj_bytes = 0;
	  }
	  break;

	case '\02':
	  {
	    /*
	     * End of a tag. Update text coordinate of the tag
	     */
	    assert(!tag_stack.empty());
	    //tag_node *node = tag_stack.pop();
	    tag_node *node = tag_stack.front();
	    node->set_end_coord(text_coord(current_row, current_col));
	
#ifdef DEBUG
	    printf("beg: %d.%d, end: %d.%d\n",
		   node->get_begin_coord().row, node->get_begin_coord().col,
		   current_row, current_col);
#endif
	  }
	  break;

	case '\03':
	  {
	    /*
	     * style token
	     */
	    current_state = GET_STYLE;
	    style = 0;
	    style_bytes = 0;
	  }
	  break;

	default:

	  if (hide_chars) {
	    hide_chars--;
	  } else {
	    *output_ptr++ = c;
	    current_col++;
	  }

	  break;
	}

      } else if (current_state == GET_OBJ) {
	/*
	 * tag address
	 */

	/* get address of object */
	obj_addr = (obj_addr << 4) + c - 1;
	obj_bytes++;

	if (obj_bytes == sizeof(void *) * 2) {
	  /* end of address, got the full address */
	  tag_node *node = (tag_node *) obj_addr;
	  new_nodes.push_back(node);


	  node->set_begin_coord(text_coord(current_row, current_col));

	  if (node->is_expandable()) {
	    // to insert indicator
	    hide_chars = INDICATOR_SIZE;
	  }

	  //tag_node *parent = tag_stack.head();
	  tag_node *parent = (*tag_stack.begin());
	  parent->add_son(node);
	  //tag_stack.push(node);
	  tag_stack.push_back(node);

	  current_state = GET_CHAR;
	}

      } else if (current_state == GET_STYLE) {

	/*
	 * style
	 */

	style = (style << 8) + c;
	style_bytes++;
	if (style_bytes == 2) {
	  if ((style & BOLD_BEGIN) &&
	      !current_bold_node) {
	    current_bold_node = new style_node(BOLD_STYLE);
	    current_bold_node->begin.row = current_row;
	    current_bold_node->begin.col = current_col;
	  }
	  if ((style & BOLD_END) &&
	      current_bold_node) {
	    current_bold_node->end.row = current_row;
	    current_bold_node->end.col = current_col;
	    //style_nodes.append(current_bold_node);
	    style_nodes.push_back(current_bold_node);
	    current_bold_node = 0;
	  }
	  if ((style & ITALIC_BEGIN) &&
	      !current_italic_node) {
	    current_italic_node = new style_node(ITALIC_STYLE);
	    current_italic_node->begin.row = current_row;
	    current_italic_node->begin.col = current_col;
	  }
	  if ((style & ITALIC_END) &&
	      current_italic_node) {
	    current_italic_node->end.row = current_row;
	    current_italic_node->end.col = current_col;
	    style_nodes.push_back(current_italic_node);
	    current_italic_node = 0;
	  }
	  current_state = GET_CHAR;
	}
      }

    } /* end for */

    /* insert text */
    *output_ptr = '\0';
    char tcl_com[100];
    sprintf(tcl_com, "global v_tmp; vtext_insert_text %s %d %d $v_tmp",
	    wpath, beg_row, beg_col);
    tcl.eval(tcl_com);

  } /* end while */

  assert(current_state == GET_CHAR);
#if TO_BE_SETTLED
  suif_assert(tag_stack.size() == 1); // should only have root left
#endif //TO_BE_SETTLED

  /* update indicators */
  for ( s_count_t _cnt=0; _cnt<new_nodes.size(); _cnt++ ) {
    tag_node *node = new_nodes[_cnt];
    if (node->is_expandable()) {
      int row = node->get_begin_coord().row;
      int col = node->get_begin_coord().col;
      if (node->is_expanded()) {
	add_indicator(row, col, true);
      } else {
	add_indicator(row, col, false);
      }
    }
  }

  /* update styles */
  for ( s_count_t i = 0 ; i<style_nodes.size(); i++ ) {
    style_node *node= style_nodes[i];
    if (node->is_bold()) {
      tcl << "vtext_set_bold" << wpath << node->begin.row <<
	node->begin.col << node->end.row << node->end.col << tcl_end;
    } else if (node->is_italic()) {
      tcl << "vtext_set_italic" << wpath << node->begin.row <<
	node->begin.col << node->end.row << node->end.col << tcl_end;
    }
    delete (node);
  }

  /* update root node */
  text_coord orig_end = root_node->get_end_coord();
  text_coord new_end = text_coord(current_row, current_col);
  root_node->set_end_coord(new_end);

  /* Now update other tags */
  tagman->node_update(root_node, orig_end, new_end);

  /* clean up */
  tcl.unlink_var("v_tmp");
  tcl.eval(tcl_command);
}

/*-------------------------------------------------------------------
 * vtext::delete_text
 *
 */
void vtext::delete_text(tag_node *node, text_coord &begin,
			text_coord &end)
{
  text_coord node_end = node->get_end_coord();

  assert(begin >= node->get_begin_coord() &&
	 end <= node_end);

  int new_end_row = node_end.row - (end.row - begin.row);
  int new_end_col = (node_end.row == end.row) ?
    (node_end.col - end.col) : node_end.col;

  tagman->node_update(node, node_end, text_coord(new_end_row, new_end_col));
}

/*-------------------------------------------------------------------
 * vtext::view
 *
 */
void vtext::view(int row, int col)
{
  tcl << "vtext_view" << wpath << row << col << 1 << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::view
 *
 */
void vtext::view(vnode *vn)
{
  tag_node *tag = tagman->lookup(vn);
  if (tag) {
    text_coord coord = tag->get_begin_coord();
    view(coord.row, coord.col);
  }
}

/*-------------------------------------------------------------------
 * vtext::view
 *
 */
void vtext::view(tag_node *tag)
{
  if (tag) {
    text_coord coord = tag->get_begin_coord();
    view(coord.row, coord.col);
  }
}

/*-------------------------------------------------------------------
 * vtext::tag_begin
 *
 */
void *vtext::tag_begin(vnode *obj)
{
  tag_node *node = new tag_node;
  node->set_object(obj);

  //FILE *fd = text_pipe->fd();
  fstream& fout = text_pipe->fout();
  char m[40];
  m[0] = '\01';
  unsigned long node_addr = (unsigned long) node;
  char *ptr = &m[1 + sizeof(void *) * 2];
  *ptr-- = 0;
  for (unsigned i = 0; i < sizeof(void *); i++) {
    *ptr-- = (node_addr & 0xF) + 1;
    node_addr >>= 4;
    *ptr-- = (node_addr & 0xF) + 1;
    node_addr >>= 4;
  }
  //fprintf(fd, "%s", m);
  fout << m;
  return (node);
}

/*-------------------------------------------------------------------
 * vtext::tag_begin
 *
 */
void *vtext::tag_begin(vnode *obj, print_fn fn, int d,
		       void *client_data)
{
  tag_node *node = (tag_node *) tag_begin(obj);
  node->set_print_fn(fn, d, client_data);
  return (node);
}

/*-------------------------------------------------------------------
 * vtext::tag_end
 *
 */
void vtext::tag_end(vnode *)
{
  //fputc('\02', text_pipe->fd());
  text_pipe->fout() << char('\02');
}

/*-------------------------------------------------------------------
 * vtext::tag_style
 *
 */
void vtext::tag_style(text_style style)
{
  fstream& fout = text_pipe->fout();
  fout << char('\03') << char((style >> 8) & 0xFF) << char(style & 0xFF);
}

/*-------------------------------------------------------------------
 * vtext::find_tags
 *
 */
tag_node_list *vtext::find_tags(vnode *obj)
{
  tag_node_list *tags = new tag_node_list;
  tagman->lookup(obj, tags);
  return (tags);
}

tag_node *vtext::find_tag(vnode *obj)
{
  return (tagman->lookup(obj));
}

/*-------------------------------------------------------------------
 * vtext::select_node_at
 *
 */
void vtext::select_node_at(int row, int col, int scope)
{
  tag_node *node = tagman->find_tag(text_coord(row, col), scope);
  if (node) {
    select(node);
  }
}

/*-------------------------------------------------------------------
 * vtext::preselect_node_at
 *
 */
void vtext::preselect_node_at(int row, int col)
{
  tag_node *node = tagman->find_tag(text_coord(row, col), 0);
  if (node) {
    text_coord begin = node->get_begin_coord();
    text_coord end = node->get_end_coord();

    tcl << "vtext_preselect" << wpath << begin.row << begin.col
      << end.row << end.col << tcl_end;
  } else {
    tcl << "vtext_preselect_clear" << wpath << tcl_end;
  }
}

/*-------------------------------------------------------------------
 * vtext::select_line
 *
 */
void vtext::select_line(int row)
{
  tcl << "vtext_select" << wpath << row << 0
    << (row + 1) << 0 << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::select
 */
void vtext::select(vnode *vn, bool add)
{
  if (!add) {
    select_clear();
  }
  current_sel = vn;

  tag_node_list found;
  tagman->lookup(vn, &found);

 for ( s_count_t _cnt=0; _cnt<found.size(); _cnt++ ) {
    tag_node *node = found[_cnt];
    select(node, true);
  }
}

/*-------------------------------------------------------------------
 * vtext::select
 */
void vtext::select(tag_node *node, bool add)
{
  if (!add) {
    select_clear();
  }

  current_sel = (vnode *) (node->get_object());
  text_coord begin = node->get_begin_coord();
  text_coord end = node->get_end_coord();

  tcl << "vtext_select_add" << wpath << begin.row << begin.col
    << end.row << end.col << tcl_end;

  current_tag = node;
}

/*-------------------------------------------------------------------
 * vtext::select_clear
 */
void vtext::select_clear(void)
{
  current_sel = 0;
  tcl << "vtext_select_clear" << wpath << tcl_end;
}

/*-------------------------------------------------------------------
 * vtext::select_expand
 */
void vtext::select_expand(void)
{
  if (current_tag) {
    tag_node *par = tagman->get_parent(current_tag);
    if (par) {
      select(par);
      current_sel = (vnode *) par->get_object();
    }
  }
}


/*-------------------------------------------------------------------
 * vtext::update_props
 *
 */
void vtext::update_props(void)
{
  vprop_list *props = vman->get_prop_list();

  for ( s_count_t i=0; i<props->size(); i++ ) {
    vprop *p = (*props)[i];
    update_prop(p);
  }
}

/*-------------------------------------------------------------------
 * vtext::update_prop
 *
 */
void vtext::update_prop(vprop *p)
{
  /* clear original tag */
  tcl << "vtext_tag_clear" << wpath << p << tcl_end;

  /* configure */
  char param[100];
  sprintf(param,
	  "-foreground {%s} -background {%s} -relief groove -borderwidth 1",
	  p->get_foreground(), p->get_background());
  tcl << "vtext_tag_configure" << wpath << p << param << tcl_end;

  /* add nodes to tag */

  for ( s_count_t _cnt=0; _cnt<p->get_node_list()->size(); _cnt++ ) {
    vnode *vn = (*(p->get_node_list()))[_cnt];

    tag_node_list found;
    tagman->lookup(vn, &found);

   for ( s_count_t _cnt1=0; _cnt1<found.size(); _cnt1++ ) {
     tag_node *node = found[_cnt1];

      text_coord begin = node->get_begin_coord();
      text_coord end = node->get_end_coord();
      tcl << "vtext_tag_add" << wpath << p << begin.row << begin.col <<
	end.row << end.col << tcl_end;
    }
  }
}

/*-------------------------------------------------------------------
 * vtext::read_text
 *
 * Returns a string of text at the specified text region
 */
char *vtext::read_text(text_coord &begin, text_coord &end)
{
  tcl << "vtext_read" << wpath << begin.row << begin.col <<
    end.row << end.col << tcl_end;

  char *result = new char[strlen(tcl.result()) + 1];
  strcpy(result, tcl.result());
  return (result);
}

/*----------------------------------------------------------------------
 * vtext::expand node
 */
void vtext::expand_node(tag_node *node)
{
  if (node->is_expandable() &&
      !node->is_expanded()) {
    node->expand(this);
    insert_text(node, true);	// replace text
  }
}

/*----------------------------------------------------------------------
 * vtext::collapse_node
 */
void vtext::collapse_node(tag_node *node)
{
  if (node->is_expandable() && node->is_expanded()) {
    node->collapse(this);
    insert_text(node, true);	// replace text

  }
}

/*----------------------------------------------------------------------
 * vtext::expand_all
 */
void vtext::expand_node_helper(tag_node *tag)
{
  for (tag_node *node = tag->son; node; node = node->next) {
    expand_node(node);
    expand_node_helper(node);
  }
}

void vtext::expand_all(void)
{
  tag_node *root = tagman->get_root();
  expand_node_helper(root);
}

/*----------------------------------------------------------------------
 * vtext::collapse_all
 */
void vtext::collapse_node_helper(tag_node *tag)
{
  for (tag_node *node = tag->son; node; node = node->next) {
    collapse_node(node);
    collapse_node_helper(node);
  }
}

void vtext::collapse_all(void)
{
  tag_node *root = tagman->get_root();
  collapse_node_helper(root);
}

/*--------------------------------------------------------------------
 * vtext::add_indicator
 *
 */
void vtext::add_indicator(int row, int col, bool on)
{
  tcl << "vtext_add_ind" << wpath << this << row << col <<
    (on ? 1 : 0) << tcl_end;
}

/*--------------------------------------------------------------------*/
/* tk command
 *
 */
int TCLTK_CALLING_CONVENTION vtext::vtext_cmd(ClientData, Tcl_Interp *interp, int argc,
			 char *argv[])
{
  static char *_firstargs[] = {"select", "preselect", "invoke", "destroy",
			      "toggle", 0};
  enum { SELECT = 0, PRESELECT, INVOKE, DESTROY, TOGGLE };

  int a = v_parse_firstarg(interp, argc, argv, _firstargs);
  if (a < 0) {
    return (TCL_ERROR);
  }

  if (argc < 3) {
    v_wrong_argc(interp);
    return (TCL_ERROR);
  }

  vtext *text;
  sscanf(argv[2], "%p", &text);

  switch (a) {
  case SELECT:
    {
      if (argc != 6) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }

      int row, col, expand_scope;
      Tcl_GetInt(interp, argv[3], &row);
      Tcl_GetInt(interp, argv[4], &col);
      Tcl_GetInt(interp, argv[5], &expand_scope);

      vnode *old_sel = text->get_selection();
      vnode *new_sel = 0;
      if (!expand_scope || old_sel == 0) {
	text->select_node_at(row, col, 0);
	new_sel = text->get_selection();
      } else {
	text->select_expand();
	new_sel = text->get_selection();
      }
      if (new_sel) {
	post_event(event(new_sel, SELECTION, text));
      }
    }
    break;

  case PRESELECT:
    {
      if (argc != 5) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }

      int row, col;
      Tcl_GetInt(interp, argv[3], &row);
      Tcl_GetInt(interp, argv[4], &col);

      vnode *old_sel = text->get_selection();
      (void) old_sel; // avoid warning
      text->preselect_node_at(row, col);
    }
    break;

  case INVOKE:
    {
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vnode *sel = text->get_selection();
      if (sel) {
	post_event(event(sel, INVOCATION, text));
      }
    }
    break;

  case DESTROY:
    {
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      text->destroy();
    }
    break;

  case TOGGLE:
    {
      if (argc != 3) {
	v_wrong_argc(interp);
	return (TCL_ERROR);
      }
      vnode *sel = text->get_selection();
      if (sel) {
	tag_node *tag = text->tagman->lookup(sel);
	if (tag->is_expandable()) {
	  if (tag->is_expanded()) {
	    text->collapse_node(tag);
	  } else {
	    text->expand_node(tag);
	  }
	}
	text->select(tag);
      }
    }
    break;

  default:
    return (TCL_ERROR);
  }

  return (TCL_OK);
}
