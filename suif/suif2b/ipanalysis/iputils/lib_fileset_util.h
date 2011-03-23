#ifndef FILESET_UTIL_H
#define FILESET_UTIL_H

#include "suifnodes/suif_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"


class FileSetMgr {
  FileSetBlock *_real_fileset;
  list<FileSetBlock*> *_lib_filesets;
  bool _suppress_var_sym_warning;
  bool _verbose;

 public:
  FileSetMgr();
  void set_real_fileset(FileSetBlock* fsb);
  void add_library_fileset(FileSetBlock* fsb);

  // read it and then add it to the library fileset list.
  FileSetBlock *add_library_file(SuifEnv *s, const String &inputFileName);

  FileSetBlock *get_real_fileset() const;
  const list<FileSetBlock *> *get_lib_filesets() const;

  

  bool set_suppress_var_sym_warning(bool val);

  // Given a proc sym in either fileset, find the one in the
  // library or real fileset.
  ProcedureSymbol *find_library_file_proc(ProcedureSymbol *ps);
  ProcedureSymbol *find_real_file_proc(ProcedureSymbol *ps);

  ProcedureDefinition *get_proc_block_from_ps(ProcedureSymbol *ps);
  ProcedureDefinition *get_proc_block(ProcedureSymbol *ps);

  VariableSymbol *find_real_var_sym(VariableSymbol *sym);
  VariableSymbol *find_library_var_sym(VariableSymbol *sym);


  bool is_symtab_in_real_fileset(SymbolTable *st);
  bool is_symtab_in_library_filesets(SymbolTable *st);

  bool is_sym_in_real_fileset(Symbol *sym);
  bool is_sym_in_library_filesets(Symbol *sym);
  
  bool sym_has_no_real_sym(Symbol *sym);
 private:

  bool is_symtab_in_fileset(SymbolTable *st,
			    FileSetBlock *tgt_fileset);
};

#endif /* FILESET_UTIL_H */
