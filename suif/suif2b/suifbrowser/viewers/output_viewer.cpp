/*-------------------------------------------------------------------
 * output_viewer
 *
 */


#include "output_viewer.h"
#include "suif_vnode.h"
#include "code_tree.h"
#include "suif_utils.h"
#include "suif_event.h"
#include "suifnodes/suif.h"
#include "iokernel/helper.h"
#include <stdlib.h>
#include <string.h>

#define NORMAL_STATE 0
#define COMMENT_STATE 1

static char *preprocess_c( char *line, int &state, char *&current_src_file,
			  int &current_src_line, bool &found_line_num );


const String output_viewer::empty_file = String("<no output file specified>");

/*--------------------------------------------------------------------
 */
struct output_node {
  int src_line;
  const char* src_file;
  int out_line;
  SuifObject* tn;

  bool mapped;		// used by map_tree_node

  output_node(int s_line, char* s_file, int o_line) {
    src_line = s_line;
    src_file = LString(s_file).c_str();
    out_line = o_line;
    mapped = false;
  }
};

typedef list<output_node*> output_node_list;

struct scope_node {
  ProcedureDefinition *proc;

  int first_line;
  int last_line;

  list<output_node*> nodes;
};

typedef list<scope_node*> scope_node_list;


/*--------------------------------------------------------------------
 * output_viewer::output_viewer
 */
output_viewer::output_viewer()
{
  current_file = empty_file;
  outtree = 0;

  proc_scopes = new scope_node_list;
}

/*--------------------------------------------------------------------
 * output_viewer::~output_viewer
 *
 */
output_viewer::~output_viewer()
{
  delete outtree;
  delete infobar;
  delete proc_scopes;
}

/*--------------------------------------------------------------------
 * output_viewer::create_window
 */
void
output_viewer::create_window()
{
  inherited::create_window();
  infobar = new vmessage(toplevel);

  /* menu */
  binding *b = new binding( (bfun) &do_open_cmd, this );
  menu->add_command( b, "File", "Open Output File ..." );
  add_close_command( menu, "File" );

  update_infobar();

  show( vman->get_selection() );
}

/*--------------------------------------------------------------------
 * output_viewer::erase_mappings()
 *
 */
void
output_viewer::erase_mappings()
{
  int count = proc_scopes->size();
  for (int i = 0; i < count; i++)
     proc_scopes->pop_front(); 
}

/*--------------------------------------------------------------------
 * output_viewer::do_open_cmd
 *
 */
void
output_viewer::do_open_cmd(event &, output_viewer* viewer)
{
  char filename[1000];
  select_file( viewer, filename, "Load output file:" );
  if ( filename[0] == 0) {
    return;
  }
  viewer->view( 0, String( filename ) );
}

/*--------------------------------------------------------------------
 * output_viewer::clear
 *
 */
void
output_viewer::clear()
{
  current_file = empty_file;

  delete outtree;
  outtree = 0;
  text->clear();

  update_infobar();
}

/*--------------------------------------------------------------------
 * output_viewer::update_infobar
 */
void
output_viewer::update_infobar()
{
  String line;

  line = String("s2c Output file: '") + current_file + String("'");
  infobar->set_message( (char *) line.c_str() );
}

/*--------------------------------------------------------------------
 * output_viewer::print_output
 *
 */
