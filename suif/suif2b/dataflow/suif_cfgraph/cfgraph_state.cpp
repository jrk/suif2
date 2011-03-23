#include "cfgraph_state.h"

#include "common/suif_hash_map.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"
#include "suif_cfgraph.h"
#include "suif_cfgraph_factory.h"
#include "utils/symbol_utils.h"
#include "utils/cloning_utils.h"
#include "super_graph/super_graph.h"

static size_t hash( const CodeLabelSymbol *a ) {
  size_t i = (long)a;
  return (i >> 2) + (i >> 10);
}


static DataType *to_data_type(Type *t) {
  if (is_kind_of<QualifiedType>(t)) {
    return(to_data_type(to<QualifiedType>(t)->get_base_type()));
  }
  return(to<DataType>(t));
}

PostponeLabelNode::PostponeLabelNode(CodeLabelSymbol *label,
				     CFGraphNode *node) :
  _is_label_from(true),
  _label(label),
  _node(node) 
{}

PostponeLabelNode::PostponeLabelNode(CFGraphNode *node,
				     CodeLabelSymbol *label) :
  _is_label_from(false),
  _label(label),
  _node(node) 
{}


bool PostponeLabelNode::is_label_from() const { return(_is_label_from); }
CodeLabelSymbol *PostponeLabelNode::from_label() const {
  assert(is_label_from()); return _label; }
CodeLabelSymbol *PostponeLabelNode::to_label() const {
  assert(!is_label_from()); return _label; }

CFGraphNode *PostponeLabelNode::from_node() const {
  assert(!is_label_from()); return _node; }
CFGraphNode *PostponeLabelNode::to_node() const {
  assert(is_label_from()); return _node; }


WalkingMaps *CFGraphState::get_maps() const { return _maps; }
SuifEnv *CFGraphState::get_suif_env() const { return _suif_env; }
void CFGraphState::set_maps(WalkingMaps *maps) { _maps = maps; }

//void CFGraphState::save_current_entry_exit(CFGraphNode *entry, CFGraphNode *exit) {
//  _prev_entry = entry; _prev_exit = exit;
//}
//void CFGraphState::clear_entry_exit() {
//  _prev_entry = NULL; _prev_exit = NULL;
//}
//CFGraphNode *CFGraphState::prev_entry() const { return _prev_entry; }
//CFGraphNode *CFGraphState::prev_exit() const { return _prev_exit; }

SGraph *CFGraphState::get_graph() const { return _cfg; }

/*
suif_cfgraph_region *CFGraphState::get_top_region() const { 
  return _the_cfg->get_top_region(); }
*/


CFGraphNode *CFGraphState::get_entry_node(AnnotableObject *obj,
					  bool is_executable) {
  if (is_node(obj, "entry")) return(find_node(obj, "entry"));
  return(add_node(obj, "entry", is_executable));
}
CFGraphNode *CFGraphState::get_exit_node(AnnotableObject *obj) {
  if (is_node(obj, "exit")) return(find_node(obj, "exit"));
  return(add_node(obj, "exit", false));

}

CFGraphNode *CFGraphState::get_proc_call_node(Statement *obj) {
  LString node_type = "call";
  if (is_node(obj, node_type)) return(find_node(obj, node_type));
  return(add_node(obj, node_type, true));
}
CFGraphNode *CFGraphState::get_proc_return_node(Statement *obj) {
  LString node_type = "return";
  if (is_node(obj, node_type)) return(find_node(obj, node_type));
  return(add_node(obj, node_type, false));
}

CFGraphNode *CFGraphState::get_cfgraph_entry_node(AnnotableObject *obj) {
  return(get_entry_node(obj, false));
}
CFGraphNode *CFGraphState::get_cfgraph_exit_node(AnnotableObject *obj) {
  return(get_exit_node(obj));
}


bool CFGraphState::is_node(AnnotableObject *obj, const LString &name) const {
  return(find_node(obj, name) != NULL);
}

