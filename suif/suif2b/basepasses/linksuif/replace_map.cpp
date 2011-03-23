
#include "replace_map.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/object_factory.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/utilities.h"
#include "utils/print_utils.h"
#include "common/suif_map.h"
#include "suif_linker.h"


  
  
// check that from==to,
// conflict with existing entries
// transitive mapping : a->b and b->c becomes a->c and b->c
//
void ReplaceMap::add_entry(SymbolTableObject* from, SymbolTableObject* to)
{
  if (from == to) return;

  //  suif_hash_map<SymbolTableObject*,SymbolTableObject*>::iterator it =
  //  _map.find(from);

  SymbolTableObject* current_to = map(from);

  if (current_to) {
    if (current_to != to) {
      SUIF_THROW(SuifException(String("Map conflict from ") +
			       to_id_string(from) + " to " + to_id_string(to)
			       + " and to " + to_id_string(current_to)));
    }
    else {
      // replacement is already true
      return;
    }
  }
  else {
    // no current mapping
    // enter the map, but make transitive mapping implicit
    _map.enter_value(from, to);
  }
      
  /*
  for (suif_hash_map<SymbolTableObject*,SymbolTableObject*>::iterator it =
	 _map.begin();
       it != _map.end();
       it++) {
    if ((*it).first == from) {   // <from, ?>
      if ((*it).second == to)
	return;
      else
	SUIF_THROW(SuifException(String("Map conflict from ") +
				 to_id_string(from) + " to " + to_id_string(to)
				 + " and to " + to_id_string((*it).second)));
    } else if ((*it).second == from) { // <?, from> -> <?, to>
      (*it).second = to;
    } else if ((*it).first == to) {  // <to, ?> -> add(<from, ?>);
      to = (*it).second;
    }
  }
  _map.enter_value(from, to);
  */
}




SymbolTableObject* ReplaceMap::map(SymbolTableObject* org)
{
  suif_hash_map<SymbolTableObject*,SymbolTableObject*>::iterator it =
	 _map.find(org);
  if (it == _map.end())
    return NULL;
  else {
    // need to recursively find the replacement, since we're not explicitly making
    // replacements based on transitivity
    SymbolTableObject *newrep = map((*it).second);
    if (newrep) {
      // mark the current replacement here (this is path compression)
      (*it).second = newrep;
      return newrep;
    }
    else {
      return (*it).second;
    }
  }
}


Type* ReplaceMap::map_type(Type* otype)
{
  SymbolTableObject* rep = map(otype);
  if (rep == NULL)
    return NULL;
  if (!is_kind_of<Type>(rep))
    SUIF_THROW(SuifException(to_id_string(otype) + " was mapped to a non-type "
			     + to_id_string(rep)));
  return to<Type>(rep);
}

Symbol* ReplaceMap::map_symbol(Symbol* osym)
{
  SymbolTableObject* rep = map(osym);
  if (rep == NULL)
    return NULL;
  if (!is_kind_of<Symbol>(rep))
    SUIF_THROW(SuifException(to_id_string(osym) +
			     " was mapped to a non-symbol " +
			     to_id_string(rep)));
  return to<Symbol>(rep);
}

/*
unsigned ReplaceMap::get_entry_count(void)
{
  return _map.size();
}

SymbolTableObject* ReplaceMap::get_nth_source(unsigned n)
{
  unsigned i = 0;
  for (suif_hash_map<SymbolTableObject*, SymbolTableObject*>::iterator it =
	 _map.begin();
       it != _map.end();
       it++) {
    if (i == n)
      return (*it).first;
    else
      i++;
  }
  SUIF_THROW(SuifException(String("Index overflow on a ReplaceMap : ") + 
	                   String(long(i)) + ", current size: " +
			   String(long(get_entry_count()))));
}
*/

list<SymbolTableObject *> * ReplaceMap::get_sources(void)
{
  list<SymbolTableObject *> *sources = new list<SymbolTableObject *>;

   for (suif_hash_map<SymbolTableObject*, SymbolTableObject*>::iterator it =
	 _map.begin();
       it != _map.end();
       it++) {

     sources->push_back( (*it).first );
   }

   return sources;
}
  
  


