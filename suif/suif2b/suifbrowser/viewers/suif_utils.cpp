/*----------------------------------------------------------------------
 * suif_utils.cc
 *
 */

#include "visual/visual.h"

#include "suif_utils.h"
#include "suif_vnode.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module.h"
#include "suifprinter/suifprinter.h"
#include "suifnodes/suif.h"
#include "suif_print.h"

extern SuifEnv* suif_env;
extern SuifPrinterModule* spm;

class line_suif_visitor : public stopping_suif_visitor {
  IInteger line;
  LString file_name;
public:
  line_suif_visitor(SuifEnv *suif_env): stopping_suif_visitor(suif_env)
  { line = 0; file_name=String(""); }
  virtual IInteger get_line_number() { return line; }
  virtual String get_file_name() { return file_name; }
  virtual void handle_annote( Annote *the_annote ) {
    if (the_annote->get_name() == LString("line")) {
      line      = ::get_line_number( the_annote );
      file_name = ::get_file_name( the_annote );
      stop       = line != 0;
    }
  }
  virtual void handle_azot(AnnotableObject* the_azot ) {
    Iter<Annote*> anot_iter = the_azot->get_annote_iterator();
    s_count_t count = the_azot->get_annote_count();
    for (s_count_t i = 0; i < count; ++i) {
      Annote* an_annote = anot_iter.current(); anot_iter.next();
      handle_annote( an_annote );
    }
  }
};

class find_line_annote_suif_visitor : public stopping_suif_visitor {
  IInteger line;
  SuifObject *node;
public:
  find_line_annote_suif_visitor(SuifEnv *suif, IInteger l) :
  stopping_suif_visitor(suif), line(l), node(0) {}
  virtual SuifObject *get_zot() { return node; }
  virtual void handle_annote(Annote *the_annote) {
   if (the_annote->get_name() == LString("line")) {
     if ( line == ::get_line_number( the_annote ) ) {
       node = the_annote;
       stop = true;
     }
   }
  }
};

class find_file_suif_visitor : public stopping_suif_visitor {
  String file_name;
public:
  find_file_suif_visitor(SuifEnv *suif,  String file ) :
      stopping_suif_visitor(suif), file_name( file ) {}
  virtual bool contains_file() { return stop; }
  virtual void handle_annote(Annote *the_annote) {
   if (the_annote->get_name() == LString("line")) {
     stop = ( file_name == ::get_file_name( the_annote ) );
   }
 }
};


ProcedureDefinition*
get_procedure_definition(SuifObject* node)
{
  ProcedureDefinition* proc = 0;
  ProcedureSymbol* sym = 0;
  if ( node->isKindOf(ProcedureSymbol::get_class_name())) {
    sym = (ProcedureSymbol*) node;
    if (!(sym->get_definition() == 0)) {
      proc = sym->get_definition();
      assert( proc );
      return proc;
    }
  }

  while (node && (!node->isKindOf(ProcedureDefinition::get_class_name()))) {
	node = node->get_parent();
  }
  return (ProcedureDefinition*) node;
}

bool
is_same_procedure(ProcedureDefinition* p1, SuifObject* p2)
{
  ProcedureSymbol* ps = p1->get_procedure_symbol();
  return ( ( p1==p2 ) || ( ps==p2 ) );
}

// returns the FileBlock in which a zot >node< is living in.
// In case of a procedure_symbol the FileBlock in which its
//   definition is living in is returned
FileBlock *
get_file_block(SuifObject* node)
{
  if ( node->isKindOf(ProcedureSymbol::get_class_name()) ) {
    SuifObject* sym = ((ProcedureSymbol*)node)->get_definition();
    if ( sym ) node = sym;
  }
  while (node && (!node->isKindOf(FileBlock::get_class_name()))) {
	node = node->get_parent();
  }
  return (FileBlock*) node;
}


IInteger
get_line_number(Annote *z)
{
  assert(z->get_name() == LString("line"));
#if TO_BE_SETTLED
  return z->component(1).get_integer();
#endif // TO_BE_SETTLED
  return 0;
}


String
get_file_name(Annote *z)
{
  assert(z->get_name() == LString("line"));
#if TO_BE_SETTLED
  return z->component(2).get_string();
#endif // TO_BE_SETTLED
   return "0";
}


// returns whether to stop prematurely (true==prematurely stopped)
bool
iterate_over(SuifObject *start_zot, stopping_suif_visitor *vis,
             s_count_t start_comp, bool do_root)
{

  assert( start_zot != 0 );

  if ( do_root ) {
    (*vis)(start_zot);
    if ( vis->stop_iterating() ) return true;
  }

#if TO_BE_SETTLED
  if ( start_comp > start_zot->num_components() ) return false;

  for (s_count_t i=start_comp; i<start_zot->num_components(); i++) {
    SuifBrick *br = start_zot->component(i);
    if ( br->is_owned_object() ) {
      SuifObject *current = br->get_zot();
      if ( current && iterate_over( current, vis ) ) return true;
    }
  }
#endif // TO_BE_SETTLED
  return false;
}


