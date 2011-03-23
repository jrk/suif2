#include "lib_fileset_util.h"


// for the SymbolXrefAnnote
#include "iputils.h"
#include "iputils_factory.h"
#include "utils/symbol_utils.h"
#include "utils/type_utils.h"

#include <fstream.h>
#include "iokernel/binary_streams.h"
#include "iokernel/object_factory.h"
#include "iokernel/synchronizer.h"
#include "iokernel/pointer_meta_class.h"
#include "suifkernel/suif_env.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"

static LString k_lib_sym = "lib_sym";
static LString k_real_sym = "real_sym";
static LString k_no_library_fn = "no_library_function";

static bool assert_on_error = false;

static bool is_attribute_annote(AnnotableObject *obj,
				const LString &kind) {
  return(obj->peek_annote(kind)!= NULL);
}

static void add_attribute_annote(AnnotableObject *obj,
				 const LString &kind) {
  if (is_attribute_annote(obj, kind)) return;
  obj->append_annote(create_brick_annote(obj->get_suif_env(),
					 kind));
}

SymbolXrefAnnote *get_xref_annote(AnnotableObject *obj, const LString &kind) {
  return(to<SymbolXrefAnnote>(obj->peek_annote(kind)));
}


static ProcedureSymbol *lookup_proc_in_fileset(const char *name,
				 FileSetBlock *tgt_fileset);
static VariableSymbol *lookup_var_in_fileset(const char *name,
				 FileSetBlock *tgt_fileset);
static void set_sym_correlate(Symbol *real_sym, Symbol *lib_sym);
static void set_proc_sym_correlate(ProcedureSymbol *real_ps, ProcedureSymbol *lib_ps, bool verbose);

FileSetMgr::FileSetMgr() :
  _real_fileset(0),
  _lib_filesets(new list<FileSetBlock*>()),
  _suppress_var_sym_warning(false),
  _verbose(false)
{}

FileSetBlock *FileSetMgr::get_real_fileset() const {
  return(_real_fileset);
}
const list<FileSetBlock *> *FileSetMgr::get_lib_filesets() const {
  return(_lib_filesets);
}
void FileSetMgr::set_real_fileset(FileSetBlock *fsb)
{
  suif_assert(_real_fileset == 0 || fsb == 0);
  _real_fileset = fsb;
}

void FileSetMgr::add_library_fileset(FileSetBlock *fsb)
{
  _lib_filesets->push_back(fsb);
}

bool FileSetMgr::set_suppress_var_sym_warning(bool val) {
  bool old_val = _suppress_var_sym_warning;
  _suppress_var_sym_warning = val;
  return(old_val);
}

bool FileSetMgr::sym_has_no_real_sym(Symbol *sym) {
  if (is_sym_in_real_fileset(sym)) {
    return(false);
  }
  return (!is_attribute_annote(sym, k_real_sym));
}



static ProcedureSymbol *lookup_proc_in_fileset(const char *name,
					FileSetBlock *tgt_fileset) {
  ProcedureSymbol *ps = NULL;
  // find the FileSetBlock

  if (tgt_fileset == NULL) return NULL;
  for (int i = 0; i < tgt_fileset->get_file_block_count(); i++) {
    FileBlock *fse = tgt_fileset->get_file_block(i);
    SymbolTable *fst = fse->get_symbol_table();
    // do NOT check the global table automatically.
    ProcedureSymbol *new_ps = lookup_proc_locally(fst, name);
    if (new_ps != NULL) {
      suif_assert_message(ps == NULL,
		 ("The proc sym %s is multiply defined", name));
      ps = new_ps;
    }
  }

  // look it up in the global table.
  ProcedureSymbol *new_ps =
    lookup_proc_locally(tgt_fileset->get_external_symbol_table(), name);
  if (new_ps != NULL) {
    suif_assert_message(ps == NULL,
	       ("The proc sym %s is multiply defined", name));
    ps = new_ps;
  }

  new_ps =
    lookup_proc_locally(tgt_fileset->get_file_set_symbol_table(), name);
  if (new_ps != NULL) {
    suif_assert_message(ps == NULL,
	       ("The proc sym %s is multiply defined", name));
    ps = new_ps;
  }

  return(ps);
}

