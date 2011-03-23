/* file "ecr_alias_pass.cc" */


/*
       Copyright (c) 1998,1999 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


/*
      This is the main implementation file of the ecr_alias_pass,
      a library for interprocedural alias analysis
*/


#include "ecr_alias_pass.h"
#include "common/lstring.h"
#include "suifpasses/suifpasses.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/print_subsystem.h"
#include "suifkernel/walking_maps.h"
#include "ecr_computation.h"
#include "ecr_annotation_manager.h"
#include "ecr_alias_state.h"
#include "ecr_alias_vis.h"
#include "iputils/lib_fileset_util.h"

#include "ecrnodes/ecr.h"
#include "common/suif_hash_map.h"
#include "suifkernel/utilities.h"
#include "bit_vector/bit_vector.h"
#include "utils/symbol_utils.h"
#include "suifkernel/command_line_parsing.h"
#include "basicnodes/basic_constants.h"
#include "utils/expression_utils.h"
#include "utils/type_utils.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "iputils/iputils.h"
#include "iputils/iputils_factory.h"

//#include "ecr_build_annote.h"

/*
 * external symbols we may or may not need
 */
bool alias_verbose = false;
bool alias_print_bot = false;
/*
 * ****************
 * *
 * * extra functionality required for printing
 * *
 * ****************
 */
template<>
void union_find_node<ecr_type *>::print(ion *out) const
  {
    if (!this->get_data()->is_bottom()
	|| alias_print_bot) {
      out->printf("node=%u:\t",this->get_id());
      // How do I dispatch this print method in the template?
      this->get_data()->print(out);
      out->put_s("\n");
    }
  }
bool ecr_progress = false;
bool ecr_verbose = false;


static size_t hash(ProcedureSymbol *a) {
  size_t i = (long)a;
  return (i >> 2) + (i >> 10);
}



class EcrAliasPass : public Pass {
private:
  WalkingMaps *_maps;

  //  OptionString *_lib_file;
  //  OptionList *_file_option;
  //  String _lib_file_name;

  OptionLiteral *_only_proc_ptrs; // cjoin joins on function pointer only.
  OptionLiteral *_do_all_procs;
  OptionLiteral *_do_by_ref;
  OptionLiteral *_only_proc_pointers;
  OptionLiteral *_verbose_output;
  OptionLiteral *_silent;
  String _libname;

  suif_hash_map<ProcedureSymbol *,unsigned int> *_procnums;
  suif_vector<ProcedureSymbol *> *_procset;
  BitVector *_reachable_procs;

  typedef void (*walker_init_fn)(WalkingMaps *);

  list<walker_init_fn> *_walker_inits;

  EcrAliasState *_state;
public:
  EcrAliasPass(SuifEnv *suif_env, const LString &name = "ecr_alias");
  ~EcrAliasPass();
  void initialize();
  void do_file_set_block(FileSetBlock *file_set_block );
  void finalize();

  //  void do_place_annotations(FileSetBlock *fsb) {
  //    fprintf(stderr, "do_place_annotations\n");
  //  }
  void do_generate_call_graph(FileSetBlock *fsb);
  //  void do_statistics(FileSetBlock *fsb) {
  //    fprintf(stderr, "do_statistics\n");
  //  }

  Module *clone() const;

  // anytime we see an interface object created,
  // store the interface (in this case a function pointer )
  virtual void interface_object_created(Module *producer,
				   const LString &interface_name);


};





EcrAliasPass::EcrAliasPass(SuifEnv *suif_env, const LString &name) :
  Pass(suif_env, name), _maps(0),
  _procnums(new suif_hash_map<ProcedureSymbol *, unsigned int>),
  _procset(new suif_vector<ProcedureSymbol *>),
  _reachable_procs(new BitVector),
  _walker_inits(new list<walker_init_fn>),
  _state(0)
{
  set_interface("ecr_alias_walker",
		(Address)ecr_alias_pass_init_suif_maps);
}

EcrAliasPass::~EcrAliasPass(){
	delete _walker_inits;
}

void EcrAliasPass::interface_object_created(Module *producer,
					    const LString &interface_name) {
  Address addr = producer->get_interface( interface_name );
  // Here's the scary part.  Convert it.
  if (addr == 0) return;
  walker_init_fn fn = (walker_init_fn) addr;
  _walker_inits->push_back(fn);
}