ostream& ReplaceMap::print(ostream& out)
{
  out << "ReplaceMap_" << unsigned(this) << " {" << endl;
  for (suif_hash_map<SymbolTableObject*, SymbolTableObject*>::iterator it =
	 _map.begin();
       it != _map.end();
       it++) {
    out << "  " << to_id_string((*it).first) << " -> " <<
	to_id_string((*it).second) << endl;
  }
  out << "}" << endl;
  return out;
}

static size_t hash(SuifObject **a) {
  size_t i = (long)a;
  return (i >> 2) + (i >> 10);
}



// Recursively call replace() via owned links only.
//
void ReplaceMap::replace(SuifObject *root)
{
  if (root == NULL) return;
  ObjectFactory* of = root->get_object_factory();
  MetaClass* suif_mc = of->find_meta_class( SymbolTableObject::get_class_name() );

  // referenced links
  suif_hash_map<SuifObject**, SymbolTableObject*> orders;

  Iterator* it = object_iterator_ut(ObjectWrapper(root),
				 of->get_pointer_meta_class(suif_mc, false));
  for (; it->is_valid(); it->next()) {
    SuifObject** org = (SuifObject**)(it->current());
    if (!is_kind_of<SymbolTableObject>(*org)) continue;
    SymbolTableObject* rep = map(to<SymbolTableObject>(*org));
    if (rep == NULL) continue;
    orders.enter_value(org, rep);
  }
  delete it;

  // owned links
  it = object_iterator_ut(ObjectWrapper(root),
		       of->get_pointer_meta_class(suif_mc, true));
  for (; it->is_valid(); it->next()) {
    SuifObject** org = (SuifObject**)it->current();
    if (is_kind_of<SymbolTableObject>(*org)) {
      SymbolTableObject* rep = map(to<SymbolTableObject>(*org));
      if (rep != NULL) {
	orders.enter_value(org, rep);
	continue;
      }
    }
    replace(*org);
  }
  delete it;

  for (suif_hash_map<SuifObject**, SymbolTableObject*>::iterator it =
	 orders.begin();
       it != orders.end();
       it++) {
    *((*it).first) = (*it).second;
  }
}


/*
// Recursively call replace() via owned links only.
//
void ReplaceMap::replace(SuifObject *root)
{
  if (root == NULL) return;
  ObjectFactory* of = root->get_object_factory();
  MetaClass* suif_mc = of->find_meta_class( SuifObject::get_class_name() );

  // referenced links
  suif_hash_map<SuifObject**, SymbolTableObject*> orders;
  Iterator* it = object_iterator_ut(ObjectWrapper(root), suif_mc,
				 of->get_pointer_meta_class(suif_mc, false));
  for (; it->is_valid(); it->next()) {
    SuifObject** org = (SuifObject**)(it->current());
    if (!is_kind_of<SymbolTableObject>(*org)) continue;
    SymbolTableObject* rep = map(to<SymbolTableObject>(*org));
    if (rep == NULL) continue;
    orders.enter_value(org, rep);
  }
  delete it;
  for (suif_hash_map<SuifObject**, SymbolTableObject*>::iterator it = orders.begin();
       it != orders.end();
       it++) {
    *((*it).first) = (*it).second;
  }
  // owned links
  suif_hash_map<SuifObject**, SymbolTableObject*> ref_orders;  
  it = object_iterator_ut(ObjectWrapper(root), suif_mc,
		       of->get_pointer_meta_class(suif_mc, true));
  for (; it->is_valid(); it->next()) {
    SuifObject** org = (SuifObject**)it->current();
    if (is_kind_of<SymbolTableObject>(*org)) {
      SymbolTableObject* rep = map(to<SymbolTableObject>(*org));
      if (rep != NULL) {
	ref_orders.enter_value(org, rep);
	continue;
      }
    }
    replace(*org);
  }
  delete it;
  for (suif_hash_map<SuifObject**, SymbolTableObject*>::iterator it =
	 ref_orders.begin();
       it != ref_orders.end();
       it++) {
    *((*it).first) = (*it).second;
  }
}
*/
