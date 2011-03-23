#include "suif_linker.h"

#include "common/suif_map.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/utilities.h"
#include "utils/print_utils.h"
#include "usefulpasses/suif_gc.h"
#include "usefulpasses/suif_validater.h"
#include "decl_groups.h"
#include "ansic.h"
#include "basicnodes/basic_factory.h"

#include <iostream.h>
#include <time.h>

LString k_linksuif_temp_filename("linksuif_temp_filename");

void SuifLinker::set_verbose(bool verbose)
{
  _link_verbose = verbose;
}

void SuifLinker::cleanup_symtab(ReplaceMap *rmap)
{
  list<SymbolTableObject *> *sources = rmap->get_sources();

  list<SymbolTableObject *>::iterator it;
  
  for (it = sources->begin();
       it != sources->end();
       ++it) {
    SymbolTableObject* from = *it;
    if (from->get_parent() != 0) {
      SymbolTable* symtab = to<SymbolTable>(from->get_parent());
      symtab->remove_symbol(from);
    }
    from->set_parent(0);
    add_garbage(from);
  }

  delete sources;
}


void SuifLinker::resolve_decl(void)
{
  DeclGroups dg(_suif_env);
  
  if (_link_verbose)
    cerr << "linksuif:   Creating declaration groups.\n";

  for (Iter<SymbolTableObject*> it = _master_fsb->get_external_symbol_table()->
	 get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    SymbolTableObject *sobj = it.current();
    if (is_kind_of<Symbol>(sobj) || is_kind_of<GroupType>(sobj)) {
      dg.add(it.current());
    }
  }

  if (_link_verbose)
    cerr << "linksuif:   Checking declaration groups.\n";
  dg.check(&_errors, _suif_env);

  ReplaceMap rmap;
  if (_link_verbose)
  cerr << "linksuif:   Updating replacement map.\n";
  dg.add_to_replace_map(&rmap);
  
  if (_link_verbose)
    cerr << "linksuif:   Cleaning up symbol table.\n";
  cleanup_symtab(&rmap);
  
  if (_link_verbose)
    cerr << "linksuif:   Replacing symbols.\n";
  rmap.replace(_master_fsb);
}
  
  

SuifLinker::SuifLinker(SuifEnv *suif_env, FileSetBlock *master, String filename) :
  _master_fsb(master),
  _type_map(),
  _errors(),
  _garbage(),
  _suif_env(suif_env)
{
  // mark file source (for errors...)
  for (Iter<SymbolTableObject> iter = 
	 object_iterator<SymbolTableObject>(master);
       iter.is_valid();
       iter.next()) {

    BrickAnnote *annote = create_brick_annote(_suif_env, k_linksuif_temp_filename);
    StringBrick *brick = create_string_brick(_suif_env, filename);
    annote->append_brick(brick);
    iter.current().append_annote(annote);
  }
}



void SuifLinker::add_error(const String& msg)
{
  String err(msg);
  _errors.add_message(err);
}

void SuifLinker::add_garbage(SuifObject* g)
{
  if (!_garbage.is_member(g))
    _garbage.push_back(g);
}


void SuifLinker::delete_garbage(void)
{
  while (!_garbage.empty()) {
    SuifObject* obj = _garbage.back();
    _garbage.pop_back();
    if (SuifGC::is_reachable(_master_fsb, obj))
      add_error(to_id_string(obj) + ", still reachable from " +
		to_id_string(_master_fsb) + ", is not deleted.");
    else
      delete obj;
  }
}


String SuifLinker::get_error_message(void)
{
  return _errors.get_string();
}

int SuifLinker::get_error_count(void)
{
  return _errors.get_message_count();
}


int SuifLinker::add(FileSetBlock *slave_fsb, String filename)
{
  // mark all symbol table objects with the original filename to facilitate useful
  // error reporting.  (strip these temp annotations later...)
  for (Iter<SymbolTableObject> iter = 
	 object_iterator<SymbolTableObject>(slave_fsb);
       iter.is_valid();
       iter.next()) {

    BrickAnnote *annote = create_brick_annote(_suif_env, k_linksuif_temp_filename);
    StringBrick *brick = create_string_brick(_suif_env, filename);
    annote->append_brick(brick);
    iter.current().append_annote(annote);
  }

  try {
    merge_fsb(slave_fsb);
    //    SuifValidater::validate(_master_fsb);
  } catch (SuifException& exp) {
    add_error(exp.get_message());
  }
  return _errors.get_message_count();
}


int SuifLinker::link(void)
{
  try {
    if (_link_verbose)
      cerr << "linksuif: Cleaning up symbol tables.\n";

    cleanup_symtab(&_type_map);

    if (_link_verbose)
      cerr << "linksuif: Replacing symbols.\n";

    _type_map.replace(_master_fsb);
    
    if (_link_verbose)
      cerr << "linksuif: Resolving declarations.\n";

    resolve_decl();
  } catch (SuifException& exp) {
    add_error(exp.get_message());
  }
  
  return _errors.get_message_count();
}
