/*-------------------------------------------------------------------
 * src_viewer
 *
 */


#include "src_viewer.h"
#include "code_tree.h"
#include "suif_vnode.h"
#include "suif_event.h"
#include "suif_utils.h"
#include "suif_menu.h"
#include <stdlib.h>
#include <string.h>
#include "suifnodes/suif.h"
#include "iokernel/helper.h"

const String src_viewer::no_source_file =
                   String("no source file found for selected object");

class source_id {
public:
  source_id(FileBlock* fb, LString file_name ) :
    _fb(fb), _file_name(file_name) {}
  
  bool operator ==( const source_id& s_id ) {
    return ( ( _fb==s_id._fb ) && 
             ( _file_name==s_id._file_name ) );
  }
private:
  FileBlock* _fb;
  LString _file_name;
};

/*--------------------------------------------------------------------
 * src_viewer::src_viewer
 */
src_viewer::src_viewer() {
  current_file_block = 0;
  current_file_name = no_source_file;
  infobar = 0;
  annote_column = 0;
  stree = 0;
  stree_cache = new list<code_tree*>;
}

/*--------------------------------------------------------------------
 * src_viewer::~src_viewer
 *
 */
src_viewer::~src_viewer() {
  // Note: stree is in the stree_cache, so it will be deleted by
  // clear_cache() method.

  delete infobar;

  clear_cache();
  delete stree_cache;
}

/*--------------------------------------------------------------------
 * src_viewer::clear_cache
 *
 */
void
src_viewer::clear_cache()
{
  delete_list_and_elements(stree_cache);
}

/*--------------------------------------------------------------------
 * src_viewer::create_window
 *
 */
void src_viewer::create_window() {
  inherited::create_window();

  infobar = new vmessage( toplevel );
  annote_column = text->add_column( 4 );
  current_file_name = no_source_file;

  create_file_menu();
  update_infobar();

  show( vman->get_selection() );
}

/*--------------------------------------------------------------------
 * src_viewer::clear
 *
 */
void src_viewer::clear() {
  current_file_block = 0;
  current_file_name = no_source_file; 
  stree = 0;

  text->clear();
 
  refresh();
}

/*--------------------------------------------------------------------
 * src_viewer::refresh
 *
 */
void src_viewer::refresh() {
  menu->remove( ROOT_MENU );
  create_file_menu();

  FileBlock* fb = current_file_block;
  current_file_block = 0;		// to force a redraw
  view( fb );
}

/*--------------------------------------------------------------------
 * src_viewer::create_file_menu
 *
 */
void src_viewer::create_file_menu() {
  menu->clear("File");
  add_std_fse_menu(menu, "File/File Set Block(s)");
  add_std_proc_menu(menu, "File/Procedure");
  menu->add_separator("File");
  add_close_command(menu, "File");
  add_std_go_menu( menu );
}

/*--------------------------------------------------------------------
 * src_viewer::build_stree
 */
void
src_viewer::build_stree()
{
  /* create a new src tree */
  stree = new code_tree;
  stree->set_map_fn((map_tn_fn) &map_tree_node, this);
  stree->id = new source_id( current_file_block, current_file_name );
  stree_cache->push_back( stree );

  post_progress(this, "Loading source file ...", 0);

  /* create link to source */
  DefinitionBlock *def_block = current_file_block->get_definition_block();
  int count = def_block->get_procedure_definition_count();
  for ( int i = 0 ; i<count; i++ ) {
    ProcedureDefinition *proc = def_block->get_procedure_definition(i);
    ExecutionObject* body = proc->get_body();
    if ( body ) {
      stree->build( proc );
    }

    post_progress( this, "Loading SUIF procedures..",
		  ((float) (i+1))/count*100);
  }
  unpost_progress( this );
}

/*--------------------------------------------------------------------
 * src_viewer::print_source
 */
bool src_viewer::print_source() {
  text->clear();

  if ( current_file_name == no_source_file) {
    text->fout() << "No source file for file_block found\n";
  } else {
    String pathname =
          suif_utils::get_path( current_file_block ) + current_file_name;
    /* read and insert file */
    if ( text->insert_file( (char *)pathname.c_str(), true ) ) {
      text->fout() << "Cannot find source file " << pathname.c_str() << endl;
    } else {
      return true;
    }
  }
 
  current_file_name = no_source_file;
  update_infobar();
  text->update();
  return false;
}

/*--------------------------------------------------------------------
 * src_viewer::map_tree_node
 *
 * Return 0 if the source line cannot be found
 */
code_range src_viewer::map_tree_node( SuifObject* node, src_viewer* viewer ) {
  String file;
  int line = find_source_line( node, file).c_int();
  if ( line && // there is an input
       ( suif_utils::is_same_file( file, viewer->get_source_file_name() ) ) ) {
    return ( code_range(line, line) );
  }
  return ( code_range(0, 0) );
}