static VariableSymbol *lookup_var_in_fileset(const char *name,
					FileSetBlock *tgt_fileset) {
  VariableSymbol *var = NULL;
  // find the FileSetBlock
  for (int i = 0; i < tgt_fileset->get_file_block_count(); i++) {
    FileBlock *fse = tgt_fileset->get_file_block(i);
    SymbolTable *fst = fse->get_symbol_table();
    // do NOT check the global table automatically.
    VariableSymbol *new_var = lookup_var_locally(fst, name);

    if (new_var != NULL) {
      suif_assert_message(var == NULL,
		 ("The var sym %s is multiply defined", name));
      var = new_var;
    }
  }

  // look it up in the global table.
  VariableSymbol *new_var =
    lookup_var_locally(tgt_fileset->get_external_symbol_table(),
			name);
  if (new_var != NULL) {
    suif_assert_message(var == NULL,
	       ("The var sym %s is multiply defined", name));
    var = new_var;
  }

  // look it up in the global table.
  new_var =
    lookup_var_locally(tgt_fileset->get_file_set_symbol_table(),
			name);
  if (new_var != NULL) {
    suif_assert_message(var == NULL,
	       ("The var sym %s is multiply defined", name));
    var = new_var;
  }

  return(var);
}


ProcedureSymbol *FileSetMgr::find_real_file_proc(ProcedureSymbol *ps) {
  if (is_sym_in_real_fileset(ps)) {
    return(ps);
  }
  suif_assert(is_sym_in_library_filesets(ps));
  ProcedureSymbol *lib_ps = ps;


  SymbolXrefAnnote *an = get_xref_annote(lib_ps, k_real_sym);
  if (an != NULL) {
    return(to<ProcedureSymbol>(an->get_xsymbol()));
  }
  suif_assert(get_xref_annote(lib_ps, k_lib_sym) == NULL);

  // Now we have to add the correspondence
  //
  ProcedureSymbol *real_ps = lookup_proc_in_fileset(lib_ps->get_name().c_str(),
						    _real_fileset);
  if (real_ps == NULL) {
    fprintf(stderr, "%s: Could not find proc %s in REAL fileset\n",
	    assert_on_error ? "ERROR" : "WARNING",
	    real_ps->get_name().c_str());
  }
  if (lib_ps == NULL) {
    if (real_ps == NULL ||
	!is_attribute_annote(real_ps, k_no_library_fn)) {
    if (!_suppress_var_sym_warning) {
      fprintf(stderr, "%s: Could not find proc %s in LIBRARY fileset\n",
	      assert_on_error ? "ERROR" : "WARNING",
	      real_ps->get_name().c_str());
    }
      if (real_ps != NULL) {
	add_attribute_annote(real_ps, k_no_library_fn);
      }
    }
  }
  if (lib_ps != NULL &&
      real_ps != NULL) {
    set_proc_sym_correlate(real_ps, lib_ps, _verbose);
  } else {
    suif_assert(!assert_on_error);
  }
  return(real_ps);
}
// externally visible.
ProcedureSymbol *FileSetMgr::find_library_file_proc(ProcedureSymbol *ps) {
  if (is_sym_in_library_filesets(ps)) {
    return(ps);
  }
  suif_assert(is_sym_in_real_fileset(ps));
  ProcedureSymbol *real_ps = ps;

  SymbolXrefAnnote *an = get_xref_annote(real_ps, k_lib_sym);
  if (an != NULL) {
    return(to<ProcedureSymbol>(an->get_xsymbol()));
  }
  suif_assert(get_xref_annote(real_ps, k_real_sym) == NULL);
  //  assert(real_ps->annotes()->peek_annote(k_real_sym) == NULL);

  // Now we have to add the correspondence
  ProcedureSymbol *lib_ps = 0;
  for (list<FileSetBlock*>::iterator iter =
	 _lib_filesets->begin();
       iter != _lib_filesets->end(); iter++) {
    lib_ps = lookup_proc_in_fileset(real_ps->get_name().c_str(),
				    *iter);
    if (lib_ps != 0) break;
  }

  if (real_ps == NULL) {
    fprintf(stderr, "%s: Could not find proc %s in REAL fileset\n",
	      assert_on_error ? "ERROR" : "WARNING",
	    real_ps->get_name().c_str());
  }
  if (lib_ps == NULL) {
    if (real_ps == NULL ||
	!is_attribute_annote(real_ps, k_no_library_fn)) {
    if (!_suppress_var_sym_warning) {
      fprintf(stderr, "%s: Could not find proc %s in LIBRARY fileset\n",
	      assert_on_error ? "ERROR" : "WARNING",
	      real_ps->get_name().c_str());
    }
      if (real_ps != NULL) {
	add_attribute_annote(real_ps, k_no_library_fn);
      }
    }
  }
  if (lib_ps != NULL &&
      real_ps != NULL) {
    set_proc_sym_correlate(real_ps, lib_ps, _verbose);
  } else {
    suif_assert(!assert_on_error);
  }
  return(lib_ps);
}


