/*-------------------------------------------------------------------
 * suif_menu
 *
 */

#include <stdlib.h>
#include <string.h>

#include "suif_menu.h"
#include "suif_vnode.h"
#include "ann_form.h"
#include "suif_event.h"
#include "suif_utils.h"
#include "suifkernel/suifkernel_forwarders.h"

//#include <pyg.h>

static void do_show_proc_list( const event &, void *);
static void do_show_proc( const event &, ProcedureDefinition *proc);
static void do_show_fse( const event &, FileBlock *fb);

static void do_edit_annote(const event &, void *);
static void do_remove_annote(const event &, void *);
static void do_add_annote(const event &, void *);

static void do_go_cmd(const event &e, void *);
static void do_go_to_node_cmd(const event &, vnode *vn);
static void do_go_back_cmd(const event &, void *);


void
add_to_procedure_list(list<ProcedureSymbol*> *&procs,
                      SymbolTable *table,
                      bool flag_def_only)
{
  assert( table );
  assert( procs );
  Iter<SymbolTableObject*> symbols_iter =
                                    table->get_symbol_table_object_iterator();

  for ( ; symbols_iter.is_valid(); symbols_iter.next() ) {
    SymbolTableObject* s = symbols_iter.current();
    if (s && s->isKindOf(ProcedureSymbol::get_class_name())) {
      ProcedureSymbol* sym = to<ProcedureSymbol>(s);
      if (flag_def_only && sym->get_definition() == 0) continue;
      // insert sorted
      const char *proc_name = sym->get_name().c_str();
      //list<ProcedureSymbol*>::iterator it = procs->begin();
      unsigned i;
      for (i = 0; i < procs->size(); ++i) {
	if (strcmp((*procs)[i]->get_name().c_str(), proc_name) > 0)
	   break;
      }
      if (i == procs->size()) {
	procs->push_back(sym);
      } else {
        procs->insert(i-1, sym);
      }
    }
  }
}

void
add_to_procedure_list(list<ProcedureSymbol*> *&procs,
                      ProcedureDefinition *def, bool flag_def_only)
{
  assert( def );
  assert( procs );
  add_to_procedure_list(procs, def->get_symbol_table(), flag_def_only);
  // iterate over the procedures
  DefinitionBlock *def_block = def->get_definition_block();
  int count = def_block->get_procedure_definition_count();
  for (int i = 0; i < count; ++i)
  {
    ProcedureDefinition *proc = def_block->get_procedure_definition(i);
    add_to_procedure_list(procs, proc, flag_def_only);
  }
}


list<ProcedureSymbol*> *
get_procedure_list(FileSetBlock *file_set,
                   bool flag_exported,
                   bool flag_static,
                   bool flag_nested,
                   bool flag_def_only)
{
  list<ProcedureSymbol*> *procs = new list<ProcedureSymbol*>;
  FileSetBlock *fsb = file_set;

  // no file_set_block sepcified
  if (!fsb) return procs;

  // =============== exported procedures ================
  if (flag_exported)
    add_to_procedure_list(procs,
                          fsb->get_external_symbol_table(), flag_def_only);

  // =============== static procedures ==================
  if (flag_static) {
    // add the symbols in the static fileset symbol table
    add_to_procedure_list(procs,
                          fsb->get_file_set_symbol_table(), flag_def_only);
    // iterate over the files and add their symbol tables
    for (int i = 0; i < fsb->get_file_block_count(); ++i) {
      FileBlock *file_block = fsb->get_file_block(i);
      add_to_procedure_list(procs,
                            file_block->get_symbol_table(), flag_def_only);
    }
  }

  // =============== nested procedures ==================
  if ( flag_nested ) {
    // iterate over the files
    for (int i=0; i < fsb->get_file_block_count(); ++i) {
      FileBlock *file_block = fsb->get_file_block(i);
      DefinitionBlock * def_block = file_block->get_definition_block();
      // iterate over the procedures
      s_count_t count = def_block->get_procedure_definition_count();
      for (s_count_t proc_counter=0; proc_counter < count; ++proc_counter) {
        ProcedureDefinition *proc = def_block->get_procedure_definition(i);
        add_to_procedure_list(procs, proc, flag_def_only );
      }
    }
  }
  return procs;
}

/*--------------------------------------------------------------------
 * add_std_fse_menu
 *
 */
void
add_std_fse_menu(vmenu* root_menu, char* parent_menu)
{
  /* file set entry submenu */
  FileSetBlock *fsb = suif_env->get_file_set_block();
  if (!fsb) return;
  for (int i = 0; i < fsb->get_file_block_count(); ++i) {
    FileBlock *file = fsb->get_file_block(i);
    binding *b = new binding((bfun) &do_show_fse, file);
    const char* name = file->get_source_file_name().c_str();    
    root_menu->add_command(b, parent_menu, name ? name : "<no name>" );
  }
}

/*--------------------------------------------------------------------
 * add_std_proc_menu
 *
 */
void
add_std_proc_menu(vmenu* root_menu, char* parent_menu)
{
  const unsigned max_procs = 10;
  list<ProcedureSymbol*> *bodies =
                     get_procedure_list(suif_env->get_file_set_block());

  if (bodies->size() <= max_procs) {
    for (s_count_t i = 0; i < bodies->size(); i++) {
      ProcedureSymbol *proc = (*bodies)[i];
      binding *b = new binding((bfun) &do_show_proc, proc);
      root_menu->add_command(b, parent_menu, proc->get_name().c_str() );
    }
    root_menu->add_separator(parent_menu);
  }

  /* add procedure list command to the menu */
  binding *b = new binding((bfun) &do_show_proc_list, 0);
  root_menu->add_command(b, parent_menu, "Procedure List ...");
}