/*--------------------------------------------------------------------
 * src_viewer::annotate_src_helper
 *
 */
void src_viewer::annotate_src_helper( code_fragment* code_f ) {
  static LString k_doall( "doall" );
  static LString k_memdep_there( "speculate_dependence" );
  static LString k_memdeps_run( "loop_unique_num" );

  for ( code_fragment* f = code_f->child(); f; f = f->next_sib()) {
    annotate_src_helper( f );
    AnnotableObject* node = to<AnnotableObject>( f->node() );
    if ( is_kind_of<ForStatement>( node ) ) {
      if ( node->peek_annote( k_doall ) ) {
        text->set_column_text( annote_column,
                               f->first_line(), "DA" );
      } else if ( node->peek_annote( k_memdeps_run ) &&
                 !node->peek_annote( k_memdep_there ) ) {
        text->set_column_text( annote_column,
			       f->first_line(), "DA?");
      }
    }
  }
}

/*--------------------------------------------------------------------
 * src_viewer::annotate_src
 */
void src_viewer::annotate_src() {
  annotate_src_helper( stree->get_root() );
}

/*--------------------------------------------------------------------
 * src_viewer::update_infobar
 */
void
src_viewer::update_infobar()
{
  String info_line =
      String("Source File: '") + String( current_file_name ) + String( "'" );
  infobar->set_message((char *)info_line.c_str());
}

/*--------------------------------------------------------------------
 * src_viewer::view
 *
 */
void src_viewer::view(FileBlock* fb, String file_name ) {
  if ( fb && ( fb != current_file_block ) || 
             ( !file_name.is_empty() && file_name != current_file_name ) ) {
    current_file_block = fb;

    if ( file_name.is_empty() ) {
      file_name = suif_utils::get_source_file( fb );
    }

    current_file_name = file_name;

    if ( print_source() ) {

      /* check cache */
      stree = 0;
      for ( s_count_t _cnt=0; _cnt<stree_cache->size(); _cnt++ ) {
        code_tree* ct = (*stree_cache)[_cnt];
        source_id* s_id = (source_id*)ct->id;
        if ( (*s_id) == source_id( current_file_block, current_file_name ) ) {
	      stree = ct;
              break;
        }
      }

      if ( !stree ) {
        build_stree();
      }

     /* annotate the source codes */
     annotate_src();

     /* create tags */
     stree->create_tags( text );
    }
  }
  update_infobar();
}

/*--------------------------------------------------------------------
 * src_viewer::view
 *
 * view a tree node
 */
void src_viewer::view( SuifObject* tn, bool select ) {
  if ( !tn ) return;

  ProcedureDefinition* pdef = get_procedure_definition( tn );

  if ( !pdef ) return; 

  String file;
  int line = find_source_line( tn, file ).c_int();
  (void) line;
  
  FileBlock* fb = get_file_block( pdef );
  
  view( fb, file );
  
  /* is there a source file ? */
  if ( !stree ) return;

  /* look up tree node */
  code_fragment* f = stree->lookup( tn );
  if ( f ) {
    vnode* vn = vman->find_vnode( f );
    text->view( f->first_line(), 0 );
    if ( select ) {
      text->select( vn );
    }
  } else {
    String file;
    int line = find_source_line( tn, file ).c_int();
    if ( line ) {
      text->view( line, 0);
      if ( select ) {
	      text->select_line( line );
      }
    }
  }
}

/*--------------------------------------------------------------------
 * event handler
 *
 * when a event occurs, this function is invoked
 */
void src_viewer::handle_event( event& e ) {
  inherited::handle_event( e );

  switch ( e.kind() ) {
  case SELECTION:
    {
      vnode* vn = e.get_object();
      void* event_source = e.get_source();
      if ( event_source == text ) return; // ignore local event
      show( vn );
    }
    break;

  case CLOSE_FILESET:
    {
      clear_cache();
      clear();
    }
    break;

  case NEW_FILESET:
    {
      /* show the first file_block */
      FileSetBlock* file_set = suif_env->get_file_set_block();
      if (file_set) {
        view(file_set->get_file_block(0));
      }
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

/*----------------------------------------------------------------------
 * show
 */
void src_viewer::show( vnode* vn ) {
  if ( !vn ) return;

  char* tag = vn->get_tag();

  if ( tag == tag_suif_object ) {

    SuifObject* obj = (SuifObject*) vn->get_object();
    if ( is_kind_of<FileBlock>( obj ) ) {
      view( to<FileBlock>( obj ) );
    } else if (is_kind_of<ProcedureSymbol>(obj)) {
      ProcedureDefinition* def = to<ProcedureSymbol>(obj)->get_definition();
      view(def, true);
    } else if (obj) {
      view( obj, true );
    }
  } else if (tag == tag_code_fragment) {
    view( ( (code_fragment *) vn->get_object())->node(), true );
  }
}