void
EcrAliasPass::initialize() {
  Module::initialize();
  _command_line -> set_description(
             "ecr_alias_pass builds a steensgaard alias analysis from a fileset and libraries" );
  _do_all_procs = new OptionLiteral("-do-all-procs");
  _do_by_ref = new OptionLiteral("-do-by-ref");
  _only_proc_pointers = new OptionLiteral("-only-proc-pointers");
  _verbose_output = new OptionLiteral("-v");
  _silent = new OptionLiteral("-silent");
  Option *libname =
    build_prefixed_string( "-lib", "library", &_libname, "External library model" );

  //  _libfile = new
  OptionSelection *sel = new OptionSelection();
  sel->add(_do_all_procs)->add(_do_by_ref)->add(_verbose_output)->
      add(libname)->add(_only_proc_pointers)->add(_silent);
  //  _file = new OptionString("file-name",&_file_name );
  //  OptionLoop *opt = new OptionLoop(_do_all_procs, true);
  OptionLoop *opt = new OptionLoop(sel, true);
  _command_line->add(opt);

  ModuleSubSystem *ms = get_suif_env()->get_module_subsystem();

  // register the EcrAliasPass as a listener and producer for the
  // ecr_alias_walker
  ms->register_interface_listener( this, "ecr_alias_walker");
  ms->register_interface_producer( this, "ecr_alias_walker");
}


lambda_type *get_lambda_type(ExecutionObject *cal,
			     EcrAliasState *state) {
  ecr_annotation_manager *mgr = state->get_ecr_annotation();
  ecr_computation *comp = state->get_ecr_comp();

  ecr_node *node = mgr->find_ecr_call_target(cal);
  ecr_node *lambda_node = comp->get_lambda_pointed_to(node);
  lambda_type *l = lambda_node->get_data()->get_lambda_type();
  return(l);
}


unsigned find_num_call_targets(ExecutionObject *cal,
			       EcrAliasState *state) {
  lambda_type *l = get_lambda_type(cal, state);
  return(l->num_targets());
}

bool is_allocation_site(ExecutionObject *cal,
			EcrAliasState *state) {
  lambda_type *l = get_lambda_type(cal, state);
  return (l->is_alloc());
}

static double safe_pct(double a, double b) {
  if (b == 0.0) return 0.0;
  return(a/b * 100.0);
}

