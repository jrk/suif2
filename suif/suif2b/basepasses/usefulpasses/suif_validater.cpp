#include "suifkernel/suif_exception.h"
#include "suifkernel/utilities.h"
#include "suifkernel/all_walker.h"
#include "utils/print_utils.h"
#include "suif_validater.h"
#include <iostream.h>


// Check that if a owns b, then b's parent is a
//


SuifValidater::OwnChecker::OwnChecker(SuifEnv *env, SuifObject* owner,
				      SuifValidater* val) :
  SuifWalker(env),
  _owner(owner),
  _validater(val),
  _is_ok(true)
{
}


Walker::ApplyStatus SuifValidater::OwnChecker::operator()(SuifObject *obj) {
  _validater->add_ownee(obj);
  if (obj == _owner)
    return Walker::Continue;
  if (_owner != obj->get_parent()) {
    _is_ok = false;
    _validater->add_error(to_id_string(obj) + " owned by " +
			  to_id_string(_owner) + " but has parent " + 
			  to_id_string(obj->get_parent()));
  }
  _is_ok &= _validater->is_valid_ownership(obj);
  return Walker::Truncate;
}










SuifValidater::RefChecker::RefChecker(SuifEnv* env, SuifValidater* val) :
  AllWalker(env),
  _validater(val),
  _is_ok(true)
{
}

Walker::ApplyStatus SuifValidater::RefChecker::operator()(SuifObject* obj)
{
  if (!_validater->is_ownee(obj)) {
    _is_ok = false;
    _validater->add_error(to_id_string(obj) +
			  " is referenced but not in the owners tree.");
  }
  return Walker::Continue;
}



    



SuifValidater::SuifValidater(void) :
  _err_buf(),
  _ownees()
{
}


bool SuifValidater::is_valid( SuifObject* root)
{
  if (root == NULL)
    SUIF_THROW(SuifException("Cannot validate NULL."));
  bool ok_stat = is_valid_ownership(root);
  RefChecker rcheck(root->get_suif_env(), this);
  root->walk(rcheck);
  ok_stat &= rcheck.is_ok();

  for (Iter<SymbolTable> it = object_iterator<SymbolTable>(root);
       it.is_valid();
       it.next()) {
    ok_stat &= is_valid_SymbolTable(&(it.current()));
  }
  return ok_stat;
}
    
String SuifValidater::get_error(void)
{
  return _err_buf.get_string();
}

void SuifValidater::validate( SuifObject* obj)
{
  SuifValidater val;
  if (!val.is_valid(obj)) {
    SUIF_THROW(SuifException(String("Validation of ") + to_id_string(obj) + 
			     " failed because\n" + val.get_error()));
  }
}
				    
void SuifValidater::add_error(String msg)
{
  _err_buf.add_message(msg);
}

bool SuifValidater::is_valid_ownership(SuifObject* root)
{
  OwnChecker ocheck(root->get_suif_env(), root, this);
  root->walk(ocheck);
  return ocheck.is_ok();
}


void SuifValidater::add_ownee( SuifObject* obj)
{
  if (!_ownees.is_member(obj))
    _ownees.push_back(obj);
}

bool SuifValidater::is_ownee( SuifObject* obj)
{
  return _ownees.is_member(obj);
}


static bool is_in_lookup_list( SymbolTableObject *obj,
			       SymbolTable* symtab)
{
  for (Iter<SymbolTable::lookup_table_pair> it =
	 symtab->get_lookup_table_iterator();
       it.is_valid();
       it.next()) {
    if (obj == it.current().second)
      return true;
  }
  return false;
}


bool SuifValidater::is_valid_SymbolTable(SymbolTable* symtab)
{
  if (symtab == NULL)
    SUIF_THROW(SuifException(String("Cannot validate a NULL SymbolTable.")));
  bool ok_stat = true;
  {for (Iter<SymbolTable::lookup_table_pair> it =
	 symtab->get_lookup_table_iterator();
       it.is_valid();
       it.next()) {
    if (!symtab->has_symbol_table_object_member(it.current().second)) {
      ok_stat = false;
      add_error(to_id_string(symtab) + " has a lookup pair <" +
		it.current().first + ", " + to_id_string(it.current().second) +
		"> with dangling object.");
    }
  }}

  {for (Iter<SymbolTableObject*> it =
	 symtab->get_symbol_table_object_iterator();
       it.is_valid();
       it.next()) {
    SymbolTableObject *sobj = it.current();
    if ((sobj->get_name().length() > 0) &&
	!is_in_lookup_list(it.current(), symtab)) {
      ok_stat = false;
      add_error(to_id_string(symtab) + " has " +
		to_id_string(it.current()) + " not in lookup list.");
    }
  }}
  return ok_stat;
}
