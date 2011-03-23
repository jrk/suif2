// $Id: printing_maps.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "suifkernel/visitor_map.h"
#include "suifkernel/suif_object.h"
#include "suifkernel/utilities.h"
#include "common/suif_list.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"
#include "osuifnodes/osuif.h"

#include "osuifprint/printing_maps.h"


//default functions for SuifObjects
static void handle_static_process_suif_object(PrintingMaps *printMap,
                                               SuifObject *obj)
{
  int tempIndent, indent;
  char txtAddress[12];
  Address voidObj = (Address)obj;

  if(printMap->is_done()) return;
  //indent the output
  const MetaClass *type = obj->get_meta_class();
  sprintf(txtAddress, "[0x%08X]", (int)voidObj);
  indent = printMap->getIndent();
  for(tempIndent =0; tempIndent < indent; tempIndent++)
    cout << ' ';
  cout << type->get_instance_name() << " ";
  if(obj->isKindOf(SymbolTableObject::get_class_name())){
    //print the name of the symbol
    SymbolTableObject *sto = (SymbolTableObject*)obj;
    cout << sto->get_name() << txtAddress ;
    cout << endl;
    return;
  }//end if

  cout << "{" << endl;

  //check if done
  if(printMap->is_done()){ 
    cout << "}" << endl;
    return;
  }
  //do the processing on the children
  printMap->get_children_map()->apply(obj);

  for(tempIndent =0; tempIndent < indent; tempIndent++)
    cout << ' ';
  cout << "}" << endl;
}


//default function for the children of the suif objects
static void handle_static_children_suif_object(PrintingMaps *printMap,
                                               SuifObject *obj){
  // Use the iterator to find all the suifobjects.
  // Processing is done on each object.
  list<SuifObject *>* the_list = collect_objects<SuifObject>(obj);
  list<SuifObject*>::iterator iter = the_list->begin();
  for(; iter != the_list->end(); iter++){
    SuifObject *child = (*iter);
    if(child == NULL) continue;  
    if(child->get_parent() != obj) continue;
    printMap->incrementIndent();
    printMap->get_process_map()->apply(child);
    printMap->decrementIndent();
    if(printMap->is_done()) return;
  }
}


PrintingMaps::PrintingMaps( SuifEnv *suif_env,
			    const LString &printing_maps_name ):
  _printing_maps_name( printing_maps_name ),
  _done( false ),
  indent( 0 ),
  _process_map( new VisitorMap(suif_env) ),
  _children_map( new VisitorMap(suif_env) )
{
}


// Register default implementations of the visit method in the process map
// and in the children map.
void PrintingMaps::init_suif_object(){
  register_children_visit_method( (Address) this,
				  (VisitMethod) handle_static_children_suif_object,
				  SuifObject::get_class_name() );

  register_process_visit_method( (Address) this,
				 (VisitMethod)handle_static_process_suif_object,
				 SuifObject::get_class_name() );
}

              
void PrintingMaps::register_children_visit_method( Address state,
						   VisitMethod visitMethod,
						   const LString &className) {
  _children_map->register_visit_method(state, visitMethod, className);
}


void PrintingMaps::register_process_visit_method( Address state,
						  VisitMethod visitMethod,
						  const LString &className) {
  _process_map->register_visit_method(state, visitMethod, className);
}


void PrintingMaps::process_a_suif_object(SuifObject *so){
  get_process_map()->apply(so);
}


void PrintingMaps::incrementIndent(){
  indent += 2;
}


void PrintingMaps::decrementIndent(){
  if(indent == 0)
    return;
  indent -= 2;
}


PrintingMaps::PrintingMaps(const PrintingMaps &other):
  _done(false),_process_map(0), _children_map(0)
{
  suif_assert(false);
}

PrintingMaps& PrintingMaps::operator=(const PrintingMaps& other){
  suif_assert(false);
  return (*this);
}

