/*-------------------------------------------------------------------
 * suif_print.h
 *
 */

#ifndef SUIF_PRINT_H
#define SUIF_PRINT_H

#include "visual/visual.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "suifkernel/group_walker.h"
#include "suif_utils.h"
#include "common/suif_list.h"
#include "iokernel/object_wrapper.h"

class formater : public default_zot_print_helper , protected SelectiveWalker {
  typedef default_zot_print_helper inherited;
public:
  vtty*   _text;
  int     _depth;
  bool _first;
  bool _want_tag;
  int     _detail;  
  vnode*  _vnode;
  bool _want_fold;
  list<vnode *>vnode_list;
  
public:
  formater(SuifEnv *suif, vtty *text,
           int depth=-1 /* =-1 =>print the whole tree */,
	   int detail = PRINT_BRIEF);	

  ApplyStatus operator() (SuifObject *zot) { return Continue; }
  void set_current_filter( const LString **filter ) { _current_filter = filter;}

  virtual void print_zot(const SuifObject *the_zot, fstream& the_ion);
  virtual void print_zot_ref( const SuifObject *the_zot, fstream& the_ion);

  virtual int get_depth() { return _depth; }

  virtual void print_zot_prefix(const SuifObject *the_zot, fstream& the_ion);

  static const LString *mark_filter[];
  static const LString *null_filter[];

  bool start_of_object(ostream& output, const ObjectWrapper &obj,int derefs);
  void end_of_object(ostream& output, const ObjectWrapper &obj);

protected:
  virtual vnode* make_tag_begin( const SuifObject* the_zot, bool want_fold = false );
  virtual void make_tag_end( vnode* vn );

  virtual void handle_zot(SuifObject* the_zot );
  virtual void handle_for_statement( ForStatement* the_zot );
  virtual void handle_scope_statement( ScopeStatement* the_zot );
  virtual void handle_while_statement( WhileStatement* the_zot );
  virtual void handle_zot_with_tag( const SuifObject* the_zot );

  static void print_helper( vtty *text, vnode *tn_vnode,
			    int depth,  int detail,
			    void *pr);
  virtual bool prepend_tag( const SuifObject* );
  virtual bool filter_tag( const SuifObject* );
  const LString **_current_filter;
  static LString *tag_list[];
 
};

#endif // SUIF_PRINT_H