VariableSymbol *FileSetMgr::find_real_var_sym(VariableSymbol *var) {
  if (is_sym_in_real_fileset(var)) {
    return(var);
  }
  suif_assert(is_sym_in_library_filesets(var));
  VariableSymbol *lib_var = var;

  SymbolXrefAnnote *an = get_xref_annote(lib_var, k_real_sym);
  if (an != NULL) {
    return(to<VariableSymbol>(an->get_xsymbol()));
  }
  suif_assert(get_xref_annote(lib_var, k_real_sym) == NULL);

  // Now we have to add the correspondence
  VariableSymbol *real_var = lookup_var_in_fileset(lib_var->get_name().c_str(),
				   _real_fileset);
  if (real_var == NULL) {
    if (!_suppress_var_sym_warning) {
      fprintf(stderr, "%s: Could not find var %s in REAL fileset\n",
	      assert_on_error ? "ERROR" : "WARNING",
	      var->get_name().c_str());
    }
  }
  if (lib_var == NULL) {
    if (real_var == NULL ||
	!is_attribute_annote(real_var, k_no_library_fn)) {
      if (!_suppress_var_sym_warning) {
	fprintf(stderr, "%s: Could not find var %s in LIBRARY fileset\n",
		assert_on_error ? "ERROR" : "WARNING",
		real_var->get_name().c_str());
      }
      if (real_var != NULL) {
	add_attribute_annote(real_var, k_no_library_fn);
      }
    }
  }
  if (lib_var != NULL &&
      real_var != NULL) {
    set_sym_correlate(real_var, lib_var);
  } else {
    suif_assert(!assert_on_error);
  }
  return(real_var);
}

// externally visible.
VariableSymbol *FileSetMgr::find_library_var_sym(VariableSymbol *var) {
  if (is_sym_in_library_filesets(var)) {
    return(var);
  }
  suif_assert(is_sym_in_real_fileset(var));
  VariableSymbol *real_var = var;

  SymbolXrefAnnote *an = get_xref_annote(real_var, k_lib_sym);
  if (an != NULL) {
    return(to<VariableSymbol>(an->get_xsymbol()));
  }
  suif_assert(get_xref_annote(real_var, k_real_sym) == NULL);

  // Now we have to add the correspondence
  VariableSymbol *lib_var = 0;
  for (list<FileSetBlock*>::iterator iter =
	 _lib_filesets->begin();
       iter != _lib_filesets->end(); iter++) {
    lib_var = lookup_var_in_fileset(real_var->get_name().c_str(),
				    *iter);
    if (lib_var != 0) break;
  }

  if (real_var == NULL) {
    if (!_suppress_var_sym_warning) {
      fprintf(stderr, "%s: Could not find var %s in REAL fileset\n",
	      assert_on_error ? "ERROR" : "WARNING",
	      var->get_name().c_str());
    }
  }
  if (lib_var == NULL) {
    if (real_var == NULL ||
	!is_attribute_annote(real_var, k_no_library_fn)) {
      if (!_suppress_var_sym_warning) {
	fprintf(stderr, "%s: Could not find var %s in LIBRARY fileset\n",
		assert_on_error ? "ERROR" : "WARNING",
		var->get_name().c_str());
      }
      if (real_var != NULL) {
	add_attribute_annote(real_var, k_no_library_fn);
      }
    }
  }
  if (lib_var != NULL &&
      real_var != NULL) {
    set_sym_correlate(real_var, lib_var);
  } else {
    suif_assert(!assert_on_error);
  }
  return(lib_var);
}