bool
output_viewer::print_output(String file)
{
  const char* filename = file.c_str();
  post_progress(this, "Loading output file ...", 0);

  text->clear();

  FILE *fd = fopen( filename, "r" );
  if (!fd) {
    text->fout() << "Cannot open file " << filename << endl;
    text->update();
    unpost_progress( this );
    return false;
  }

  /* read the file */
  fseek(fd, 0, SEEK_END);
  long length = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  char *buffer = new char[length];
  int l = fread(buffer, 1, length, fd);
  buffer[l] = 0;
  fclose(fd);

  int current_line = 1;
  int current_src_line = -1;
  bool new_src_line = false;
  bool parse_error = false;
  int scope = 0;
  scope_node *current_scope = 0;
  int state = NORMAL_STATE;

  erase_mappings();		// erase previous mappings

  char *next_line;
  char *current_src_file = 0;
  for (char *line = buffer; line; line = next_line) {
    next_line = strchr(line, '\n');
    if (next_line) {
      *next_line++ = 0;
    }

    /*
     * Preprocess C
     */
    bool found_line_num;
    char *ppc = preprocess_c(line, state, current_src_file,
			     current_src_line, found_line_num);
    /*
     * Scan for "{" and "}" to determine current scope.
     */
    for (char *p = ppc; *p; p++) {
      if (*p == '{') {
	if (scope == 0) {
	  current_scope = new scope_node;
	  current_scope->first_line = current_line;
	  current_scope->proc = 0;
	  proc_scopes->push_back(current_scope);
	}
	scope++;

      } else if (*p == '}') {
	scope--;
	if (scope < 0) {
	  parse_error = true;
	}
	if (scope == 0) {
	  current_scope->last_line = current_line;
	}
      }
    }
    delete (ppc);

    /* Check if current line has line number annotation */
    if (found_line_num) {
      new_src_line = true;
      continue;			// don't print it out
    }

    /*
     * Record the line, if nec.
     */
    if (new_src_line) {
      /* enter this line as a node */
      output_node *node = new output_node(current_src_line,
					  current_src_file,
					  current_line);
      if (current_scope) {
	current_scope->nodes.push_back(node);
	
	/* match current scope to procedure */
	if (current_scope->proc == 0) {
	  SuifObject *tn =
	     map_line_to_tree_node( find_file_block(String(current_src_file)),
					            current_src_line);
	  if (tn) {
	    current_scope->proc = get_procedure_definition( tn );
	  }
	}
      }

      new_src_line = false;
    }

    /* print the line out */
    text->fout() << line << endl;
    current_line++;
  }

  delete (buffer);
  text->update();

  if (scope != 0 || parse_error) {
    display_message(this, "Warning: unable to parse the file `%s'.", filename);
  }

  if ( proc_scopes->empty()) {
    display_message(this, "Warning: cannot find line number information in "
		    "the file `%s'.",
		    filename);
    unpost_progress( this );
    return false;
  }

  /*
   * Get the file set entry of this file
   */
  if (!current_fse) {
    if ( current_src_file ) {
      current_fse = find_file_block(current_src_file);
    }
    if (!current_fse) {
      display_message(this,
      	      "Warning: cannot find the corresponding file set entry.");
      unpost_progress(this);
      return false;
    }
  }
  /*
   * Create new output tree
   */
  delete outtree;
  outtree = new code_tree;
  outtree->set_map_fn( (map_tn_fn) &map_tree_node, this );


  // iterate over the procedures
  DefinitionBlock *def_block = current_fse->get_definition_block();
  int num_procs = def_block->get_procedure_definition_count();
  for (int i = 0; i < num_procs; ++i) {

    ProcedureDefinition *proc = def_block->get_procedure_definition(i);

    ExecutionObject *body = proc->get_body();
    if ( body ) {
      outtree->build( proc );
    }

    post_progress(this, "Loading output file..",
		  ((float) (i+1))/num_procs*100);
  }


  /* create tags */
  outtree->create_tags(text);

  unpost_progress(this);

  return true;
}

/*--------------------------------------------------------------------
 * preprocess_c
 *
 * This is a very naive C preprocessor that removes / * and * / comments,
 * # directives, escape codes beginning with '\', and strings are ignored.
 *
 * It is assumed that s2c doesn't generate complicated C codes.
 *
 */
static char *
preprocess_c(char *line, int &state, char *&current_src_file,
	     int &current_src_line, bool &found_line_num)
{
  found_line_num = false;
  char *buffer = new char[strlen(line) + 1];
  char *buffer_ptr = buffer;

  for (char *p = line; *p; p++) {
    if (state == COMMENT_STATE) {
      if (p[0] == '*' && p[1] == '/') {
	state = NORMAL_STATE;
	p++;
      }
    } else {
      if (p[0] == '/' && p[1] == '*') {
	state = COMMENT_STATE;

	/* check if this is a line number comment */
	int line_num;
	char src_file[200];
	if (sscanf(p, "/* line: %d \"%s\" */",
		   &line_num, src_file) == 2) {
	
	  /* this line contains the line number and file info */
	  int l = strlen(src_file);
	  if (src_file[l-1] == '\"') src_file[l-1] = 0;
	  current_src_file = (char *) LString(src_file).c_str();
	  current_src_line = line_num;
	  found_line_num = true;
	}
	p++;

      } else {

	*buffer_ptr++ = *p;
      }
    }
  }

  *buffer_ptr = 0;
  return (buffer);
}


