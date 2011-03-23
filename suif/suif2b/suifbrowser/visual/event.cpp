/*-------------------------------------------------------------------
 * event.cc
 *
 */

#include "event.h"
#include "binding.h"
#include <stdio.h>
#include <stdlib.h>
#include "common/suif_vector.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "iokernel/helper.h"

struct event_binding {
  binding *b;
  int event_k;
};

typedef suif_vector<event_binding*> event_binding_list;


static event_binding_list *Event_Bindings;

/*-------------------------------------------------------------------
 * init_eman
 */
void init_eman(void)
{
  Event_Bindings = new event_binding_list;
}

/*-------------------------------------------------------------------
 * exit_eman
 */
void exit_eman(void)
{
  delete Event_Bindings;
}

/*-------------------------------------------------------------------
 * post_event
 */
void post_event(const event &e)
{
  int k = e.kind();

  /* invoke the event bindings */
  /*for ( s_count_t i=0; i<Event_Bindings->count(); i++ ) {
    event_binding *eb = Event_Bindings->elem( i );
    if (k & eb->event_k) {
      eb->b->invoke(e);
    }*/

  for ( s_count_t i=0; i<Event_Bindings->size(); i++ ) {
    event_binding *eb = (*Event_Bindings)[ i ];
    if (k & eb->event_k) {
      eb->b->invoke(e);
    }
  }
}

/*-------------------------------------------------------------------
 * add_event_binding
 */
void add_event_binding(binding *b, int event_mask)
{
  event_binding *eb = new event_binding;;
  eb->b = b;
  eb->event_k = event_mask;
  //Event_Bindings->append(eb);
  Event_Bindings->push_back(eb);
}

/*-------------------------------------------------------------------
 * remove_event_binding
 */
void remove_event_binding(binding *b)
{
 for ( s_count_t i=0; i<Event_Bindings->size(); i++ ) {
    event_binding *eb = (*Event_Bindings)[i];
    if ( eb->b == b ) {
      Event_Bindings->erase( i );
      //delete eb;
    }
 } 

  //delete_list_and_elements(Event_Bindings);
}

/*-------------------------------------------------------------------
 * set_event_mask
 */
void set_event_mask(binding *b, int event_mask)
{
  /*for ( s_count_t i=0; i<Event_Bindings->count(); i++ ) {
    event_binding *eb = Event_Bindings->elem( i );
    if (eb->b == b) {
      eb->event_k = event_mask;
    }
  }*/

  for ( s_count_t i=0; i<Event_Bindings->size(); i++ ) {
    event_binding *eb = (*Event_Bindings)[i];
    if (eb->b == b) {
      eb->event_k = event_mask;
    }
  }
}
