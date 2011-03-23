/*----------------------------------------------------------------------
 * dynatype.h
 *
 *
 * A "dynatype" is a dynamic type class. This is used for having dynamic
 * typed objects, and allow dynamic type-checking in forms.
 *
 * The "dynatype_manager" class manages all dynamic types. Dynamic types
 * are registered to the manager. There should only be one instance of
 * the manager, called "typeman".
 *
 */

#ifndef DYNATYPE_H
#define DYNATYPE_H

#include "common/suif_vector.h"

typedef char *(*type_check_fn)(char *value, char *&error_msg);

/*----------------------------------------------------------------------
 * Dynamic type
 */

class dynatype {
  char *type_name;
  type_check_fn tc_fn;

public:
  dynatype(char *type, type_check_fn fn) {
    type_name = type;
    tc_fn = fn;
  }

  char *type(void) { return type_name; }
  type_check_fn check_fn(void) { return tc_fn; }
};

//typedef slist_tos<dynatype*> dynatype_list;
  typedef suif_vector<dynatype*> dynatype_list;

/*----------------------------------------------------------------------
 * Dynamic type manager
 */

class dynatype_manager {

private:
  dynatype_list *type_list;
  class dynatype *lookup(char *type_name);

  //tos_handle<dynatype*> type_iter;
  suif_vector<dynatype*>::iterator type_iter;

public:
  dynatype_manager(void);
  ~dynatype_manager(void);

  void register_type(char *type_name, type_check_fn check_fn);
  char *type_check(char *type_name, char *value, char *&error_msg);

  void reset_type_iter(void);
  dynatype *next_type(void);

};

extern dynatype_manager typeman;

#endif

