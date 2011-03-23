#ifndef CFGRAPH_STATE_H
#define CFGRAPH_STATE_H

#include "cfgraph_forwarders.h"

#include "suifkernel/suifkernel_forwarders.h"
#include "suif_cfgraph_forwarders.h"

#include "common/common_forwarders.h"
#include "common/suif_list.h"
#include "suifnodes/suif_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifkernel/walking_maps.h"
#include "sgraph/sgraph.h"
#include "super_graph/super_graph.h"
#include "cfgraph_module.h"


class GraphGlue {
  SuifObject *_obj;
  LString _name;
  //cached
  CFGraphNode *_node;
public:
  GraphGlue(SuifObject *obj, const LString &name, CFGraphNode *node);
  GraphGlue(const GraphGlue &other);
  GraphGlue &operator=(const GraphGlue &other);
  bool operator==(const GraphGlue &other) const;
  void set_node(CFGraphNode *node);
  CFGraphNode *get_node() const;
};

// This little class encapsulates a label
// It is needed for patch-up work.
class PostponeLabelNode {
  bool _is_label_from;  // true: label->node,
  // false: node->label
  CodeLabelSymbol *_label;
  CFGraphNode *_node;
public:
  PostponeLabelNode(CodeLabelSymbol *label,
		    CFGraphNode *node);
  PostponeLabelNode(CFGraphNode *node,
		    CodeLabelSymbol *label);
  bool is_label_from() const;
  CodeLabelSymbol *from_label() const;
  CodeLabelSymbol *to_label() const;
  CFGraphNode *from_node() const;
  CFGraphNode *to_node() const;
};


// A cfg is an ngraph<cfgraphnode*>
// that builds a super_graph on it.

class CFGraphState {
  SuifCFGraphBuilderModule *_builder;
  SGraph *_cfg;                      // graph of the node numbers.
  suif_vector<CFGraphNode *> *_cfgnodes;
  SGraphNodeList *_entries;
  SGraphNodeList *_exits;

  //  SuperGraph *_super_graph;          // the region graph.

  //  cfgraph *_cfg;
  CFGraphAnnote *_cfg_annote;

  list<PostponeLabelNode *> *_postponed_edges;
  //  CFGraphNode *_prev_entry;
  //  CFGraphNode *_prev_exit;
  typedef suif_hash_map<CodeLabelSymbol *, CFGraphNode *> label_node_map;
  typedef suif_hash_map<CodeLabelSymbol *, LabelLocationStatement *> label_loc_map;
  label_node_map *_label_map;
  label_loc_map *_label_location_map;

  list<CodeLabelSymbol *> *_address_taken_labels;
  list<CFGraphNode *> *_indirect_jumps;

  suif_vector<GraphGlue *> *_glue_table;

  WalkingMaps *_maps;
  SuifEnv *_suif_env;

public:
  CFGraphState(SuifEnv *, SuifCFGraphBuilderModule *builder,
	       ProcedureDefinition *pd);
  ~CFGraphState();
  SuifCFGraphBuilderModule *get_builder() const { return _builder; }

  //void set_working_procedure_definition(ProcedureDefinition *proc_def);
  //  void clear_working_procedure_definition();

  WalkingMaps *get_maps() const;
  void set_maps(WalkingMaps *map);
  SuifEnv *get_suif_env() const;
  SGraph *get_graph() const;  // for queries by number and 
  // additions to the graph.

  CFGraphNode *get_node(unsigned num) const;

  unsigned get_node_id(const CFGraphNode *node) const; // translation

  bool is_node(AnnotableObject *obj, const LString &tag) const;
  CFGraphNode *find_node(AnnotableObject *obj, const LString &tag) const;

  // find it or add it if needed.
  CFGraphNode *add_node(AnnotableObject *obj, const LString &tag,
			bool is_executable
			);

  // add a node with some expression.
  CFGraphNode *add_expression_node(AnnotableObject *obj, 
				   const LString &tag, 
				   bool is_one_to_one,
				   Expression *op);

  // Add a node that owns a statement
  CFGraphNode *add_statement_node(AnnotableObject *obj, 
				  const LString &tag, 
				  bool is_executable,
				  bool is_one_to_one,
				  Statement *);

  void add_pending_indirect_jump(CFGraphNode *st);
  void add_address_taken_label(CodeLabelSymbol *st);
  //  void map_label_to_location(CodeLabelSymbol *,
  //			     LabelLocationStatement *st);

  // Here are the helper functions
  //  void save_current_entry_exit(CFGraphNode *entry, CFGraphNode *exit);
  //  void clear_entry_exit();

  //  CFGraphNode *prev_entry() const;
  //  CFGraphNode *prev_exit() const;

  // Connect edges.
  void connect_edge(CFGraphNode *from_node, CFGraphNode *to_node);
  CFGraphBuilderReturn connect_entry_exit(CFGraphNode *from_node, 
			  const CFGraphBuilderReturn &bd,
			  CFGraphNode *to_node);
  void connect_edge_to_label(CFGraphNode *from_node,
			     CodeLabelSymbol *to_label);
  void connect_edge_from_label(CodeLabelSymbol *from_label,
			       CFGraphNode *to_node);

  void connect_edge_if_empty(CFGraphNode *from_node, CFGraphNode *to_node);

  //  suif_cfgraph_region *get_top_region() const;

  // Call this to create a new label node for the
  // statement.
  CFGraphNode *create_label_node(Statement *the_statement,
				 const LString &name,
				 CodeLabelSymbol *label);

  CFGraphNode *create_test_node(Statement *the_statement,
				bool is_one_to_one,
				Expression *test);

  // return NULL if not there.
  CFGraphNode *get_node_from_label(CodeLabelSymbol *label) const;
  LabelLocationStatement *get_location_from_label(CodeLabelSymbol *label) const;

  VariableSymbol *create_variable(const String &name,
				  AnnotableObject *obj,
				  QualifiedType *the_type);

  // These really belong int their own subsystems.
  // 
  bool is_operand_constant(Expression *op) const;
  bool is_operand_true(Expression *op) const;

  // read the entry and exit nodes
  // Same as get_node(SuifObject *, "entry") or "exit"
  CFGraphNode *get_entry_node(AnnotableObject *, bool is_executable);
  CFGraphNode *get_exit_node(AnnotableObject *);
  CFGraphNode *get_cfgraph_entry_node(AnnotableObject *);
  CFGraphNode *get_cfgraph_exit_node(AnnotableObject *);

  CFGraphBuilderReturn build(ExecutionObject *obj);


  CFGraphNode *get_proc_call_node(Statement *cal);
  CFGraphNode *get_proc_return_node(Statement *cal);

  void fix_up_labels();

  // Call this to register the label node with the label.
  void register_label_node(CodeLabelSymbol *label,
			   CFGraphNode *node);
  void register_label_location(CodeLabelSymbol *label,
			       LabelLocationStatement *location);

  //  void build_dominance_frontier();
  void finish_annotes();
 
private:
  CFGraphNode *internal_add_node(AnnotableObject *obj, 
				 const LString &name,
				 bool is_executable,
				 bool is_one_to_one,
				 Statement *statement);
  CFGraphNode *add_node_num(unsigned node_id,
			    AnnotableObject *obj, 
			    const LString &name,
			    bool is_executable,
			    bool is_one_to_one,
			    Statement *statement);
  
};





#endif