bool FileSetMgr::is_sym_in_real_fileset(Symbol *sym) {
  if (sym == NULL) return false;
  SymbolTable *st = sym->get_symbol_table();
  return(is_symtab_in_real_fileset(st));
}

bool FileSetMgr::is_sym_in_library_filesets(Symbol *sym) {
  if (sym == NULL) return false;
  SymbolTable *st = sym->get_symbol_table();
  return(is_symtab_in_library_filesets(st));
}
bool FileSetMgr::is_symtab_in_library_filesets(SymbolTable *symtab) {
  if (symtab == 0) return false;
  for (list<FileSetBlock*>::iterator iter =
	 _lib_filesets->begin();
       iter != _lib_filesets->end(); iter++) {
    if (is_symtab_in_fileset(symtab, *iter)) return true;
  }
  return false;
}

bool FileSetMgr::is_symtab_in_real_fileset(SymbolTable *symtab) {
  return(is_symtab_in_fileset(symtab, _real_fileset));
}

/*
bool FileSetMgr::is_sym_in_fileset(Symbol *sym,
			  FileSetBlock *tgt_fileset) {
  if (sym == NULL) return false;
  SymbolTable *st = sym->get_symbol_table();
  return(is_symtab_in_fileset(st, tgt_fileset));
}
*/

bool FileSetMgr::is_symtab_in_fileset(SymbolTable *st,
				      FileSetBlock *tgt_fileset) {
  if (tgt_fileset == NULL) return(false);
  if (st == NULL) return(false);
  SymbolTable *par = find_super_scope(st);
  while (par != NULL) {
    st = par;
    par = find_super_scope(st);
  }
  return(st == tgt_fileset->get_external_symbol_table());
}


static void proc_sym_err(ProcedureSymbol *real_ps, ProcedureSymbol *lib_ps) {

  fprintf(stderr,
	  "-----: proc %s: real and library have incompatible types\n",
	  real_ps->get_name().c_str());
  fprintf(stderr, "   REAL : ");
  //  real_type->print_full(stderr);
  fprintf(stderr, "   LIB  : ");
  //  lib_type->print_full(stderr);
}


static void set_proc_sym_correlate(ProcedureSymbol *real_ps, ProcedureSymbol *lib_ps, bool verbose) {
  //ProcedureType *real_type = unqualify_procedure_type(real_ps->get_type());
  //ProcedureType *lib_type = unqualify_procedure_type(lib_ps->get_type());

  bool are_isomorphic = false;
  {
    //    isomorphic_type_check tcheck(false);
    //    are_isomorphic = tcheck.isomorphic(real_type, lib_type);
    // @@@ need my iomorphic type check!!
    are_isomorphic = true;
  }
  if (!are_isomorphic) {
    proc_sym_err(real_ps, lib_ps);
    //    fprintf(stderr, "ERROR: types are not compatible: real(");
    //    real_type->print_abbrev(stderr);
    //    fprintf(stderr, ") vs. lib(");
    //    lib_type->print_abbrev(stderr);
    //    fprintf(stderr, ")\n");
    if (verbose) {
      //      full_type_print tprint(false);
      fprintf(stderr, "REAL_TYPE:\n");
      //      tprint.print(stderr, real_type);
      fprintf(stderr, "LIB_TYPE:\n");
      //      tprint.print(stderr, lib_type);
    }
    suif_assert(!assert_on_error);
  }


  /* Check for varargs... */

  /*
  if (!real_type->compatible(lib_type)) {
    // just here for debugging.
    bool val = real_type->compatible(lib_type);

    if ((real_type->return_type()->op() == TYPE_VOID)
	!= (lib_type->return_type()->op() == TYPE_VOID)) {
      fprintf(stderr, "ERROR: VOID vs non-VOID return type\n");
      proc_sym_err(real_ps, lib_ps);
    }
    if (real_type->has_varargs()
	!= lib_type->has_varargs()) {
      fprintf(stderr, "ERROR: varargs vs not-varargs\n");
      proc_sym_err(real_ps, lib_ps);
    }
    if (real_type->args_known()
	!= lib_type->args_known()) {
      fprintf(stderr, "ERROR: known vs unknown args\n");
      proc_sym_err(real_ps, lib_ps);
    }  else {
      if (real_type->num_args() !=
	  lib_type->num_args()) {
	fprintf(stderr, "ERROR: real_args=%d vs lib_args=%d\n",
		real_type->num_args(),
		lib_type->num_args());
	proc_sym_err(real_ps, lib_ps);
      }
    }
    if (!real_type->return_type()->compatible(lib_type->return_type())) {
      fprintf(stderr, "       incompatible return type\n");
      proc_sym_err(real_ps, lib_ps);
    }
    unsigned min_args = u_min(lib_type->num_args(),
			      real_type->num_args());
    unsigned i;
    for (i = 0; i < min_args; i++) {
      if (!real_type->arg_type(i)->compatible(lib_type->arg_type(i))) {
	fprintf(stderr, "       incompatible arg=%d\n", i);
	proc_sym_err(real_ps, lib_ps);
      }
    }
  }
  */
  set_sym_correlate(real_ps, lib_ps);
}