bool
iterate_next(SuifObject *start_zot, stopping_suif_visitor *vis,
             SuifObject *stop_at, s_count_t start_comp)
{

  // search downwards
  if ( iterate_over( start_zot, vis, start_comp, false ) ) return true;

  // if nothing found search upwards
  if (start_zot==stop_at) return false; // we have searched everything

  assert(start_zot != 0);

  SuifObject *upward_zot=start_zot->get_parent();

  // there is no parent => everything was searched
  if ( !upward_zot ) return false;

#if TO_BE_SETTLED
  for ( start_comp=0; start_comp<upward_zot->num_components(); start_comp++) {
    SuifBrick *br = upward_zot->component( start_comp );
    if ( br->is_owned_zot() && (br->get_zot()==start_zot) ) {
      return iterate_next( upward_zot, vis, stop_at, start_comp+1 );
    }
  }
#endif // TO_BE_SETTLED
  return false;
}


bool
iterate_previous(SuifObject *start_zot, stopping_suif_visitor *vis,
                 SuifObject *stop_at, bool check_self)
{
    // s_count_t start_comp;

 if ( check_self ) {
    (*vis)(start_zot);
    if ( vis->stop_iterating() ) return true;
  }

  SuifObject *upward_zot=start_zot->get_parent();

  if ( !upward_zot || (stop_at==start_zot) ) return false;

#if TO_BE_SETTLED
  // set start_comp to the number indexing the start_zot
  for ( start_comp=upward_zot->num_components(); start_comp != 0; start_comp--) {
    SuifBrick br = upward_zot->component( start_comp-1 );
    if ( br.is_owned_zot() && (br.get_zot()==start_zot) ) {
      break;
    }
  }

  assert( start_zot == upward_zot->component(start_comp-1).get_zot() );

  while ( --start_comp ) { // iterate over siblings to the left of node
    SuifBrick br = upward_zot->component( start_comp-1 );
    if ( br.is_owned_zot() && (br.get_zot()!=stop_at) ) {
      if ( iterate_right_to_left( br.get_zot(), vis ) ) return true;
    }
  }
#endif // TO_BE_SETTLED

  return iterate_previous( upward_zot, vis, stop_at, true );
}


bool
iterate_right_to_left(SuifObject *start_zot, stopping_suif_visitor *vis,
                      s_count_t start_comp, bool do_root)
{

  assert(start_zot != 0);

#if TO_BE_SETTLED
  if (!start_comp) start_comp = start_zot->num_components();

  while ( start_comp ) {
    SuifBrick br = start_zot->component(start_comp-1);
    if ( br.is_owned_zot() ) {
      SuifObject *current = br.get_zot();
      if ( current && iterate_right_to_left( current, vis ) ) return true;
    }
    start_comp--;
  }

  if ( do_root ) {
    vis->apply( start_zot );
    if ( vis->stop_iterating() ) return true;
  }
#endif // TO_BE_SETTLED

  return false;
}

// finds the lower "right" node in a zot tree
SuifObject *
last_zot(SuifObject* z)
{

#if TO_BE_SETTLED
  while ( true ) {
    s_count_t num = z->num_components();
    while ( num ) { // go from right to left
      SuifBrick br = z->component( num-1 );
      if ( br.is_owned_zot() ) {
        z=br.get_zot();
        break;
      }
      num--;
    }
    if (!num) break; // we have reached a leaf zot
  }
#endif // TO_BE_SETTLED
  return z;
}

/*--------------------------------------------------------------------
 * find_source_line
 */
IInteger
find_source_line(SuifObject *node, String &file_name)
{
#if TO_BE_SETTLED
  line_suif_visitor vis(suif_env);

  if ( !node ) return 0;

  // ==== check the node itself ====
  vis.apply( node );
  if (vis.get_line_number() != 0) {
    file_name = vis.get_file_name();
    return vis.get_line_number();
  }

  // ==== check the nodes before ====
  iterate_previous( node, &vis, get_procedure_definition( node ) );
  if ( vis.get_line_number() != 0 ) {
    file_name = vis.get_file_name();
    return vis.get_line_number();
  }

  // ==== last try: get the line number from the next node that has one ====
  iterate_next( node, &vis, get_procedure_definition( node ) );
  if ( vis.get_line_number() != 0 ) {
    file_name = vis.get_file_name();
    return vis.get_line_number();
  }
#endif // TO_BE_SETTLED

  return 0;
}

FileBlock *
find_file_block(String src_file)
{
  s_count_t _cnt;

  FileSetBlock* fsb = suif_env->get_file_set_block();
  s_count_t num_file_blocks = fsb->get_file_block_count();

  /* ======== only one file block? => return this FileBlock */
  if (num_file_blocks == 1) {
    return fsb->get_file_block(0);
  }
  /* ======== look for the name in the FileBlock ======== */
  for ( _cnt = 0 ; _cnt < num_file_blocks; _cnt++ ) {
    FileBlock *fb = fsb->get_file_block(_cnt);
    if ( suif_utils::get_source_file(fb) == src_file ) {
      return fb;
    }
  }

  /* ========== look for the line number annotations too ======== */
#if TO_BE_SETTLED
  find_file_suif_visitor vis(suif_env, src_file );
  for ( _cnt = 0 ; _cnt < num_file_blocks; _cnt++ ) {
    FileBlock *fb = fsb->get_file_block(_cnt);
    iterate_over( fb, &vis );
    if (vis.contains_file()) {
      return fb;
    }
  }
#endif // TO_BE_SETTLED

  return 0;
}

