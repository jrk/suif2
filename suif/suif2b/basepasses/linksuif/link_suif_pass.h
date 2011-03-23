#ifndef _LINKSUIFPASS_H_
#define _LINKSUIFPASS_H_

/**
  * This pass links suif files together according to ANSI C standard.
  *
  * Usage:
  *   link_suif [<suiffile> ...]
  *
  * <Suiffile> is the pathname of a suif file.
  * The content of that file will be read in as a FileSetBlock,
  * which will be linked with the current FileSetBlock in the current
  * SuifEnv.
  *
  * After this pass, the current FileSetBlock will have all the FileBlock
  * from the new file.  All external symbol tables will be merged.
  *
  */

#include "utils/simple_module.h"

class LinkSuifPass : public SimpleModule {
public:
  LinkSuifPass( SuifEnv* suif_env );
  virtual ~LinkSuifPass();
  virtual void execute(suif_vector<LString>*);
  virtual String get_help_string(void) const;

  /** Return the name of this command "link_suif".
    */
  static const LString get_command_name();
 private:
  bool _link_verbose;

};


#endif /* _LINKSUIFPASS_H_ */

