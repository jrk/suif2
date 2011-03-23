#ifndef _SUIFLINK_SUIF_VALIDATER_H_
#define _SUIFLINK_SUIF_VALIDATER_H_

/**
  * @file
  * This file contains SuifValidater class.
  */



#include "suifnodes/suif.h"
#include "suifkernel/message_buffer.h"
#include "suifkernel/all_walker.h"

/** The SuifValidater checks for static representation problems in SuifObject.
 *
 * There are two ways to use it:
 * 
 * \code
 *    SuifValidater police;
 *    if (!police.is_valid(object_to_be_checked))
 *      report_error(police.get_error());
 * \endcode
 * 
 * \code
 *    SuifValidater::validate(object_to_be_checked)  // throw SuifException
 * \endcode
 *
 * Currently it went throught the following phases:
 * \arg 1. Traverse only the 'owner' links, make sure the parent is set
 *    right.  Collect all owned objects in a list.
 * \arg 2. Traverse the 'referenced' links, make sure all referenced objects
 *    are in the list from (1).
 * \arg 3. For all symbol tables check the consistency of fields
 *    "lookup_table" and "symbol_table_objects".  Check that
 *     (a). all named objects in "symbol_table_objects" have an entry in
 *          "lookup_table", and
 *     (b). all objects in "lookup_table" also exist in "symbol_table_objects".
 */
class SuifValidater {

  /** An object that walks through a SuifObject and collects all owned
    * sub-object into a SuifValidater.
    */
  class OwnChecker : public SuifWalker {
  private:
     SuifObject*      _owner;
    SuifValidater*    _validater;
    bool              _is_ok;
  public:
    OwnChecker(SuifEnv*,  SuifObject*, SuifValidater*);
    virtual Walker::ApplyStatus operator() (SuifObject *x);
    bool is_ok(void) { return _is_ok; };
  };


  /** An object that walks through a SuifObject and checks that all
    * referenced objects are member of the ownee list in a SuifValidater.
    */
  class RefChecker : public AllWalker {
  private:
    SuifValidater*  _validater;
    bool            _is_ok;
  public:
    RefChecker(SuifEnv*, SuifValidater*);
    Walker::ApplyStatus operator()(SuifObject*);
    bool is_ok(void) { return _is_ok; };
  };
  



 public:
  /** Constructor. */
  SuifValidater(void);

  /** Validate a SuifObject, and recursively validate its sub-objects.
    * Errors are collected in an internal buffer.  Use get_error() to
    * retrieve the error.
    * @param obj the object to be validated.
    * @return true iff no error found in \a obj.
    * @see get_error.
    */
  bool is_valid(SuifObject* obj);

  /** Get all the error collected so far.
    * @return a readable string describing all errors collected.
    */
  String get_error(void);

  /** Validate an object.
    * @param obj the object to be validated.
    * @exception SuifException with message string describing the validation
    *            error.
    */
  static void validate(SuifObject* obj);

 private:
  AndMessageBuffer         _err_buf;
  suif_vector<SuifObject*> _ownees;

  void add_error(String);
  bool is_valid_ownership(SuifObject*);
  void add_ownee(SuifObject*);
  bool is_ownee(SuifObject*);
  bool is_valid_SymbolTable(SymbolTable*);

  friend class OwnChecker;
  friend class RefChecker;
};


#endif // _SUIFLINK_SUIF_VALIDATER_H_