void EcrAliasPass::do_file_set_block(FileSetBlock *file_set_block ) {
  SuifEnv *s = get_suif_env();

  ecr_owner *ecr_own = new ecr_owner();
  //FileSetMgr *fileset_mgr = new FileSetMgr(0, new list<FileSetBlock*>(), _silent->is_set());

  FileSetMgr *fileset_mgr = new FileSetMgr();
  fileset_mgr->set_suppress_var_sym_warning(true/*_silent->is_set()*/);

  ecr_computation *ecr_comp = new ecr_computation(ecr_own,
						  _only_proc_pointers->is_set());

  ecr_annotation_manager *ecr_annotation =
    new ecr_annotation_manager(s, ecr_comp, fileset_mgr);
  _state = new EcrAliasState(s,
			     ecr_comp,
			     ecr_own,
			     ecr_annotation,
			     fileset_mgr, _silent->is_set());
  WalkingMaps *maps = new WalkingMaps(s, "ecr_alias_pass_walker",
				      (Address) _state);
  _state->set_maps(maps);
  _maps = maps;

  {
  // Now do the deferred initialization
  for (list<walker_init_fn>::iterator iter =
	 _walker_inits->begin();
       iter != _walker_inits->end();
       iter++) {
    walker_init_fn fn = (*iter);
    // This is an indirect function call through
    // something that had bee untyped.
    // I'd REALLY like to be able to validate it
    // before using it.
    (*fn)(maps);
  }
  }


  // Now back to our regularly scheduled stuff.

  _state->get_fileset_mgr()->set_real_fileset(file_set_block);
  if (!(emptyString == _libname)) {
    fprintf(stderr, "Using library: %s\n", _libname.c_str());
    //    FileSetBlock *fsb =
    _state->get_fileset_mgr()->add_library_file(s, _libname);
  }

  //  BuildCStringState *cstate = new BuildCStringState(s);


  // We should also read in the library filesets here


  // First do all the symbol tables.
  // (And perhaps the ones in the libraries
  _state->process_a_symbol_table(file_set_block->get_external_symbol_table());
  _state->process_a_symbol_table(file_set_block->get_file_set_symbol_table());

  // Also keep track of all of the procedures
  // Now find all of the symbol tables in the fileblocks
  // and the definition blocks

  // Also keep track of all of the procedures
  //  suif_hash_map<ProcedureSymbol *,unsigned int> procnums;
  list<ProcedureSymbol *> to_visit;  // entry procedures
  //  vector<ProcedureSymbol*> procset;

  for (Iter<FileBlock*> iter = file_set_block->get_file_block_iterator();
	iter.is_valid();
	iter.next()) {
    FileBlock *fb = iter.current();
    _state->process_a_symbol_table(fb->get_symbol_table());
    {
    for (Iter<VariableDefinition*> iter =
	   fb->get_definition_block()->get_variable_definition_iterator();
	 iter.is_valid();
	 iter.next()) {
      VariableDefinition *vardef = iter.current();
      _state->process_a_variable_definition(vardef);
    }
    }
    for (Iter<ProcedureDefinition*> iter =
	   fb->get_definition_block()->get_procedure_definition_iterator();
	 iter.is_valid();
	 iter.next()) {
      ProcedureDefinition *procdef = iter.current();
      ProcedureSymbol *ps = procdef->get_procedure_symbol();
      unsigned id = _procset->size();
      _procset->push_back(ps);
      _procnums->enter_value(ps, id);
      if (is_kind_of<FileSetBlock>(ps->get_symbol_table()->get_parent())) {
      //      if (is_external_symbol_table(ps->get_symbol_table())) {
	// If all of the procedures are here...
	if (String("main") == ps->get_name()) {
	  to_visit.push_back(ps);
	}
      }
    }
  }

  // Now we have all of the procedure definitions
  //

  if (_do_all_procs->is_set()) {
    // If we are doing all of the procedures:
    for (suif_vector<ProcedureSymbol*>::iterator iter =
	   _procset->begin();
	 iter != _procset->end();
	 iter++) {
      ProcedureSymbol *ps = (*iter);
      ProcedureDefinition *procdef = ps->get_definition();
      _state->process_a_procedure_definition(procdef);
      if (!ps->peek_annote("reachable_proc")) {
	ps->append_annote(create_general_annote(get_suif_env(),
						"reachable_proc"));
      }
    }
    if(!_silent->is_set()){
      fprintf(stderr, "Procs : %d. All reachable\n",
	    _procset->size());
    }
    for (unsigned i =0; i < _procset->size(); i++) {
      _reachable_procs->set_bit(i, 1);
    }
    // do the same for all of the library procedures?
  } else {
    BitVector visited;
    suif_assert_message(to_visit.size() != 0,
			("Could not find a entry procedure 'main'\n"));
    //    list<ProcedureSymbol *> to_visit;
    //    for (suif_vector<ProcedureSymbol*>::iterator iter =
    //	   entry.begin();
    //	 iter != entry.end();
    //	 iter++) {
    //      ProcedureSymbol *ps = (*iter);
    //      to_visit.push_back(ps);
    //    }

    while (!to_visit.empty()) {
      // Do this is a breadth first manner.
      while (!to_visit.empty()) {
	//ProcedureSymbol *ps = to_visit.front();
	//	to_visit.pop_front();
	ProcedureSymbol *ps = to_visit.back();
	to_visit.pop_back();
	if (_verbose_output->is_set()) {
	  fprintf(stderr, "visiting Proc: '%s'\n", ps->get_name().c_str());
	}
	if (!ps->peek_annote("reachable_proc")) {
	  ps->append_annote(create_general_annote(get_suif_env(),
						  "reachable_proc"));
	}
	ProcedureDefinition *procdef = ps->get_definition();
	if (procdef != NULL) {
	  // skip libraries that have none.
	  // @@ fix this later
	  _state->process_a_procedure_definition(procdef);
	}
	suif_hash_map<ProcedureSymbol*,unsigned>::iterator it =
	  _procnums->find(ps);
	suif_assert(it != _procnums->end());
	unsigned procnum = (*it).second;
	suif_assert(procnum < _procset->size());
	suif_assert(visited.get_bit(procnum) == false);
	visited.set_bit(procnum, true);//.set_bit(procnum, true);
      }

      if (_do_by_ref->is_set()) {
	list<ProcedureSymbol*> *the_list = _state->get_reachable_procedures();
	while (!the_list->empty()) {
	  ProcedureSymbol *ps = the_list->front();
	  the_list->pop_front();
	  suif_hash_map<ProcedureSymbol*,unsigned>::iterator it =
	    _procnums->find(ps);
	  if (it == _procnums->end()) continue;

	  suif_assert(it != _procnums->end());
	  unsigned procnum = (*it).second;
	  if (!visited.get_bit(procnum)) {
	    to_visit.push_front(ps);
	  }
	}
	delete the_list;
      } else { // find the reachable procedures from the call sites
	// This is not an efficient way to do this.
	// it should be integrated into the algorithm
	BitVector procs_to_visit;
	list<CallStatement*> *the_list = _state->get_call_sites();
	while (!the_list->empty()) {
	  ExecutionObject *cal = the_list->front();
	  the_list->pop_front();
	  ecr_node *node =
	    _state->get_ecr_annotation()->find_ecr_call_target(cal);
	  ecr_node *lambda_node =
	    _state->get_ecr_comp()->get_lambda_pointed_to(node);
	  lambda_type *l = lambda_node->get_data()->get_lambda_type();
	  unsigned num_targs = l->num_targets();
	  for (unsigned i = 0; i < num_targs; i++) {
	    ProcedureSymbol *ps = l->get_target(i);
	    suif_hash_map<ProcedureSymbol*,unsigned>::iterator it =
	      _procnums->find(ps);
	    if (it == _procnums->end()) continue;

	    suif_assert(it != _procnums->end());
	    unsigned procnum = (*it).second;
	    if (!procs_to_visit.get_bit(procnum)) {
	      if (!visited.get_bit(procnum)) {
		to_visit.push_front(ps);
		procs_to_visit.set_bit(procnum, true);
	      }
	    }
	  }
	}
	delete the_list;

      }
    }
    (*_reachable_procs) = visited;

      // Print out the unreached procedures and stats
    unsigned num_procs = _procnums->size();
    unsigned check_num_procs = 0;
    unsigned num_reachable = 0;
    for(suif_hash_map<ProcedureSymbol*,unsigned>::iterator iter =
	  _procnums->begin();
	iter != _procnums->end(); iter++) {
      suif_hash_map<ProcedureSymbol*,unsigned>::pair &p = *iter;
      check_num_procs++;
      if (visited.get_bit(p.second)) {
	num_reachable++;
      } else {
	if (_verbose_output->is_set()) {
	  fprintf(stderr, "Unreachable Procedure: '%s'\n",
		  p.first->get_name().c_str());
	}
      }
    }
    suif_assert(num_procs == check_num_procs);
    if(!_silent->is_set()){
     fprintf(stderr, "Reachable Procs/All Procs : %d/%d (%0.2f%%)\n",
	    num_reachable, num_procs,
	    safe_pct(num_reachable, num_procs));
    }
  }
  finalize();
  delete _state; _state = NULL;
  delete _procnums; _procnums = NULL;
  delete _procset; _procset = NULL;
  delete _reachable_procs; _reachable_procs = NULL;
  //delete _walker_inits; _walker_inits = NULL;
}