CFGraphNode *CFGraphState::find_node(AnnotableObject *obj, const LString &name) const 
{
  GraphGlue glue(obj, name, NULL);
  for (suif_vector<GraphGlue *>::iterator iter = _glue_table->begin();
       iter != _glue_table->end(); iter++) {
    if (glue == (*(*iter))) { return((*iter)->get_node()); }
  }
  return(NULL);
}

// bool is_executable
CFGraphNode *CFGraphState::internal_add_node(AnnotableObject *obj, 
					     const LString &name,
					     bool is_executable,
					     bool is_one_to_one,
					     Statement *statement) {
  assert(!is_node(obj, name));

  unsigned node_id = _cfg->max_num_nodes();
  _cfg->add_node(node_id);
  CFGraphNode *new_node = create_c_f_graph_node(get_suif_env(),
						name, 
						is_executable,
						is_one_to_one,
						obj,
						node_id,
						statement);
  _cfg_annote->append_node(new_node);

  GraphGlue *glue = new GraphGlue(obj, name, new_node);
  _glue_table->push_back(glue);
  return(new_node);
}

CFGraphNode *CFGraphState::add_node_num(unsigned node_id,
					AnnotableObject *obj, 
					const LString &name,
					bool is_executable,
					bool is_one_to_one,
					Statement *statement) {
  assert(!is_node(obj, name));

  //  unsigned node_id = _cfg->max_num_nodes();
  //  _cfg->add_node(node_id);
  CFGraphNode *new_node = create_c_f_graph_node(get_suif_env(),
						name, 
						is_executable,
						is_one_to_one,
						obj,
						node_id,
						statement);
  //  _cfg_annote->remove_node(node_id);
  //  _cfg_annote->insert_node(node_id, new_node);
  // @@ this is a bad dependency.  I
  // know that the entry and exit node will be 0 and 1
  // and entered in that order.  If that changes,
  // we'll have to fix this.
  suif_assert((int)node_id == (int)_cfg_annote->get_node_count());
  _cfg_annote->append_node(new_node);
  return(new_node);
}
    
CFGraphNode *CFGraphState::add_node(AnnotableObject *obj, 
				    const LString &name,
				    bool is_executable
				    ) 
{
  return(internal_add_node(obj, name, is_executable, false, NULL));
}
CFGraphNode *CFGraphState::add_statement_node(AnnotableObject *obj, 
					      const LString &name,
					      bool is_executable,
					      bool is_one_to_one,
					      Statement *st) 
{
  return(internal_add_node(obj, name, is_executable, is_one_to_one, st));
}
CFGraphNode *CFGraphState::add_expression_node(AnnotableObject *obj, 
					     const LString &name,
					     bool is_one_to_one,
					     Expression *op) 
{
  SuifEnv *s = get_suif_env();
  // maybe a branch here instead..
  EvalStatement *eval = create_eval_statement(s);
  eval->append_expression(op);
  return(internal_add_node(obj, name, true, is_one_to_one, eval));
}

void CFGraphState::connect_edge(CFGraphNode *from_node, 
				CFGraphNode *to_node) {
  if (from_node == NULL || to_node == NULL) return;
  _cfg->add_edge(SGraphEdge(from_node->get_id(), to_node->get_id()));
}

CFGraphBuilderReturn CFGraphState::connect_entry_exit(CFGraphNode *from_node, 
				      const CFGraphBuilderReturn &body,
				      CFGraphNode *to_node) {
  connect_edge(from_node, body.get_entry());
  connect_edge(body.get_exit(), to_node);
  if (body.get_entry() == NULL
      && body.get_exit() == NULL) 
    {
      connect_edge(from_node, to_node);
    }
    
  connect_edge_if_empty(from_node, to_node);
  return(CFGraphBuilderReturn(from_node, to_node));
}


VariableSymbol *CFGraphState::
create_variable(const String &name, AnnotableObject *obj, 
		QualifiedType *the_type) {
  SymbolTable *st = find_scope(obj);
  VariableSymbol *var = create_variable_symbol(obj->get_suif_env(), 
					       the_type, name, false);
  st->add_symbol(var);
  //  st->addobj->scope()->add_sto(new VariableSymbol(name, the_type));
  return(var);
}

