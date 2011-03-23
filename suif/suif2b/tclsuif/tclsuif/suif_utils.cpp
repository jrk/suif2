#include "suif_utils.h"
#include "common/suif_vector.h"
#include "iokernel/object_factory.h"
#include "iokernel/meta_class.h"
#include "iokernel/meta_class_iter.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/list_meta_class.h"
#include "iokernel/aggregate_meta_class.h"
#include "iokernel/stl_meta_class.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/token_stream.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/utilities.h"
#include "basicnodes/basic.h"
#include "utils/print_utils.h"



/** Find the SuifObject from a object via owner links.
  */
static SuifObject* suif_search_object(SuifObject* root, const char* id)
{
  if (strcmp(to_id_string(root).c_str(), id)==0) return root;
  for (Iter<SuifObject> all_iter1 = object_iterator<SuifObject>(root);
       all_iter1.is_valid(); all_iter1.next()) {
    SuifObject *obj = &all_iter1.current();
    if (strcmp(to_id_string(obj).c_str(), id)==0)
      return obj;
  }
  for (Iter<SuifObject> all_iter1 = object_iterator<SuifObject>(root);
       all_iter1.is_valid(); all_iter1.next()) {
    SuifObject *node = &all_iter1.current();
    for (Iter<SuifObject> iter = suif_object_ref_iterator<SuifObject>(node);
	 iter.is_valid();
	 iter.next()) {
      SuifObject* obj = &iter.current();
      if (strcmp(to_id_string(obj).c_str(), id)==0)
	return obj;
    }
  }
  return 0;
}



/** Find the SuifObject from the current file set block.
  * Search only via the owner link.
  * @param suif_env IN
  * @param id       IN name of the object searching for.
  */
SuifObject* suif_get_object(const char* id, SuifEnv* suif_env)
{
  FileSetBlock *fsb = suif_env->get_file_set_block();
  if (fsb == 0) return 0;
  return suif_search_object(fsb, id);
}



SuifObject* suif_get_object_with_test(const char* id,
				      SuifEnv* suif_env,
				      Tcl_Interp *interp)
{
  SuifObject* sobj = suif_get_object(id, suif_env);
  if (sobj == 0) {
    Tcl_AppendResult(interp, "Cannot find object \"", id, "\"", TCL_STATIC);
    return 0;
  }
  return sobj;
}



/** Get all immediate children (owned objects).
  * @param root     IN  the parent.
  * @param children OUT into which the children are dumped.
  * @return number of immediate children.
  */
unsigned suif_get_children(SuifObject* root, suif_vector<SuifObject*>* children)
{
  if (root == 0) return 0;
  unsigned cnt = 0;
  for (Iter<SuifObject> iter = collect_instance_objects<SuifObject>(root);
       iter.is_valid();
       iter.next()) {
    children->push_back(&(iter.current()));
    cnt++;
  }
  return cnt;
}


/*
  if (root == 0) return 0;
  ObjectFactory* of = root->get_object_factory();
  MetaClass* suif_mc = of->find_meta_class( SuifObject::get_class_name() );

  unsigned cnt = 0;
  Iterator* it = object_iterator(root, root->get_meta_class(), suif_mc,
				 of->get_pointer_meta_class(suif_mc, true));
  for (; it->is_valid(); it->next()) {
    SuifObject** org = (SuifObject**)(it->current());
    children->push_back(*org);
    cnt++;
  }
  delete it;
  return cnt;

}
*/



/** Get all immediate children (owned objects).
  * @param root     IN  the parent.
  * @param children OUT into which the children are dumped.
  * @return number of immediate children.
  */
unsigned suif_get_referenced(SuifObject* root,
			     suif_vector<SuifObject*>* referenced)
{
  if (root == 0) return 0;
  unsigned cnt = 0;
  for (Iter<SuifObject> iter = suif_object_ref_iterator<SuifObject>(root);
       iter.is_valid();
       iter.next()) {
    referenced->push_back(&(iter.current()));
    cnt++;
  }
  return cnt;
}





/** Get the tcl representation of field values.
  */

static int suif_get_value(const Address addr,
			  const MetaClass *mc,
			  Tcl_DString* dstr);