/*--------------------------------------------------------------------
 * output_viewer::map_tree_node
 *
 * This is a mapping function from a tree-node to the corresponding
 * line number on the output file
 */
code_range
output_viewer::map_tree_node(SuifObject* tn, output_viewer* viewer)
{

  /* find the src file and src line */
  String file;
  int line = find_source_line( tn, file ).c_int();
  if (line) {

    int possible_line = 0;

    ProcedureDefinition *proc = get_procedure_definition( tn );

    if (!proc) return code_range( INT_MAX, 0 );

    /* iterate through the procedures */
    for ( s_count_t _cnt=0; _cnt<viewer->proc_scopes->size(); _cnt++ ) {
      scope_node *scope = (*viewer->proc_scopes)[_cnt];

      if (scope->proc == proc) {
	
	/* this is the correct procedure */
	if (tn->isKindOf(ProcedureDefinition::get_class_name()) ) {
	  return code_range( scope->first_line, scope->last_line );
	}
	
        for ( s_count_t _cnt1=0; _cnt1 < scope->nodes.size(); _cnt1++ ) {
          output_node *node = scope->nodes[_cnt1];

	  if (node->src_line == line &&
	      !strcmp( node->src_file, file.c_str() ) ) {

	    if (!node->mapped) {
	      node->mapped = true;
	      return code_range(node->out_line, node->out_line);
	    } else {
	      possible_line = node->out_line;
	    }
	  }
	}
      }
    }

    if ( possible_line ) {
      return code_range( possible_line, possible_line );
    } else {
      return code_range( INT_MAX, 0 );
    }
  }
  return code_range( INT_MAX, 0 );
}

/*--------------------------------------------------------------------
 * output_viewer::view
 *
 */
void
output_viewer::view(FileBlock* fb, String out_file)
{
  if (!current_fse || current_fse != fb) {
    clear();

    if ( out_file.is_empty()) {
      out_file = suif_utils::get_source_file( fb );
      out_file.truncate_to_last('.');
      out_file = out_file + "_gen.c";
      // out_file = suif_utils::get_source_file( fb ) + String(".out_c.c");
    }

    if ( fb ) {
      current_fse = fb;
      out_file = suif_utils::get_path( fb ) + out_file;
    }
    print_output( out_file );
    current_file = out_file;
    update_infobar();
  }
}

/*--------------------------------------------------------------------
 * output_viewer::view
 *
 * view a SuifObject
 */
void
output_viewer::view( SuifObject* tn, bool select)
{
  if (!tn) return;

  if (tn->isKindOf(ProcedureSymbol::get_class_name()))
      tn = get_procedure_definition( tn );

  if (!tn) return;

  FileBlock *fb = get_file_block( tn );
  if ( fb ) view( fb );

  if ( !outtree ) return;

  code_fragment* f = outtree->lookup(tn);

  if ( f ) {
    text->view(f->first_line(), 0);
    text->select_clear();

    if ( select ) {
      f->select_code_fragments( text, true );
    }
#ifdef AG

    if (select) {
      vnode *vn = vman->find_vnode(f);
      text->select(vn);
    }
#endif
  }

}

/*--------------------------------------------------------------------
 * event handler
 *
 * when a event occurs, this function is invoked
 */
void
output_viewer::handle_event(event& e)
{
  inherited::handle_event( e );

  switch ( e.kind() ) {
  case SELECTION:
    {
      void* event_source = e.get_source();
      if (event_source == text) return; // ignore local event
      show( e.get_object() );
    }
    break;

  case CLOSE_FILESET:
    {
      clear();
    }
    break;

  default:
    break;
  }
}


/*--------------------------------------------------------------------
 * output_viewer::show
 *
 */
void
output_viewer::show(vnode* vn)
{
  if ( !vn ) return;
  char* tag = vn->get_tag();
  SuifObject* tn;

  if (tag == tag_suif_object) {
    SuifObject* obj = (SuifObject*) vn->get_object();

    if (obj->isKindOf(FileBlock::get_class_name())) {
      view( (FileBlock*) obj);
    } else {
      view( obj, true );
    }
  } else if ( tag == tag_code_fragment ) {
    tn = ((code_fragment *) vn->get_object())->node();
    view(tn, true);
  }
}