CFGraphNode *CFGraphState::
get_node_from_label(CodeLabelSymbol *label) const {
  label_node_map::iterator iter = _label_map->find(label);
  if (iter != _label_map->end()) return (*iter).second;
  return(NULL);
}

LabelLocationStatement *CFGraphState::
get_location_from_label(CodeLabelSymbol *label) const {
  label_loc_map::iterator iter = _label_location_map->find(label);
  if (iter != _label_location_map->end()) return (*iter).second;
  return(NULL);
}


void CFGraphState::
connect_edge_to_label(CFGraphNode *from_node, CodeLabelSymbol *to_label) {
  if (from_node == NULL || to_label == NULL) return;
  CFGraphNode *to_node = get_node_from_label(to_label);
  if (to_node == NULL) {
    _postponed_edges->push_front(new PostponeLabelNode(from_node, to_label));
  } else {
    connect_edge(from_node, to_node);
  }
}

void CFGraphState::
connect_edge_from_label(CodeLabelSymbol *from_label, CFGraphNode *to_node) {
  if (from_label == NULL || to_node == NULL) return;
  CFGraphNode *from_node = get_node_from_label(from_label);
  if (from_node == NULL) {
    _postponed_edges->push_front(new PostponeLabelNode(from_label, to_node));
  } else {
    connect_edge(to_node, from_node);
  }
}


void CFGraphState::
connect_edge_if_empty(CFGraphNode *from_node, CFGraphNode *to_node) {
  if (from_node == NULL || to_node == NULL) return;
  if (from_node == to_node) return;
  if (!_cfg->node_has_successors(from_node->get_id())) {
    connect_edge(from_node, to_node);
  }
}

CFGraphBuilderReturn CFGraphState::
build(ExecutionObject *obj) {
  return(_builder->build(this, obj));
}

/*
CFGraphNode *CFGraphState::
create_label_node(Statement *the_statement,
		  LString name,
		  CodeLabelSymbol *label) {
  CFGraphNode *label_node = 
    _cfg->create_label_node_by_name(the_statement,
					name, label);
  register_label_node(label, label_node);
  return(label_node);
}
*/

CFGraphNode *CFGraphState::create_test_node(Statement *the_statement,
					    bool is_one_to_one,
					    Expression *test) {
  
  CFGraphNode *test_node = 
    add_expression_node(test, "test_node", is_one_to_one, 
			deep_suif_clone(test));
    //    add_sourceop_node(the_statement, "test_node", is_one_to_one, 
  return(test_node);
}

CFGraphNode *CFGraphState::
create_label_node(Statement *the_statement,
		  const LString &name,
		  CodeLabelSymbol *label) {
  CFGraphNode *label_node = 
    add_node(the_statement, name, false);
  register_label_node(label, label_node);
  return(label_node);
}

void CFGraphState::
register_label_node(CodeLabelSymbol *label,
		    CFGraphNode *node) {
  _label_map->enter_value(label, node);
}

void CFGraphState::
register_label_location(CodeLabelSymbol *label,
			LabelLocationStatement *location) {
  _label_location_map->enter_value(label, location);
}

void CFGraphState::finish_annotes() {
  // Copy the sgraph to the annote
  for (suif_vector<CFGraphNode *>::iterator it =
	 _cfgnodes->begin(); it != _cfgnodes->end(); it++) {
    CFGraphNode *node = *it;
    SGraphNode n = node->get_id();
    
    while (n >= (SGraphNode)_cfg_annote->get_node_count()) {
      _cfg_annote->append_successor(NULL);
    }
    if (_cfg_annote->get_node(n) == NULL) {
      _cfg_annote->replace_node(n, node);
    }
    suif_assert_message(_cfg_annote->get_node(n) == node,
			("Error in annotation building"));
  }

  for (SEdgeIter iter = _cfg->get_edge_iterator();
       iter.is_valid(); iter.next()) {
    SGraphEdge e = iter.current();
    SGraphNode from = e.from();
    SGraphNode to = e.to();

    //CFGraphNode *from_node = _cfg_annote->get_node(from);
    CFGraphNode *to_node = _cfg_annote->get_node(to);
    
    while (from >= (SGraphNode)_cfg_annote->get_successor_count()) {
      _cfg_annote->append_successor(NULL);
    }
    CFGraphNodeList *succ_list = _cfg_annote->get_successor(from);
    if (succ_list == NULL) {
      succ_list = create_c_f_graph_node_list(get_suif_env());
      _cfg_annote->replace_successor(from, succ_list);
    }
    succ_list->append_node(to_node);
  }
}


