#ifndef _SUIFLINK__SUIFLINKER_H_
#define _SUIFLINK__SUIFLINKER_H_

/**
  * A SuifLinker konws how to link two FileSetBlocks.
  *
  * A SuifLinker is created with a FileSetBlock object,
  * which is considered the master FileSetBlock.
  * The method link(FileSetBlock *slave) will merge information
  *   from the slave to the master.
  * At the end of link(), the FileBlocks from slave will appear in
  *   the master FileSetBlock and all the symbols will be merged
  *   according to ANSI C rules.
  */

#include "suifnodes/suif.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/message_buffer.h"
#include "replace_map.h"

extern LString k_linksuif_temp_filename;

/** This class object links file set blocks according to ANSI rules.
  * This object is created with a file set block, which is considered the master.
  * Each subsequent call to link(FileSetBlock*) will link in another file set
  * block to the master.
  * Each call to link() may add error messages to the internal error buffer.
  * Each SuifLinker also has a buffer for garbage.  Grabage object can be
  * added with add_garbage().  The method delete_garbage() will delete all
  * objects in the garbage buffer.
  */
class SuifLinker {
 public:

  /** Constructor.
    * @param master the master file set block, other file set block will be linked
    *               into this one.
    */
  SuifLinker(SuifEnv *se, FileSetBlock *master, String filename);

  /** Link one file set block.
    * @param slave the FileSetBlock to be linked.
    * @return number of errors found.
    */
  int add(FileSetBlock *slave, String filename);

  /** Merge all FileBlocks and resolve all symbols and type objects.
    * @return number of errors found.
    */
  int link(void);

  /** Retrieve the error message.
    * This method should be called after linking all file set blocks.
    */
  String get_error_message(void);

  /** Get the number of errors accumulated.
    */
  int get_error_count(void);

  /** Add to the garbage can.
    * @param obj the object to be descarded.
    */
  void add_garbage(SuifObject* obj);

  /** Delete all garbage in the garbage can.
    */
  void delete_garbage(void);

  void set_verbose(bool verbose);

 private:
  FileSetBlock * _master_fsb;
  ReplaceMap _type_map;
  AndMessageBuffer _errors;
  suif_vector<SuifObject*> _garbage;
  SuifEnv * _suif_env;

  bool _link_verbose;

  void add_error(const String& msg);

  Type* merge_type(Type* oldtype, ReplaceMap *map, SymbolTable *);
  QualifiedType* merge_qualified(QualifiedType *oldtype,
				 ReplaceMap *map,
				 SymbolTable *symtab);
  PointerType *merge_pointer(PointerType *oldtype,
			     ReplaceMap *typemap,
			     SymbolTable *symtab);
  IntConstant *merge_bound(Expression* bounds,
			   ReplaceMap *typemap,
			   SymbolTable *symtab);
  ArrayType *merge_array(ArrayType *oldtype,
			 ReplaceMap *typemap,
			 SymbolTable *symtab);
  CProcedureType *merge_cprocedure(CProcedureType *oldtype,
				   ReplaceMap *typemap,
				   SymbolTable *symtab);
  GroupType* merge_group(GroupType *oldtype,
			 ReplaceMap *typemap,
			 SymbolTable *symtab);
  GroupType* merge_group_fields(GroupType *oldtype,
			 ReplaceMap *typemap,
			 SymbolTable *symtab);
  EnumeratedType* merge_enumerated(EnumeratedType *oldtype,
				   ReplaceMap *typemap,
				   SymbolTable *symtab);
  Type* merge_subtype(Type *oldtype,
		      ReplaceMap *typemap,
		      SymbolTable *symtab);
  void merge_fsb(FileSetBlock *slave);
  void merge_symtab(SymbolTable *master,
		    SymbolTable *slave,
		    ReplaceMap *typemap);

  /** Phase 2, resolve all declarations/definitions in the external symbol
    * table.
    */
  void resolve_decl(void);

  /** For all symbols that are going to be replaced, remove them from their
    * parent symbol table and delete them (put into garbage can).
    */
  void cleanup_symtab(ReplaceMap*);
};

#endif /* _SUIFLINK__SUIFLINKER_H_ */
