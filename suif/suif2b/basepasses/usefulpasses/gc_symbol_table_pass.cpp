
#include "suif_gc.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/command_line_parsing.h"
#include "basicnodes/basic.h"
#include <iostream.h>
#include "gc_symbol_table_pass.h"
#include "common/suif_list.h"
#include "common/suif_hash_map.h"
#include "suifkernel/utilities.h"
#include "utils/expression_utils.h"
#include "utils/trash_utils.h"



const LString GCSymbolTablePass::get_class_name()
{
  static LString name( "gc_symbol_table" );
  return name;
}


GCSymbolTablePass::GCSymbolTablePass( SuifEnv* suif_env ) :
  Module( suif_env )
{
  // override an inherited instance variable
  _module_name = get_class_name();
}


void GCSymbolTablePass::initialize(void)
{
  Module::initialize();
  _command_line -> set_description("Remove unreferenced Types and Symbols from"
				   " external symbol table.");
}


Module* GCSymbolTablePass::clone() const
{
  return (Module*)this;
}

bool GCSymbolTablePass::delete_me() const
{
  return false;
}

#define suif_list list

void GCSymbolTablePass::execute(void)
{
  // Let's try a little optimization.
  // we'll do GC like the trash_it routine.
  // walk through all of the owned objects in the program.
  // for the non-symbols, walk through their sub-objects and
  // find any referenced symbols
  // throw away any that are not needed.
  FileSetBlock *fsb = get_suif_env()->get_file_set_block();
  if (fsb == NULL) return;

  static LString k_trash = "trash";
  // remove the trash annote.  we'll replace it later
  Annote *trash_annote = fsb->take_annote(k_trash);


  suif_hash_map<SymbolTableObject*,bool> referenced_map;
  suif_list<SymbolTableObject*> live_symbols;

  // Now we have a map of Object->parent object
  // We need another one for object -> referenced
  for (Iter<SuifObject> all_iter1 = object_iterator<SuifObject>(fsb);
       all_iter1.is_valid(); all_iter1.next()) {
    SuifObject *obj = &all_iter1.current();

    // for the ownership links of symbols:
    if (is_kind_of<SymbolTable>(obj)) continue;

    if (is_kind_of<SymbolTableObject>(obj)) {
      SymbolTableObject *sym = to<SymbolTableObject>(obj);
      //sym->print_to_default();

      if (referenced_map.find(sym) == referenced_map.end()) {
	//fprintf(stderr, "Found unreferenced Symbol:");
	// referenced_map[sym] = false;
	referenced_map.enter_value(sym, false);
      }
      // we will check these later.
      continue;
    }

    for (Iter<SymbolTableObject> ref_iter = 
	   suif_object_ref_iterator<SymbolTableObject>(obj, 
					    SuifObject::get_class_name());
	 ref_iter.is_valid(); ref_iter.next()) {
      SymbolTableObject *sym_ref = &ref_iter.current();
      
      suif_hash_map<SymbolTableObject*,bool>::iterator find =
	referenced_map.find(sym_ref);
      if (find == referenced_map.end()
	  || (*find).second == false) {
	//fprintf(stderr, "Found Symbol Reference:");
	//sym_ref->print_to_default();
	//referenced_map[sym_ref] = true;
	referenced_map.enter_value(sym_ref, true);
	live_symbols.push_back(sym_ref);
      }
    }
  }

  // Now we have to do a worklist algorithm with the live symbols.
  // any symbol they own or reference must also be live
  while (!live_symbols.empty()) {
    SymbolTableObject *obj = live_symbols.back();
    live_symbols.pop_back();
    // Implicit assumption:
    // it will NEVER OWN another symbol.
    // If it did, we just need to use the object_instance_iterator
    // to add these.
    
    // Everything it references is live.
    for (Iter<SymbolTableObject> ref_iter = 
	   suif_object_ref_iterator<SymbolTableObject>(obj, 
					    SuifObject::get_class_name());
	 ref_iter.is_valid(); ref_iter.next()) {
      SymbolTableObject *sym_ref = &ref_iter.current();
      
      suif_hash_map<SymbolTableObject*,bool>::iterator find =
	referenced_map.find(sym_ref);
      if (find == referenced_map.end()
	  || (*find).second == false) {
	//fprintf(stderr, "Found Symbol reference:");
	//sym_ref->print_to_default();
	// referenced_map[sym_ref] = true;
	referenced_map.enter_value(sym_ref, true);
	live_symbols.push_back(sym_ref);
      }
    }
  }

  if (trash_annote != NULL)
    fsb->append_annote(trash_annote);


  // walk over all of the reference
  // and delete or put back
  for (suif_hash_map<SymbolTableObject*, bool>::iterator obj_iter = 
	 referenced_map.begin();
       obj_iter != referenced_map.end(); obj_iter++) {
    bool is_referenced = (*obj_iter).second;
    if (!is_referenced) {
      SymbolTableObject *sym = (*obj_iter).first;
      //fprintf(stderr, "Trashing unused symbol:");
      //sym->print_to_default();
      SymbolTable *st = sym->get_symbol_table();
      if (st != NULL) {
	if (sym->get_name() != emptyLString) {
	  st->remove_all_from_lookup_table(sym);
	}
	st->remove_symbol_table_object(sym);
      }
      remove_suif_object(sym);
      trash_it(_suif_env, sym);
      //      delete sym;
    }
  }
  //  SuifGC::collect(_suif_env->get_file_set_block());
}
