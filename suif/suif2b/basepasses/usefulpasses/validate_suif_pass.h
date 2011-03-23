#ifndef _VALIDATESUIFPASS_H_
#define _VALIDATESUIFPASS_H_

/** @file
  * Defines class ValidateSuifPass.
  */

#include "suifkernel/module.h"

/** Validate the current FileSetBlock.
  * This pass takes no argument.
  */
class ValidateSuifPass : public Module {
public:
  ValidateSuifPass( SuifEnv* suif_env );
  virtual ~ValidateSuifPass();

  virtual void initialize();

  virtual Module* clone() const;

  virtual void execute();

  virtual bool delete_me() const;

  static const LString get_class_name();

};

// @class ValidateSuifPass validate_suif_pass.h usefulpasses/validate_suif_pass.h


#endif /* _VALIDATESUIFPASS_H_ */