void CFGraphState::fix_up_labels() {
  while (!_postponed_edges->empty()) {
    PostponeLabelNode *node = _postponed_edges->front();
    _postponed_edges->pop_front();
    if (node->is_label_from()) {
      CodeLabelSymbol *from_label = node->from_label();

      /*
      label_node_map::iterator fiter = _label_map->find(from_label);
      assert(fiter != _label_map->end());
      CFGraphNode *from_node = (*fiter).second;
      */
      CFGraphNode *from_node = get_node_from_label(from_label);

      CFGraphNode *to_node = node->to_node();
      connect_edge(from_node, to_node);
    } else {
      CodeLabelSymbol *to_label = node->to_label();

      /*
      label_node_map::iterator fiter = _label_map->find(to_label);
      assert(fiter != _label_map->end());
      CFGraphNode *to_node = (*fiter).second;
      */
      CFGraphNode *to_node = get_node_from_label(to_label);

      suif_assert_message(to_node != NULL,
			  ("Postponed label %s not found",
			   to_label->get_name().c_str()));
      CFGraphNode *from_node = node->from_node();
      connect_edge(from_node, to_node);
    }
  }

  while (!_indirect_jumps->empty()) {
    CFGraphNode *node = _indirect_jumps->front();
    _indirect_jumps->pop_front();

    for (list<CodeLabelSymbol*>::iterator iter = 
	   _address_taken_labels->begin();
	 iter != _address_taken_labels->end(); iter++) {
      CodeLabelSymbol *sym = (*iter);
      CFGraphNode *to_node = get_node_from_label(sym);
      suif_assert(to_node);
      connect_edge(node, to_node);
    }
  }
}

void CFGraphState::add_pending_indirect_jump(CFGraphNode *node) {
  for (list<CFGraphNode*>::iterator iter = 
	 _indirect_jumps->begin(); iter != _indirect_jumps->end();
       iter++) {
    if ((*iter) == node) return;
  }
  _indirect_jumps->push_back(node);
}

void CFGraphState::add_address_taken_label(CodeLabelSymbol *sym) {
  for (list<CodeLabelSymbol *>::iterator iter = 
	 _address_taken_labels->begin(); 
       iter != _address_taken_labels->end();
       iter++) {
    if ((*iter) == sym) return;
  }
  _address_taken_labels->push_back(sym);
}

/*
void CFGraphState::set_working_procedure_definition(ProcedureDefinition *proc_def) {
  clear_working_procedure_definition();
  CFGraphAnnote *an = create_c_f_graph_annote(get_suif_env(),
					      "cfg_proc_annote",
					      proc_def, 0, 0);
  _cfg = new SGraphList();
  proc_def->append_annote(an);
  
  _cfgnodes = new suif_vector<CFGraphNode *>;
  _entries = new SGraphNodeList();
  _exits = new SGraphNodeList();
  
  //_super_graph = new SuperGraph;
  
  _cfg_annote = an;
  _cfg_annote->set_cfg(_cfg);
  _postponed_edges = new list<PostponeLabelNode *>;
  _label_map = new label_node_map;
  _label_location_map = new label_loc_map;
  _address_taken_labels  = new list<CodeLabelSymbol *>;
  _indirect_jumps = new list<CFGraphNode *>;
  _glue_table = new suif_vector<GraphGlue *>;

  // Now get the entry and exit nodes
  CFGraphNode *the_entry = get_cfgraph_entry_node(proc_def);
  CFGraphNode *the_exit = get_cfgraph_exit_node(proc_def);
  _cfg_annote->set_entry_node(the_entry->get_id());
  _cfg_annote->set_exit_node(the_exit->get_id());
}
*/
CFGraphState::~CFGraphState() 
{
  delete _cfg;
  _cfg_annote = 0;
  delete _postponed_edges;
  _postponed_edges = 0;
  delete _label_map; 
  _label_map = 0;
  delete _label_location_map;
  _label_location_map = 0;
  delete _address_taken_labels;
  _address_taken_labels = 0;
  delete _indirect_jumps;
  _indirect_jumps = 0;
  delete _glue_table;
  _glue_table = 0;
}