void EcrAliasPass::finalize() {
  SuifEnv *s = get_suif_env();
  FileSetBlock *file_set_block = s->get_file_set_block();
  PrintSubSystem *psub = s->get_print_subsystem();

  unsigned num_call_sites = 0;
  unsigned num_alloc_sites = 0;
  unsigned direct_call_sites = 0;
  unsigned indirect_call_sites = 0;
  unsigned indirect_targs = 0;
  unsigned indirect_zero_call_sites = 0;



  list<CallStatement *> *call_sites = _state->get_call_sites();
  {
  for (list<CallStatement*>::iterator iter =
	 call_sites->begin();
       call_sites->end() != iter; iter++) {

    CallStatement *call = (*iter);

    ProcedureSymbol *target = get_procedure_target_from_call(call);
    if (target != NULL) {
      direct_call_sites++;
    } else {
      // find the number of possible targets.
      unsigned local_targs = find_num_call_targets(call, _state);
      if (local_targs == 0) {
	indirect_zero_call_sites++;
      }
      indirect_targs += local_targs;
      indirect_call_sites++;
      if (_verbose_output->is_set()) {
	// it can't be called, but usually points out a problem.
	String addr_expr =
	  psub->print_to_string("cprint", call->get_callee_address());
	fprintf(stderr, "Indirect Call(%s): %d targets\n",
		addr_expr.c_str(),
		indirect_targs);
      }
    }
    if (is_allocation_site(call, _state)) {
      num_alloc_sites++;
    }
    num_call_sites++;
  }
  }
  delete call_sites;
  if(!_silent->is_set()){
    fprintf(stderr, "Direct Call Sites/All Call Sites: %d/%d (%0.2f%%)\n",
	  direct_call_sites, num_call_sites,
	  safe_pct(direct_call_sites, num_call_sites));
    fprintf(stderr, "Alloc Sites/All Call Sites: %d/%d (%0.2f%%)\n",
	  num_alloc_sites, num_call_sites,
	  safe_pct(num_alloc_sites, num_call_sites));

    fprintf(stderr, "Indirect Call Targets/Indirect Calls: %d/%d (%0.1f)\n",
	  indirect_targs, indirect_call_sites,
	  safe_pct(indirect_targs, indirect_call_sites)/100.0);
    fprintf(stderr, "Indirect NULL Call Targets/Indirect Calls: %d/%d (%0.2f%%)\n",
	  indirect_zero_call_sites, indirect_call_sites,
	  safe_pct(indirect_zero_call_sites, indirect_call_sites));
  }

  do_generate_call_graph(file_set_block);

  ecr_annotation_manager *mgr = _state->get_ecr_annotation();
  mgr->place_annotations(file_set_block);

  //  do_place_annotations(file_set_block, _state);
  //  do_statistics(file_set_block);
}

