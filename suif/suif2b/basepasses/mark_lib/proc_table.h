#ifndef _PROC_TABLE_H_
#define _PROC_TABLE_H_

/*
 * This object represents a table of procedure symbols.
 * Subclasses of this may contains other information about the procedure.
 */

#include "common/suif_vector.h"
#include "common/lstring.h"
#include "common/MString.h"
#include "basicnodes/basic.h"

class ProcTable {
 protected:
  ProcTable(void) {};
 public:
  virtual bool is_in_table(ProcedureSymbol*) = 0;
};


/*
 * This is a specific kind of ProcTable.  In StaticProcTable, the system
 * procedure names are read from a file.
 */

class StaticProcTable : public ProcTable {
 private:
  suif_vector<LString> _proc_names;
 public:
  StaticProcTable(String filename=String());
  virtual ~StaticProcTable(void) {};
  virtual void process_input_line(String line);

  virtual bool is_in_table(ProcedureSymbol*);
};



#endif // _PROC_TABLE_H_
