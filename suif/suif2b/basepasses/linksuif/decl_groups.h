
#ifndef _SUIFLINK_DECL_GROUPS_H_
#define _SUIFLINK_DECL_GROUPS_H_



#include "common/suif_vector.h"
#include "suifkernel/message_buffer.h"
#include "suifnodes/suif.h"
#include "replace_map.h"
#include <iostream.h>


/** This object is designed to help SuifLinker.
  * A DeclGroups is a collection of DeclGroup.
  * Each DeclGroup is a collection symbol or type objects who want to be merged.
  * E.g. two global variables of the same name but declared in two file set
  * block would belong to one DeclGroup.
  *
  * However, only symbols are actually merged by this object.
  * Types are merged by a different object.  It is included here only
  * for checking name conflicts.
  */ 
class DeclGroups {

  /** A DeclGroup is a collection of objects to be merged together.
    * E.g. procedure definitions and declarations of the same name.
    * Each group has a head symbol, which is the "most defined" object in the
    * collection.  E.g. a procedure definition.
    */
  class DeclGroup {
  public:
    /** Constructor.
      * The head symbol may change if a "more defined" symbol is added to this group.
      * @param head the head symbol of the new DeclGroup.
      */
    DeclGroup(SymbolTableObject* head, SuifEnv *se);

    /** Add an object to this group.
      * @param obj the object to be added.
      * @return true if the add is successful, otherwise return false and \a obj is
      *         not added.
      */
    bool add(SymbolTableObject* obj);

    /** Check for name crash between objects in this group with those in the other
      * group.
      * @param g the other group.
      * @param buf buffer for error messages.
      * @return number of error messages added to \a buf.  Return 0 if no name crash.
      */
    int check(DeclGroup* g, AndMessageBuffer* buf, SuifEnv *suif_env);

    /** Add entries <oldsymbol, newsymbol> to a ReplaceMap so the caller can replace
      * all oldsymbol with newsymbol.
      * @param map the ReplaceMap.
      * @return number of entries added.
      * @see ReplaceMap
      */
    int add_to_replace_map(ReplaceMap* map);

    /** Print content of the group to an ostream.
      * @param out the ostream for printing.
      */
    ostream& print(ostream& out);

  private:
    SuifEnv *_suif_env;
    SymbolTableObject* _head_symbol;
    suif_vector<SymbolTableObject*> _folks;

    bool add_symbol(Symbol*);
    bool add_group(GroupType*);
    bool add_enumerated(EnumeratedType*);
  };

 public:
  DeclGroups(SuifEnv *);
  ~DeclGroups(void);
  void add(SymbolTableObject* sym);
  int check(AndMessageBuffer*, SuifEnv *suif_env);
  int add_to_replace_map(ReplaceMap*);
  ostream& print(ostream&);
 private:
  suif_hash_map<LString, suif_vector<DeclGroup*>* > _groups;
  SuifEnv *_suif_env;
};


#endif // _SUIFLINK_DECL_GROUPS_H_