static void add_call_graph_edge(ProcedureSymbol *source, ExecutionObject *call,
			 ProcedureSymbol *dest) {
  static LString k_call_target = "call_target";
  CallTargetAnnote *an =
    to<CallTargetAnnote>(call->peek_annote(k_call_target));
  if (an == NULL) {
    an = create_call_target_annote(source->get_suif_env(),
				   k_call_target);
    call->append_annote(an);
  }
  for (Iter<ProcedureSymbol*> iter =
	 an->get_target_iterator();
       iter.is_valid();
       iter.next()) {
    ProcedureSymbol *ps = iter.current();
    if (ps == dest) return;
  }
  an->append_target(dest);
}

static void add_call_graph_node(ProcedureSymbol *ps) {
  return;
}

void EcrAliasPass::do_generate_call_graph(FileSetBlock *file_set_block ) {
  //  SuifEnv *s = get_suif_env();
  EcrAliasState *state = ((EcrAliasState *)_maps->get_user_state());

  // iterate over all the procedures and print them as nodes.
  // iterate over all the call sites and print out their
  // direct target and/or indirect targets

  {
  for (suif_vector<ProcedureSymbol*>::iterator iter =
	 _procset->begin();
       iter != _procset->end();
       iter++) {
    ProcedureSymbol *ps = (*iter);
    add_call_graph_node(ps);
  }
  }

  // Now let's build a call graph
  list<CallStatement *> *call_sites = state->get_call_sites();
  for (list<CallStatement*>::iterator iter =
	 call_sites->begin();
       call_sites->end() != iter; iter++) {

    CallStatement *call = (*iter);
    ProcedureDefinition *source_def = get_procedure_definition(call);
    if (source_def == NULL && !_state->is_silent()) {
      fprintf(stderr, "WARNING: invalid proc_def\n");
      continue;
    }
    ProcedureSymbol *source = source_def->get_procedure_symbol();
    if (!source->peek_annote("reachable_proc")) { continue; }

    ProcedureSymbol *target = get_procedure_target_from_call(call);
    if (target != NULL) {
      // Here is an edge:
      add_call_graph_edge(source, call, target);
    } else {
      lambda_type *l = get_lambda_type(call, state);
      unsigned num_targs = l->num_targets();
      for (unsigned i = 0; i < num_targs; i++) {
	ProcedureSymbol *target = l->get_target(i);
	add_call_graph_edge(source, call, target);
      }
    }
  }
  delete call_sites;
}



Module *EcrAliasPass::clone() const {
  return(Module*)this;
}



class PrintCallGraphPass : public Pass {

  String _output_file;
public:
  PrintCallGraphPass(SuifEnv *suif_env,
		     const LString &name = "print_call_graph_pass") :
    Pass(suif_env, name) {}
  void initialize();
  void do_file_set_block(FileSetBlock *file_set_block );
  Module *clone() const { return (Module*)this; }
};

