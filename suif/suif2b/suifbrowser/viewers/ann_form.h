/*----------------------------------------------------------------------
 * ann_form.h
 *
 * The "ann_form" window is an annotation edit form. It displays the
 * immediate values of an unstructured annote, allows the user to edit,
 * insert, delete fields.
 *
 */

#ifndef ANN_FORM_H
#define ANN_FORM_H

#include "base_viewer.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifnodes/suif_forwarders.h"

class ann_form : public form_base_viewer {
  typedef form_base_viewer inherited;

private:
  AnnotableObject *parent;
  Annote *current_obj;
  bool new_annote;

  static void do_update(event &e, ann_form *form);
  static void do_cancel(event &e, ann_form *form);
  static void do_insert(event &e, ann_form *form, char *type);
  static void do_delete(event &e, ann_form *form);

  void notify_of_change();
public:
  ann_form( AnnotableObject *parent, Annote *obj);
  ~ann_form(void);
  virtual char *class_name(void) { return "Annote form"; }
  virtual void destroy(void);

  virtual void create_window(void);
  virtual void handle_event(event &e);
  virtual AnnotableObject* get_parent() { return parent; }

};

#endif // ANN_FORM_H