/*
void CFGraphState::clear_working_procedure_definition() {
  //  delete _cfg;
  _cfg = 0;
  _cfg_annote = 0;
  delete _postponed_edges;
  _postponed_edges = 0;
  delete _label_map; 
  _label_map = 0;
  delete _label_location_map;
  _label_location_map = 0;
  delete _address_taken_labels;
  _address_taken_labels = 0;
  delete _indirect_jumps;
  _indirect_jumps = 0;
  delete _glue_table;
  _glue_table = 0;
}
*/

CFGraphState::CFGraphState(SuifEnv *suif_env, 
			   SuifCFGraphBuilderModule *builder,
			   ProcedureDefinition *proc_def) :
  _builder(builder),
  _cfg(new SGraphList()),
  _cfgnodes(new suif_vector<CFGraphNode*>),
  _entries(new SGraphNodeList),
  _exits(new SGraphNodeList),
  //  _super_graph(0),
  _cfg_annote(create_c_f_graph_annote(suif_env,
				      "cfg_proc_annote",
				      proc_def, 0, 0)),
  _postponed_edges(new list<PostponeLabelNode*>),
  //  _prev_entry(0),
  //  _prev_exit(0),
  _label_map(new label_node_map),
  _label_location_map(new label_loc_map),
  _address_taken_labels(new list<CodeLabelSymbol *>),
  _indirect_jumps(new list<CFGraphNode *>),
  _glue_table(new suif_vector<GraphGlue *>),
  //_maps(),
  _suif_env(suif_env)
{
  suif_assert_message(proc_def->peek_annote("cfg_proc_annote") == NULL,
		      ("Attempt to rebuild CFGraphState on a procedure"));
  
  proc_def->append_annote(_cfg_annote);
  //  _cfg_annote->set_cfg(_cfg);

  // Now get the entry and exit nodes
  CFGraphNode *the_entry = get_cfgraph_entry_node(proc_def);
  CFGraphNode *the_exit = get_cfgraph_exit_node(proc_def);
  _cfg_annote->set_entry_node(the_entry->get_id());
  _cfg_annote->set_exit_node(the_exit->get_id());
}

//
// Also, any branch to a label can be added if the label exists.
//     otherwise, it is stored into a table and deferred.
/*
 *
 * The basic pattern here:
 *  Build the nodes and edges for
 *  the suif node. 
 * Call register_label for any label statements
 * Call register_branch for any branches
 * Call add_edge for any edges to add
 *  set the (default_entry, default_exit) before returning.
 * Return the entry node and fall-through exit node for the handled statement
 */


GraphGlue::GraphGlue(SuifObject *obj, const LString &name, CFGraphNode *node) :
  _obj(obj), _name(name), _node(node) {}
GraphGlue::GraphGlue(const GraphGlue &other) :
    _obj(other._obj), _name(other._name), _node(other._node) {}
GraphGlue &GraphGlue::operator=(const GraphGlue &other) {
  _obj = other._obj;
  _name = other._name;
  _node = other._node;
  return(*this);
}
bool GraphGlue::operator==(const GraphGlue &other) const{
  if (_obj != other._obj) return false;
  if (_name != other._name) return false;
  if (_node == 0 || other._node == 0) return true;
  return(_node == other._node);
}
void GraphGlue::set_node(CFGraphNode *node) { _node = node; }
CFGraphNode *GraphGlue::get_node() const { return( _node ); }