void
PrintCallGraphPass::initialize() {
  Module::initialize();
  _command_line -> set_description(
				   "print_call_graph_pass prints the edges "
				   "in the call graph on a suif file" );
  OptionSelection *sel = new OptionSelection();

  Option *libname =
    build_prefixed_string( "-output", "outfile", &_output_file,
			   "Output file. Defaults to stdout" );
  sel->add(libname);
  _command_line->add(new OptionLoop(sel, true));
}

void PrintCallGraphPass::do_file_set_block(FileSetBlock *file_set_block) {
  // Iterate over all of the
  FILE *fp = NULL;
  if (_output_file != emptyString) {
    fp = fopen(_output_file.c_str(), "w");
    suif_assert_message(fp != NULL,
			("Unable to open output file '%s'\n",
			 _output_file.c_str()));
  }
  if (fp == NULL) {
    fp = stdout;
  }

  for (Iter<CallTargetAnnote> iter =
	 object_iterator<CallTargetAnnote>(file_set_block);
       iter.is_valid(); iter.next()) {
    CallTargetAnnote *an = &iter.current();
    CallStatement *call = to<CallStatement>(an->get_parent());
    ProcedureSymbol *source = get_procedure_definition(call)->get_procedure_symbol();

    bool is_direct =
      (get_procedure_target_from_call(call) != NULL);

    for (Iter<ProcedureSymbol*> iter =
	   an->get_target_iterator();
	 iter.is_valid();
	 iter.next()) {
      ProcedureSymbol *target = iter.current();
      if (!is_direct) {
	fprintf(fp, "I");
      }
      fprintf(fp, "CALL: %s->%s\n",
	      source->get_name().c_str(),
	      target->get_name().c_str());
    }
  }
}




/*
 * The CountSuifObjectsPass will
 * count the number of suif objects in the file
 * I'd like to add a parameter that
 * allows the user to specify the name
 * of a metaclass to find.
 */

class CountSuifObjectsPass : public Pass {
public:
  CountSuifObjectsPass(SuifEnv *suif_env, const LString &name = "count_suif_objects_pass") :
    Pass(suif_env, name) {}
  void do_file_set_block(FileSetBlock *file_set_block ) {
    unsigned count = 0;
    for (Iter<SuifObject> iter = object_iterator<SuifObject>(file_set_block);
	 iter.is_valid(); iter.next(), count++) {
    }
    fprintf(stderr, "SuifObject count: %d\n", count);
    unsigned count_2 = object_iterator<SuifObject>(file_set_block).length();

    fprintf(stderr, "SuifObject count2: %d\n", count_2);
    if (count_2 != count) {
      fprintf(stderr, "SuifObject iterator has problems!!!\n");
    }

  }
  Module *clone() const { return (Module*)this; }
};


//extern "C" void EXPORT init_ecr_annotenodes( SuifEnv* suif );
//extern "C" void EXPORT init_basicnodes(SuifEnv *suif_env);
//extern "C" void EXPORT init_suifnodes(SuifEnv *suif_env);
//extern "C" void EXPORT init_bit_vector(SuifEnv *suif_env);

extern "C" void EXPORT init_ecr_alias(SuifEnv *suif_env) {
  // This will be fixed as soon as we have the deferred initialization
  // support in the SuifEnv

  //  static bool done = false;
  //  if (done) return;
  //  done = true;

  suif_env->require_module("suifnodes");
  suif_env->require_module("bit_vector");
  suif_env->require_module("ecrnodes");
  suif_env->require_module("iputils");
  suif_env->require_module("suifprinter");


  // register the suifpass as a listener for the
  // ecr_alias_walker



  ModuleSubSystem *ms = suif_env->get_module_subsystem();

  EcrAliasPass *pass = new EcrAliasPass(suif_env,
					"ecr_alias_pass");
  ms->register_module(pass);
  ms->register_module(new CountSuifObjectsPass(suif_env));
  ms->register_module(new PrintCallGraphPass(suif_env));


  // register the EcrAliasPass as a listener and producer for the
  // ecr_alias_walker
  ms->register_interface_listener( pass, "ecr_alias_walker");
  ms->register_interface_producer( pass, "ecr_alias_walker");
  //  ms->register_module(new FixEcr(suif_env));
}

extern void init_ecr_annote_cloning(CloneSubSystem *css) {}