#include "common/suif_hash_map.h"
#include "common/suif_vector.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/all_walker.h"
#include "suifkernel/utilities.h"
#include "suifkernel/suif_exception.h"
#include "utils/print_utils.h"
#include <iostream.h>

#include "suif_gc.h"


// 
// Collector - responsible for identifying garbages.
//             Garbages here are symbol table objects from a symbol table.
//             The Collector will traverse from a root object, and mark
//               the symbol table object garbage if it is not reachable
//               from the root object.
//             The Collector marks a symbol table object garbage by removing
//               it from the symbol table and add it to the garbage list.
//             The garbages are deleted in the destructor.
//

class Collector : public AllWalker {
private:
  suif_hash_map<SymbolTableObject*,SymbolTableObject*> _visited_map;
  SymbolTable*                    _symtab;
  suif_vector<SymbolTableObject*> _garbages;
  FileSetBlock* const             _fsb;

protected:
  void set_visited(SymbolTableObject*);
  bool is_visited(SymbolTableObject*);
  void add_garbage(SymbolTableObject*);
public:
  Collector(FileSetBlock* fsb) :
    AllWalker(fsb->get_suif_env()), _visited_map(), _symtab(NULL), _garbages(),
    _fsb(fsb) {};
  Walker::ApplyStatus operator()(SuifObject*);
  void collect(SuifObject* root, SymbolTable*);
  void delete_garbage(void);
};

void Collector::set_visited(SymbolTableObject* sobj)
{
  _visited_map.enter_value(sobj, sobj);
}

bool Collector::is_visited(SymbolTableObject* sobj)
{
  return (_visited_map.find(sobj) != _visited_map.end());
}

void Collector::add_garbage(SymbolTableObject* sobj)
{
  if (!_garbages.is_member(sobj))
    _garbages.push_back(sobj);
}

Walker::ApplyStatus Collector::operator()(SuifObject* obj)
{
  if (obj == _symtab)
    return Walker::Truncate;
  if (is_kind_of<SymbolTableObject>(obj))
    set_visited(to<SymbolTableObject>(obj));
  return Walker::Continue;
}


void Collector::delete_garbage(void)
{
  AndMessageBuffer errs;
  while (!_garbages.empty()) {
    SymbolTableObject* g = _garbages.back();
    _garbages.pop_back();
    if (g->get_parent() != NULL)
      errs.add_message(to_id_string(g) + " still owned by " +
		       to_id_string(g->get_parent()) + " not deleted.");
    else if (SuifGC::is_reachable(_fsb, g))
      errs.add_message(to_id_string(g) + " still reachable, not deleted");
    else
      ;//      delete g;
  }
  if (errs.get_message_count() > 0)
    SUIF_THROW(SuifException(String("GC failed because ") +
			     errs.get_string()));
}




void Collector::collect(SuifObject* root, SymbolTable* symtab)
{
  if (symtab == NULL  || symtab->get_symbol_table_object_count() == 0)
    return;
  _symtab = symtab;
  root->walk(*this);
  list<SymbolTableObject*> l;
  for (Iter<SymbolTableObject*> it = _symtab->
	 get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    SymbolTableObject *symobj = it.current();
    if (!is_visited(symobj)) {
      l.push_back(symobj);
    }
  }
  for (list<SymbolTableObject*>::iterator liter = l.begin();
       liter != l.end(); liter++) {
    SymbolTableObject *symobj = *liter;
    _symtab->remove_symbol(symobj);
    add_garbage(symobj);
  }
  delete_garbage();
}



void SuifGC::collect(FileSetBlock* fsb)
{
  if (fsb == NULL) return;
  Collector c(fsb);
  c.collect(fsb, fsb->get_external_symbol_table());
  c.collect(fsb, fsb->get_file_set_symbol_table());
  for (int i = 0; i < fsb->get_file_block_count(); i++) {
    FileBlock *fb = fsb->get_file_block(i);
    c.collect(fb, fb->get_symbol_table());
  }
}


bool SuifGC::is_reachable(SuifObject* from, SuifObject *to)
{
  class ReachChecker : public AllWalker {
  private:
    SuifObject* const  _dest;
    bool               _bingo;
  public:
    ReachChecker(SuifEnv *env, SuifObject* dst) :
      AllWalker(env), _dest(dst), _bingo(false) {};
    Walker::ApplyStatus operator()(SuifObject *obj) {
      if (obj != _dest) return Walker::Continue;
      _bingo = true;
      return Walker::Stop;
    };
    bool is_reachable(void) { return _bingo; };
  };

  ReachChecker rc(to->get_suif_env(), to);
  from->walk(rc);
  return rc.is_reachable();
}