static int get_elementary_value(const MetaClass* mc,
				const Address addr,
				Tcl_DString* dstr)
{
  if (mc->get_instance_name() == LString("String"))
    Tcl_DStringAppendElement(dstr, ((String*)addr)->c_str());
  else if (mc->get_instance_name() == LString("LString"))
    Tcl_DStringAppendElement(dstr, ((LString*)addr)->c_str());
  else if (mc->get_instance_name() == LString("long"))
    Tcl_DStringAppendElement(dstr, String(*(long*)addr).c_str());
  else if (mc->get_instance_name() == LString("int"))
    Tcl_DStringAppendElement(dstr, String(*(int*)addr).c_str());
  else if (mc->get_instance_name() == LString("double"))
    Tcl_DStringAppendElement(dstr, String(*(double*)addr).c_str());
  else if (mc->get_instance_name() == LString("bool"))
    Tcl_DStringAppendElement(dstr, String(*(bool*)addr).c_str());
  else if (mc->get_instance_name() == LString("size_t"))
    Tcl_DStringAppendElement(dstr, String((long)(*(size_t*)addr)).c_str());
  else if (mc->get_instance_name() == LString("IInteger"))
    Tcl_DStringAppendElement(dstr, String(((IInteger*)addr)->c_long()).c_str());
  else
    Tcl_DStringAppendElement(dstr, mc->get_instance_name()+"(TBI)");
  return TCL_OK;
}
      


static int get_list_value(const ListMetaClass* mc,
			  const Address addr,
			  Tcl_DString* dest)
{
  Tcl_DString dstr;
  Tcl_DStringInit(&dstr);
  ListIterator* lit = (ListIterator*)mc->get_iterator(addr);
  for (; lit->is_valid(); lit->next()) {
    suif_get_value(lit->current(), lit->current_meta_class(), &dstr);
  }
  delete lit;
  Tcl_DStringAppendElement(dest, Tcl_DStringValue(&dstr));
  Tcl_DStringFree(&dstr);
  return TCL_OK;
}


static int get_pointer_value(const PointerMetaClass* mc,
			     const Address addr,
			     Tcl_DString* dstr)
{
  const Address baseaddr = *(Address*)addr;
  if (baseaddr == NULL) {
    Tcl_DStringAppendElement(dstr, "");
    return TCL_OK;
  }
  const MetaClass *basemc = mc->get_base_type()->
    get_meta_class(baseaddr);
  return suif_get_value(baseaddr, basemc, dstr);
}


static int get_object_value(const MetaClass* mc,
			    const Address addr,
			    Tcl_DString* dstr)
{
  Tcl_DStringAppendElement(dstr, to_id_string( (SuifObject*)addr ));
  return TCL_OK;
} 


static int get_aggregate_value(const MetaClass* mc,
			       const Address addr,
			       Tcl_DString* dstr)
{
  Tcl_DString values;
  Tcl_DStringInit(&values);
  Iterator* it = mc->get_iterator(addr, Iterator::All);
  for (; it->is_valid(); it->next()) {
    suif_get_value(it->current(), it->current_meta_class(), &values);
  }
  delete it;
  Tcl_DStringAppendElement(dstr, Tcl_DStringValue(&values));
  Tcl_DStringFree(&values);
  return TCL_OK;
}
  


static int suif_get_value(const Address addr, const MetaClass* mc,
			  Tcl_DString *dstr)
{
  if (addr == NULL) {
    Tcl_DStringAppendElement(dstr, "");
    return TCL_OK;
  } else if (mc->is_elementary())
    return get_elementary_value(mc, addr, dstr);
  else if (mc->isKindOf(ObjectAggregateMetaClass::get_class_name()))
    return get_object_value((ObjectAggregateMetaClass*)mc, addr, dstr);
  else if (mc->isKindOf(AggregateMetaClass::get_class_name()))
    return get_aggregate_value((AggregateMetaClass*)mc, addr, dstr);
  else if (mc->isKindOf(PointerMetaClass::get_class_name()))
    return get_pointer_value((PointerMetaClass*)mc, addr, dstr);
  else if (mc->isKindOf(ListMetaClass::get_class_name()))
    return get_list_value((ListMetaClass*)mc, addr, dstr);
  else {
    Tcl_DStringAppendElement(dstr, mc->get_instance_name()+"(TBI)");
    return TCL_OK;
  }
}
  

