#ifndef CFGRAPH_VIS_H
#define CFGRAPH_VIS_H

/* 
 * ******************************************************
 * *
 * * class cfgraph_build_walker
 * *
 * * Walk through the program representation and
 * * apply the visitor
 * *
 * ******************************************************
 */
//#include <suif.h>
//#include <rinds.h>
//#include "suif_cfgraph.h"
//#include <sgraph_region.h>
//#include <slist_tos.h>

//class cfgraph_build_walker;

//typedef cfsgraph_region<CFGraphNode *,einfo> suif_cfgraph_region;

//typedef sgraph_region suif_cfgraph_region;





#include "cfgraph_state.h"


#include "suifkernel/walking_maps.h"
#include "cfgraph_module.h"

//extern void cfgraph_pass_init_suif_maps(WalkingMaps *maps);
//extern void cfgraph_pass_init_suif_maps(WalkingMaps *maps);
extern void cfgraph_pass_init_suif_maps(SuifCFGraphBuilderModule *);


// Eventually this will be a number static functions
// 

// class cfgraph_build_visitor /* : public suif_visitor */{
//   CFGraphAnnote *_the_cfg;

//   //  cfgraph_build_walker *_the_walker; // just referenced.
//   //  slist_tos<VariableSymbol *> _vardef_stack;  //

//   cfgraph_build_visitor(const cfgraph_build_visitor &ea) {};
//   bool _pre_node;

//   slist_tos<postpone_label_node *> _postponed_edges;
//   CFGraphNode *_prev_entry;
//   CFGraphNode *_prev_exit;

// public:
//   cfgraph_build_visitor(// cfgraph_build_walker *the_walker,
// 			CFGraphAnnote *the_cfg);
// 			//	ecr_computation *ecr_comp,
// 			//	ecr_annotation_manager *ecr_annotation);
  
//   void set_pre_node() { _pre_node = true; }
//   void set_post_node() { _pre_node = false; }
//   bool is_pre_node() const { return _pre_node; }
//   bool is_post_node() const { return !_pre_node; }

//   void save_current_entry_exit(CFGraphNode *entry, CFGraphNode *exit) {
//     _prev_entry = entry; _prev_exit = exit;
//   }
//   CFGraphNode *prev_entry() const { return _prev_entry; }
//   CFGraphNode *prev_exit() const { return _prev_exit; }

//   void connect_edge(CFGraphNode *from_node, CFGraphNode *to_node);

//   void connect_edge_to_label(CFGraphNode *from_node, CodeLabelSymbol *to_label);
//   void connect_edge_from_label(CodeLabelSymbol *from_label, CFGraphNode *to_node);

//   void connect_edge_if_empty(CFGraphNode *from_node, CFGraphNode *to_node);
//   suif_cfgraph_region *get_top_region() const { 
//     return _the_cfg->get_top_region(); }

//   // Create a new node with the name on the Statement
//   CFGraphNode *create_label_node(Statement *the_statement,
// 				  LString name,
// 				  CodeLabelSymbol *label);

//   void register_label_node(CodeLabelSymbol *label,
// 			   CFGraphNode *node);
//   CFGraphNode *create_test_node(Statement *the_statement,
// 				 SourceOp test) {
//     return(_the_cfg->create_test_node(the_statement, test));
//   }


//   VariableSymbol *create_variable(String name,
// 				  AnnotableObject *obj,
// 				  Type *the_type);
//   /*
//   slist_tos<CFGraphNode *> _prev_stack;  // Add an edge from this
//                                           // to entry
//   slist_tos<CFGraphNode *> _entry_stack;        // callee pushes this
//   slist_tos<CFGraphNode *> _fall_through_stack; // callee pushed this
//                                           // usually becomes prev
// 					  */
  
//   void handle_procedure_definition(ProcedureDefinition *the_procdef) ;
//   //  void handle_procedure_symbol(ProcedureSymbol *the_procsym) ;

