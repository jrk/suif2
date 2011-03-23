
#ifndef _SUIFLINK_REPLACE_MAP_H_
#define _SUIFLINK_REPLACE_MAP_H_

#include "basicnodes/basic.h"
#include "common/suif_hash_map.h"
#include <iostream.h>

/** This class replaces all occurences of symbols with new symbols in a SuifObject.
  * It is designed to help SuifLinker to replace symbols; e.g. in replacing
  * declarations with their definitions.
  * Internally it has a mapping of <oldObject, newObject>.
  * This class is responsible for walking through SuifObjects and replacing 
  * old objects with new ones as registered in the internal map.
  */
class ReplaceMap {
 private:
  suif_hash_map<SymbolTableObject*, SymbolTableObject*> _map;
 public:

  /** Constructor. */
  ReplaceMap(void) : _map() {};

  /** Add an entry of <oldObject, newObject> to the internal map.
    * @param oldObject the old symbol to be replaced.
    * @param newObject the new symbol to be replaced with.
    */
  void add_entry(SymbolTableObject* oldObject, SymbolTableObject* newObject);

  /** Find from internal map the new object to replace with.
    * @param old the old object.
    * @return the new symbol associated with \a old in the internal map.
    *         If \a old is not in the map, return NULL.
    */
  SymbolTableObject* map(SymbolTableObject* old);

  /** Same as map() except for Type object.
    */
  Type* map_type(Type*);

  /** Same as map() except for Symbol object.
    */
  Symbol* map_symbol(Symbol*);

  /** Recursively replace all owned and referenced objects from root.
    * @param root the object whose fields need replacement.
    */
  void replace(SuifObject *root);

  /** Get number of entries in the internal map.
    */
  //  unsigned get_entry_count(void);

  /** Get the old object from the i_th entry of the internal map.
    * @return the old object from the \a i th (0 based) entry.
    * @exception SuifException there are less than \a i entries in the internal map.
    */
  //  SymbolTableObject* get_nth_source(unsigned i);

  list<SymbolTableObject *> *get_sources(void);

  /** Print the content of this map.
    * @param out the ostream.
    */
  ostream& print(ostream& out);
};



#endif /* _SUIFLINK_REPLACE_MAP_H_ */