int suif_get_type(const Address addr, const MetaClass* mc, Tcl_DString* res);


/**
  * {aggregate <metaclass_name> <field_names> <field_types>}
  */
static int get_aggregate_type(const Address addr, 
			      const MetaClass* mc,
			      Tcl_DString* res)
{
  Tcl_DStringAppendElement(res, "aggregate");
  Tcl_DStringAppendElement(res, mc->get_instance_name());

  Tcl_DString names;
  Tcl_DStringInit(&names);
  suif_get_field_names(addr, mc, &names);
  Tcl_DStringAppendElement(res, Tcl_DStringValue(&names));
  Tcl_DStringFree(&names);

  Tcl_DString types;
  Tcl_DStringInit(&types);
  Iterator* it = mc->get_iterator(addr, Iterator::All);
  for (; it->is_valid(); it->next()) {
    suif_get_type(it->current(), it->current_meta_class(), &types);
  }
  delete it;
  Tcl_DStringAppendElement(res, Tcl_DStringValue(&types));
  Tcl_DStringFree(&types);
  return TCL_OK;
}





/** Get the printed form of the type in mc and append to a DString.
  */
int suif_get_type(const Address addr, const MetaClass* mc, Tcl_DString* res)
{
  Tcl_DString dstr;
  Tcl_DStringInit(&dstr);
  if (mc->is_elementary()) {
    Tcl_DStringAppendElement(&dstr, "elementary");
    Tcl_DStringAppendElement(&dstr, mc->get_instance_name());
  } else if (mc->isKindOf(ObjectAggregateMetaClass::get_class_name())) {
    Tcl_DStringAppendElement(&dstr, "object");
    Tcl_DStringAppendElement(&dstr, mc->get_instance_name());
  } else if (mc->isKindOf(AggregateMetaClass::get_class_name())) {
    get_aggregate_type(addr, mc, &dstr);
  } else if (mc->isKindOf(PointerMetaClass::get_class_name())) {
    PointerMetaClass *pmc = to<PointerMetaClass>(mc);
    Tcl_DStringAppendElement(&dstr, "pointer");
    suif_get_type(*(void**)(addr), pmc->get_base_type(), &dstr);
    Tcl_DStringAppendElement(&dstr,
			     pmc->is_owning_pointer() ? "owned" : "referenced");
  } else if (mc->isKindOf(ListMetaClass::get_class_name())) {
    ListMetaClass *lmc = to<ListMetaClass>(mc);
    Tcl_DStringAppendElement(&dstr, "list");
    suif_get_type(addr, lmc->get_element_meta_class(), &dstr);
  } else {
    Tcl_DStringAppendElement(&dstr, "unknown");
    Tcl_DStringAppendElement(&dstr, mc->get_instance_name());
  }
  Tcl_DStringAppendElement(res, Tcl_DStringValue(&dstr));
  Tcl_DStringFree(&dstr);
  return TCL_OK;
} 



int suif_get_field_names(const Address addr,
			 const MetaClass* mc,
			 Tcl_DString *dstr)
{
  Iterator* it = mc->get_iterator(addr, Iterator::All);
  for (; it->is_valid(); it->next()) {
    Tcl_DStringAppendElement(dstr, it->current_name().c_str());
  }
  delete it;
  return TCL_OK;
}



int suif_get_field_values(const Address addr, const MetaClass* mc,
			  Tcl_DString* dstr)
{
  Iterator* it = mc->get_iterator(addr, Iterator::All);
  for (; it->is_valid(); it->next()) {
    suif_get_value(it->current(), it->current_meta_class(), dstr);
  }
  delete it;
  return TCL_OK;
}



int suif_get_field_types(const Address addr,
			 const MetaClass* mc,
			 Tcl_DString* res)
{
  Iterator* it = mc->get_iterator(addr, Iterator::All);
  for (; it->is_valid(); it->next()) {
    suif_get_type(it->current(), it->current_meta_class(), res);
  }
  delete it;
  return TCL_OK;
}
