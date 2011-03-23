#ifndef USEFULPASSES__TIME_MODULE_H
#define USEFULPASSES__TIME_MODULE_H

#include "suifkernel/module.h"

class TimeModule : public Module {
public:
  TimeModule( SuifEnv* suif_env );
  virtual ~TimeModule();

  virtual Module* clone() const;
  virtual bool delete_me() const;

  virtual void execute();
};

#endif
