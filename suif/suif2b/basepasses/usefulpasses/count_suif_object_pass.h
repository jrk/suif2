#ifndef _COUNTSUIFOBJECTPASS_H_
#define _COUNTSUIFOBJECTPASS_H_

/**
  * @file 
  * Defines CountSuifObjectPass class.
  *
  */

#include "suifkernel/module.h"

/** Count number of object instances of each subclass of SuifObject
  * and print the results to cout.
  */
class CountSuifObjectPass : public Module {
public:
  CountSuifObjectPass( SuifEnv* suif_env );

  virtual void initialize();

  virtual Module* clone() const;

  virtual void execute();

  virtual bool delete_me() const;

  static const LString get_class_name();

};


// @class CountSuifObjectPass count_suif_object_pass.h usefulpasses/count_suif_object_pass.h

#endif /* _COUNTSUIFOBJECTPASS_H_ */