/*--------------------------------------------------------------------
 * show_proc_list
 *
 */
static void do_show_proc_list(const event& , void*)
{
  vman->show_window( vman->find_window_class( "Procedure List" ) );
}


/*--------------------------------------------------------------------
 * show_proc
 *
 */
static void do_show_proc(const event &, ProcedureDefinition* proc)
{
  vnode *vn = create_vnode( proc );
  post_event(event(vn, SELECTION, 0));
}


/*--------------------------------------------------------------------
 * show_fse
 *
 */
static void do_show_fse( const event &, FileBlock *file)
{
  vnode *vn = create_vnode( file );
  post_event(event(vn, SELECTION, 0));
}


/*--------------------------------------------------------------------
 * add_std_edit_menu
 *
 */
void add_std_edit_menu( vmenu* root_menu, char* parent_menu)
{
  binding *b = new binding((bfun) &do_edit_annote, 0);
  root_menu->add_command(b, parent_menu, "Modify Annote..");

  b = new binding((bfun) &do_add_annote, 0);
  root_menu->add_command(b, parent_menu, "Add Annote..");

  b = new binding((bfun) &do_remove_annote, 0);
  root_menu->add_command(b, parent_menu, "Remove Annote");
}


/*--------------------------------------------------------------------
 * edit_annote
 *
 */
static void
do_edit_annote(const event &, void *)
{
  vnode *vn = vman->get_selection();

  if (!vn /* @@@ || !is_generic_annote( (SuifObject*)vn->get_object() ) */ ) {
    display_message(0, "No annote selected. Please select an annote first.");
    return;
  }

  Annote *ann = (Annote *) vn->get_object();
  ann_form *form = new ann_form((AnnotableObject*) ann->get_parent(), ann);
  form->create_window();
}


/*--------------------------------------------------------------------
 * remove_annote
 *
 */
static void
do_remove_annote(const event &, void *)
{
  vnode *vn = vman->get_selection();

  if (vn &&
     ((SuifObject*)vn->get_object())->isKindOf(Annote::get_class_name())) {
    Annote *ann = (Annote *) vn->get_object();
    AnnotableObject *parent = (AnnotableObject*) ann->get_parent();
    ProcedureDefinition *proc = get_procedure_definition( ann );
    parent->remove_annote(ann);


    delete vn;
    delete ann;

    /* post event, notifying that the object has been modified */

    if ( proc ) {
      vnode *p = create_vnode(proc);
      post_event(event(p, PROC_MODIFIED, 0));
    } else {
      FileBlock *fb = get_file_block( parent );
      if (fb) {
        post_event(event(  create_vnode(fb), FSE_MODIFIED, 0));
      } else {
        assert(false);		// this should not happen..
      }
    }
  } else {
    display_message(0, "No annote selected. Please select an annote first.");
    return;
  }
}


/*--------------------------------------------------------------------
 * add_annote
 *
 */
static void
do_add_annote( const event &, void*)
{
  vnode *vn = vman->get_selection();
  if ( (!vn) || (vn->get_tag() != tag_suif_object) ) {
    display_message(0, "Please select a suif object first.");
    return;
  }

  AnnotableObject *obj = (AnnotableObject *) vn->get_object();
  ann_form *form = new ann_form(obj, 0);
  form->create_window();
}


/*--------------------------------------------------------------------
 * add_std_go_menu
 *
 * This is a dynamic menu. When the menu button is activated, the
 * menu items are then constructed.
 */
void
add_std_go_menu(vmenu* root_menu)
{
  binding *b = new binding((bfun) &do_go_cmd, 0);
  root_menu->add_menu(b, "Go");
}


static void
do_go_cmd(const event& e, void*)
{
  binding *b;
  char *node_name;
  char *node_info;

  vmenu *menu = (vmenu *) e.get_source();
  menu->clear("Go");

  b = new binding((bfun) &do_go_back_cmd, 0);
  menu->add_command(b, "Go", "Back");

  menu->add_separator("Go");

  /* construct list of previously selected objects */
  vnode_list *hist = vman->get_selection_history();

  for ( s_count_t i=1; /* jump over current object */
        (i<20) && (i<hist->size()); i++ ) {
    vnode *vn = (*hist)[i];
    char *tag = vn->get_tag();

    if ( tag == tag_suif_object ) {
      SuifObject *obj = (SuifObject *) vn->get_object();
      node_info = (char*)obj->get_meta_class()->get_class_name().c_str();
    } else if (tag == tag_code_fragment) {
      node_info = "";
    } else {
      suif_assert_message( false, ("Unknown tag") );
    }

    node_name = new char[strlen(tag)+strlen(node_info)+100 /*just to be safe*/];

    sprintf(node_name, "[%s] (0x%p) %s", tag, vn->get_object(), node_info);

    b = new binding((bfun) &do_go_to_node_cmd, vn);
    menu->add_command(b, "Go", node_name);

    delete [] node_name;
  }
}


static void
do_go_to_node_cmd( const event&, vnode* vn)
{
  post_event( event( vn, SELECTION, 0 ) );
}


static void
do_go_back_cmd(const event&, void*)
{
  vman->go_back();
}
