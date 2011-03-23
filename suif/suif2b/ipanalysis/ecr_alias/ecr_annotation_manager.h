#ifndef ECR_ANNOTATION_MANAGER
#define ECR_ANNOTATION_MANAGER

#include "ecr_type.h"
#include "suifnodes/suif_forwarders.h"
#include "basicnodes/basic_forwarders.h"
/*
 * We define 4 classes here:
 * 1. the generic annotation that we will use.
 *    Note that they can not be read in, only out.
 * 2. The annotation manager that
 *    keeps the mapping of objects to their ecr nodes.
 *    An ecr node WILL automatically be created
 *    when queried.  It will also be automatically
 *    updated when queried
 * 3 & 4 a visitor and walker to strip the temporary
 *    annotations on instructions and value blocks.
 */


#include "ecr_alias_forwarders.h"

#include "ecrnodes/ecr_constants.h"

extern LString k_ecr_tmp_statement_result;
extern LString k_ecr_tmp_value_block_result;
extern LString k_ecr_tmp_instruction_result;
extern LString k_ecr_tmp_variable_definition_result;

class ecr_annotation_manager {

  // return is NON-NULL
  ecr_node *create_ecr_for_obj(AnnotableObject *obj,
			       const LString & k_tmp);
#if 0
  ecr_node *create_empty_ecr_by_num(const LString & k_tmp, 
				    AnnotableObject *the_azot,
				    unsigned num);
  //  Annote *create_empty_annote(const LString & k_tmp, 
  //			      AnnotableObject *the_azot,
  //			      unsigned num);

#endif

  ecr_node *retrieve_named_ecr(const LString & k_tmp,
			       AnnotableObject *obj);

#if 0
  ecr_node *get_named_ecr_by_num(const LString & k_tmp,
				 AnnotableObject *the_azot, 
				 unsigned num);
#endif
public:
  // Here are the rest of the functions;
  ecr_annotation_manager(SuifEnv *suif_env, ecr_computation *ecr_comp,
			 FileSetMgr *fileset_mgr);

  // These are the fake nodes that are built.
  // After any find operation, the ecr is the TOP node.

  // For safety, we would like a 
  // FIND (gets it. creates a new one if needed)
  // SET sets it and updates it)
  // 


  
  void setup_procedure(ProcedureSymbol *ps);

  // put the ecr that points to the lambda on the node
  ecr_node *find_ecr_procedure(ProcedureSymbol *ps);

  // For normal instructions where multiple results just
  // copy the original single result
  ecr_node *find_ecr_expression_result(Expression *i);

  ecr_node *find_ecr_variable_definition_result(VariableDefinition *v);

  //ecr_node *find_ecr_statement_multi_result(Statement *i, unsigned opn);
  ecr_node *find_ecr_statement_result(Statement *i);
  ecr_node *find_ecr_value_block_result(ValueBlock *valblock);

 
  ecr_node *find_ecr_variable(VariableSymbol *var);

  // Could be a CallExpression or a CallStatement
  ecr_node *find_ecr_call_target(ExecutionObject *cal);
  ecr_node *find_ecr_allocation(ExecutionObject *cal);
  ecr_node *find_ecr_store_to(StoreStatement *the_store); 
  ecr_node *find_ecr_load_from(LoadExpression *the_load);

  // shorthand for getting the ecr from the Expression or variable
  ecr_node *find_ecr_from_dst(Statement *st, unsigned opn);

  void clear_map(const LString &name);

  typedef suif_hash_map<AnnotableObject*,ecr_node *> ecr_map;
  typedef suif_hash_map<LString, ecr_map*> full_ecr_map;


  ecr_map *retrieve_ecr_map(const LString &k_tmp);
  ecr_map *get_ecr_map(const LString &k_tmp) const;

  // Place all ecr annotations for non-tmp objects
  void place_annotations(FileSetBlock *fsb);

private:
  ecr_node *retrieve_ecr(ecr_map *map,
			 AnnotableObject *obj);
  
  // Need this to create a new node.
  SuifEnv *_suif_env;
  ecr_computation *_ecr_comp;
  FileSetMgr *_fileset_mgr;
  full_ecr_map *_map;
};


//void add_attribute_annote(AnnotableObject *the_azot, const LString & k_tmp);
//bool is_attribute_annote(AnnotableObject *the_azot, const LString & k_tmp);


extern "C" void enter_ecr_annotation_manager(int *argc, char *argv[]);
extern "C" void exit_ecr_annotation_manager();

#endif /* ECR_ANNOTATION_MANAGER */
