/*-------------------------------------------------------------------
 * dynatype.cc
 *
 */

#include "dynatype.h"
#include "vtcl.h"
#include <stdlib.h>
#include "suifkernel/suifkernel_forwarders.h"

dynatype_manager typeman;

/*-------------------------------------------------------------------
 * dynatype_manager::dynatype_manager
 *
 */
dynatype_manager::dynatype_manager(void)
{
  type_list = new dynatype_list;
}

/*-------------------------------------------------------------------
 * dynatype_manager::~dynatype_manager
 *
 */
dynatype_manager::~dynatype_manager(void)
{
  delete (type_list);
}

/*-------------------------------------------------------------------
 * dynatype_manager::register_type
 *
 */
void dynatype_manager::register_type(char *type_name, type_check_fn fn)
{
  dynatype *type = new dynatype((char*)LString(type_name).c_str(), fn);
  type_list->push_back(type);
}

/*-------------------------------------------------------------------
 * dynatype_manager::lookup
 *
 */
dynatype *dynatype_manager::lookup(char *type_name)
{
  //for ( s_count_t _cnt=0; _cnt<type_list->count(); _cnt++ ) {
   // dynatype *type = type_list->elem( _cnt ); 
  
  for ( s_count_t _cnt=0; _cnt<type_list->size(); _cnt++ ) {
    dynatype *type = (*type_list)[_cnt]; 
    if (strcmp(type->type(), type_name) == 0) {
      return type;
    } 
  }
  return (0);
}

/*-------------------------------------------------------------------
 * dynatype_manager::type_check
 *
 */
char *dynatype_manager::type_check(char *type_name, char *value,
				   char *&error_msg)
{
  dynatype *dyn = lookup(type_name);
  if (dyn) {
    return dyn->check_fn()(value, error_msg);
  } else {
    error_msg = "Cannot find specified type.";
    return 0;
  }
}

/*-------------------------------------------------------------------
 * dynatype_manager::reset_type_iter
 */
void dynatype_manager::reset_type_iter(void)
{
  //type_iter = type_list->head_handle();
  type_iter = (*type_list).begin();
}

/*-------------------------------------------------------------------
 * dynatype_manager::next_type
 */
dynatype *dynatype_manager::next_type(void)
{
  dynatype *ret=0;

  //if (!type_iter.is_0()) {
    //ret = type_list->elem( type_iter );
    //type_iter = type_list->next_handle( type_iter );
/******Need to check if this does the same - what does iter.is_0 do?***/
  if (type_iter <= (*type_list).end()) {
    ret = *type_iter;
    type_iter++;
  }
  return ret;
}