static void set_sym_correlate(Symbol *real_sym, Symbol *lib_sym) {
  suif_assert(get_xref_annote(lib_sym, k_real_sym) == NULL);
  suif_assert(get_xref_annote(real_sym, k_lib_sym) == NULL);
  //  assert(lib_sym->annotes()->peek_annote(k_real_sym) == NULL);
  //  assert(real_sym->annotes()->peek_annote(k_lib_sym) == NULL);

  SuifEnv *s = real_sym->get_suif_env();
  SymbolXrefAnnote *lib_an =
    create_symbol_xref_annote(s, k_real_sym, real_sym);
  SymbolXrefAnnote *real_an =
    create_symbol_xref_annote(s, k_lib_sym, lib_sym);

  lib_sym->append_annote(lib_an);
  real_sym->append_annote(real_an);
}

/* The ProcedureSymbol MUST belong to the real_file_set */
/* The block will belong to the library FileSetBlock */

ProcedureDefinition *FileSetMgr::get_proc_block_from_ps(ProcedureSymbol *ps) {
  ProcedureDefinition *procdef = ps->get_definition();
  //  suif_assert(procdef != NULL);
  return(procdef);
}

// NULL return means that
// EITHER: the procedure has no body
//         or it is an allocator
//
ProcedureDefinition *FileSetMgr::get_proc_block(ProcedureSymbol *real_ps) {

  suif_assert(is_sym_in_real_fileset(real_ps));

  ProcedureDefinition *tp = get_proc_block_from_ps(real_ps);
  if (tp == NULL) {
    ProcedureSymbol *lib_ps = find_library_file_proc(real_ps);
    /*
    suif_assert_message(lib_ps != NULL,
	       ("Procedure %s not in library fileset",real_ps->get_name().c_str()));
	       */
    if (lib_ps == NULL) return(NULL);

    ProcedureDefinition *tp = get_proc_block_from_ps(lib_ps);
    if (tp == NULL) return(NULL);
    /*
    suif_assert_message(lib_ps != NULL,
	       ("Procedure body for %s not available from library fileset",
		real_ps->get_name().c_str()));
		*/
    return(tp);
  }
  //suif_assert_message(!proc_is_allocator(ps),
  //("ERROR: allocation procedure %s in real fileset", ps->get_name().c_str()));

  return(tp);
}



FileSetBlock *FileSetMgr::add_library_file(SuifEnv *s,
					   const String &inputFileName) {

  FileSetBlock* old_file_set_block = s->get_file_set_block();
  s->set_file_set_block(0);
  s->read(inputFileName);
  FileSetBlock* lib_file_set_block = s->get_file_set_block();
  s->set_file_set_block(0);
  s->set_file_set_block(old_file_set_block);
  add_library_fileset(lib_file_set_block);
  return lib_file_set_block;
}