/*--------------------------------------------------------------------
 * map_line_to_tree_node
 *
 * given a src line and the root of a suif subtree,
 * map it to the suif tree node
 *
 */

SuifObject *
map_line_to_tree_node(SuifObject *root, int line)
{
  if (!root) return 0;

#if TO_BE_SETTLED
  find_line_annote_suif_visitor vis(suif_env, line );

  iterate_over( root, &vis );

  return vis.get_zot();
#else // TO_BE_SETTLED
  return 0;
#endif // TO_BE_SETTLED
}

String
suif_utils::get_path(FileBlock* fb)
{
    String name = fb->get_source_file_name();
    const char *str = name.c_str();
    const char *end = strrchr( str, '/' );
    if (end) {
      return name.substr( 0, end-str+1 );
    }
    end = strrchr( str, '\\' );
    if (end) {
      return name.substr( 0, end-str+1 );
    }
    return String("./");
}

bool
suif_utils::is_absolute(String file)
{
  if ( file.is_empty() ) return false;

  const char* str = file.c_str();
  return  ( ( str[0]=='/' ) ||
            ( str[0] && str[1]==':' ) );
}

bool
suif_utils::is_same_file(String name1, String name2)
{
  bool abs1 = is_absolute( name1 );
  bool abs2 = is_absolute( name2 );


  if ( abs1 == abs2 ) {
    // both files have the same prefix
    return name1 == name2;
  }

  if ( name1.is_empty() || name2.is_empty() ) return false;

  // one of them must be absolute
  if ( !(abs1 || abs2) ) {
    return false;
  }

  // does the ending of both files match (?)
  // for example: name1:     xx.cc
  //              name2:  /a/xx.cc
  const char* n1 = name1.c_str();
  const char* n2 = name2.c_str();
  char* ptr;
  if ( ( ptr = strstr( n1, n2 ) ) && (ptr[strlen(n2)]==0 ) ) return true;
  if ( ( ptr = strstr( n2, n1 ) ) && (ptr[strlen(n1)]==0 ) ) return true;

  return name1 == name2;
}



String
suif_utils::get_source_file(FileBlock* fb)
{
  String file_name = fb->get_source_file_name();
#if TO_BE_SETTLED

  if (file_name.is_empty()) {
    // so let's try something else ==> find a line annote and use that filename
    // use iterate previous to avoid getting a line number from an include file
    // [this is just a guess that such a thing could happen]
    line_suif_visitor vis(suif_env);

    iterate_previous( last_zot(fb), &vis, fb );

    file_name = vis.get_file_name();
  }
#endif // TO_BE_SETTLED
  return file_name;
}


SuifObject*
suif_utils::get_zot(vnode* vn)
{
  SuifObject* return_value = 0;
  if ( vn ) {
    char *tag = vn->get_tag();

    if (tag == tag_suif_object) {
      return_value = (SuifObject*) vn->get_object();
    }
  }
  return return_value;
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Copied from suifx/zot_class.cc: Should be appropriately replaced later.
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void default_zot_print_helper::print_zot(const SuifObject *the_zot,fstream&the_ion)
{
    _current_indentation += 4;
    if (the_zot == 0)
      {
        print_zot_prefix(0, the_ion);
        the_ion << "<<NULL>>\n";
      }
    else
      {
        //the_zot->print(the_ion);
        print(the_ion, the_zot);
      }
    _current_indentation -= 4;
}

void default_zot_print_helper::print_zot_ref(const SuifObject *the_zot, fstream&the_ion) 
{
    if (the_zot == 0)
        the_ion << "<<NULL>>";
    else
        //the_zot->print(the_ion);
        print(the_ion, the_zot);
}

void default_zot_print_helper::add_indentation(int additional_amount)
{
    _current_indentation += additional_amount;
}

void default_zot_print_helper::remove_indentation(int removed_amount)
{
    _current_indentation -= removed_amount;
}

void default_zot_print_helper::print_zot_prefix(const SuifObject *the_zot,
                                                fstream&the_ion)
{
    if (the_zot != 0) {
        int prefix_space = 4;
        for (int space_num = 0; space_num < prefix_space; ++space_num)
            the_ion << ' ';
        //the_zot->id_number().print(the_ion);
      }
    else
      {
        int prefix_space = 4;
        for (int space_num = 0; space_num < prefix_space; ++space_num)
            the_ion << ' ';
        the_ion << '0';
      }
    the_ion << ":  ";
    for (int indent_num = 0; indent_num < _current_indentation; ++indent_num)
        the_ion << ' ';
}

void default_zot_print_helper::print_zot_continuation_prefix(
        const SuifObject * the_zot, fstream& the_ion)
{
    int prefix_space = 4;
    for (int space_num = 0; space_num < prefix_space; ++space_num)
        the_ion << ' ';
}