//   // statements
//   void handle_statement(Statement *st) ;
//   void handle_eval_statement(EvalStatement *the_eval) ;
//   void handle_for_statement(ForStatement *the_for) ;
//   void handle_if_statement(IfStatement *the_if) ;
//   void handle_while_statement(WhileStatement *the_while) ;
//   void handle_do_while_statement(DoWhileStatement *the_while) ;
//   void handle_va_start_statement(VaStartStatement *the_vastart) ;
//   void handle_va_start_old_statement(VaStartOldStatement *the_vastart) ;
//   void handle_va_end_statement(VaEndStatement *the_vaend) ;
//   void handle_store_statement(StoreStatement *the_store) ;
//   void handle_scope_statement(ScopeStatement *the_scope) ;

//   void handle_return_statement(ReturnStatement *the_ret) ; 
//   void handle_jump_statement(JumpStatement *the_jump) ; 
//   // Any label with address taken in this procedure.
//   void handle_jump_indirect_statement(JumpIndirectStatement *the_jump) ; 
//   void handle_branch_statement(BranchStatement *the_branch) ; 
//   void handle_multi_way_branch_statement(MultiWayBranchStatement *the_mwb); 
//   void handle_label_location_statement(LabelLocationStatement *the_label);
//   void handle_assert_statement(AssertStatement *the_assert);
//   void handle_mark_statement(MarkStatement *the_mark);

//   // CFG nodes
//   /*
//   void handle_jump_cfgn(jump_cfgn *the_jump);
//   void handle_indirect_jump_cfgn(jump_indirect_cfgn *the_jump);
//   void handle_branch_cfgn(branch_cfgn *the_branch);
//   void handle_multi_way_branch_cfgn(multi_way_branch_cfgn *the_branch);
//   void handle_no_exit_cfgn(no_exit_cfgn *the_noexit);
//   */

//   // CFOs
//   //  void handle_cfo(cfo *the_cfo);
//   //  void handle_cfg(cfg *the_cfg);
//   //  void handle_expression_list(expression_list *the_expression_list);
//   void handle_statement_list(StatementList *the_statement_list);
  
  
//   // expressions
//   void handle_expression(Expression *the_instr) ;

//   // expressions with control flow
//   void handle_call_expression(CallExpression *cal) ;
//   void handle_sc_and_expression(ScAndExpression *the_and) ;
//   void handle_sc_or_expression(ScOrExpression *the_or) ;
//   void handle_sc_select_expression(ScOrExpression *the_select) ;

//   // branching
//   //  void handle_return_expression(ReturnStatement *the_ret) ; 
//   //  void handle_jump_instruction(jump_instruction *the_jump) ; 
//   //  void handle_jump_indirect_instruction(jump_indirect_instruction *the_jump) ; 
//   //  void handle_branch_instruction(branch_instruction *the_branch) ; 
//   //  void handle_multi_way_branch_instruction(multi_way_branch_instruction *the_mwb); 
//   //  void handle_label_location_instruction(label_location_instruction *the_label);
//   //  void handle_assert_instruction(assert_instruction *the_assert);
  
//   bool is_operand_constant(SourceOp op);
//   // Only valid is is_operand_constant() is true.
//   bool is_operand_true(SourceOp op);
  
// };

/*
class cfgraph_build_walker : public zot_walker {
private:
  //  ecr_computation *_ecr_comp;   // This class owns all of the ecrs
  // and must clean them up
  cfgraph_build_visitor *_the_ecr_visitor;

  cfgraph_build_walker(const cfgraph_build_visitor &ea) {};

public:
  cfgraph_build_walker(CFGraphAnnote *the_cfg) {
    _the_ecr_visitor = new cfgraph_build_visitor(this, the_cfg);
  }

  void node_preorder(zot *the_zot) {
    _the_ecr_visitor->set_pre_node();
    _the_ecr_visitor->apply(the_zot);
  }

  void node_postorder(zot *the_zot) {
    _the_ecr_visitor->set_post_node();
    _the_ecr_visitor->apply(the_zot);
  }
};

*/
//void enter_cfgraph_vis(int *argc, char *argv[]);
//void exit_cfgraph_vis();

#endif /* CFGRAPH_VIS_H */
