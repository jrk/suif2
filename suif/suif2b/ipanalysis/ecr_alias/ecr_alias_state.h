#ifndef ECR_ALIAS_STATE
#define ECR_ALIAS_STATE

/* 
 * ******************************************************
 * *
 * * class ecr_alias_state
 * *
 * * This class contains the state used to
 * * compute the ecr classes
 * *
 * ******************************************************
 */

#include "ecr_alias_forwarders.h"

#include "suifnodes/suif_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include "common/suif_list.h"
#include "ecr_type.h"

#include "suifkernel/walking_maps.h"

class ecr_annotation_manager;

class EcrAliasState;

class EcrAliasState { 
private:
  SuifEnv *_suif_env;
  ecr_computation *_ecr_comp;   // This class owns all of the ecrs
  ecr_owner *_ecr_owner;   // This class owns all of the ecrs
  ecr_annotation_manager *_ecr_annotation;   // This class owns all of the ecrs
  WalkingMaps *_maps;
  // and must clean them up
  list<ProcedureSymbol *> _reachable_procedures;
  //
  //list<CallExpression *> _call_sites;
  list<CallStatement *> _call_sites;
  //  list<ExecutionObject *> _allocation_sites;
  FileSetMgr *_fileset_mgr;
  bool _silent;
public:
  EcrAliasState(SuifEnv *suif_env,
		ecr_computation *ecr_comp, 
		ecr_owner *ecr_owner,
		ecr_annotation_manager *ecr_annotation,
		FileSetMgr *fileset_mgr, bool silent=false);
  // Must set the maps later
  ~EcrAliasState();
  SuifEnv *get_suif_env() const;
  ecr_computation *get_ecr_comp() const;
  ecr_owner *get_ecr_owner() const;
  ecr_annotation_manager *get_ecr_annotation() const;
  WalkingMaps *get_maps() const;
  void set_maps(WalkingMaps *);
  
  FileSetMgr *get_fileset_mgr() const;

  bool is_silent(){return _silent;}

  //  void set_real_fileset(FileSetBlock *fse);
  //  void add_library_fileset(FileSetBlock *fse);

  // This can only be invoked on a fileset.
  void process_the_fileset(FileSetBlock *fse);
  void process_a_symbol_table(SymbolTable *symtab);
  void process_a_procedure_definition(ProcedureDefinition *procdef);
  void process_a_variable_definition(VariableDefinition *vardef);

  // a procedure is reachable if it's addres has been
  // taken in the code that has been processed.
  list<ProcedureSymbol *> *get_reachable_procedures() const;
  list<CallStatement *> *get_call_sites() const;
  void add_reachable_procedure(ProcedureSymbol* ps);
  void add_call_site(CallStatement* cal);

  // ---------------------------------------

  //
  // these methods are called from the registered walking
  // routines.
  //
  // -----------------------------------------------------

  // -----------------------------------------------------


  void process_expression_dests(Expression *i);
  void process_statement_dests(Statement *the_statement);
  void process_symbol_address_expression(SymbolAddressExpression *lda,
					Symbol *sym);
  void process_load_procedure_address_expression(SymbolAddressExpression *lda,
						  ProcedureSymbol *ps);
  void process_load_symbol_address_expression(SymbolAddressExpression *the_load, 
					       Symbol *sym);
  void process_procedure_address_taken(ecr_node *ex, 
				       ProcedureSymbol *ps);
  void process_variable_address_taken(ecr_node *ex, 
				      VariableSymbol *ps);


  void process_load(ecr_node *ex, ecr_node *ey) ; // x = *y
  void process_alloc(CallStatement *cal) ;
  void process_store(ecr_node *ex, ecr_node *ey) ; // *x = y
  void strip_tmp_ecr_info(SuifObject *the_azot) ;
  void process_assign(ecr_node *ex, ecr_node *ey) ; // x = y
  void process_cjoin_x(ecr_node *ex,
		       ecr_node *ey_tau, ecr_node *ey_lambda);  // x = y
  void process_cjoin_y(ecr_node *ex_tau, ecr_node *ex_lambda, 
		       ecr_node *ey);  // x = y
  void process_cjoin(ecr_node *ex_tau, ecr_node *ex_lambda, 
		     ecr_node *ey_tau, ecr_node *ey_lambda);  // x = y

  bool proc_is_allocator(ProcedureSymbol *ps) const ;
    
  ecr_node *find_ecr_variable(VariableSymbol *var);


  lambda_type *meet_procedure_varargs(ProcedureSymbol *ps);
  void extend_lambda_inputs(lambda_type *l, unsigned i);
  void extend_lambda_outputs(lambda_type *l, unsigned i);
  void report_lambda_varargs_use(lambda_type *l, unsigned i);


};





#endif /* ECR_ALIAS_STATE */
