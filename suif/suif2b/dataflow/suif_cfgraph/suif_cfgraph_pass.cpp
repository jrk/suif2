/* file "suif_cfgraph_pass.cc" */


/*
       Copyright (c) 1998 Stanford University

       All rights reserved.

       This software is provided under the terms described in
       the "suif_copyright.h" include file.
*/

#include "common/suif_copyright.h"


/*
      This is the main implementation file of the suif_cfgraph_pass,
      a library for interprocedural alias analysis
*/


#include "suif_cfgraph_pass.h"
#include "common/lstring.h"
#include "ion/ion.h"
#include "suifpasses/suifpasses.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "suifkernel/io_subsystem.h"
#include "suifnodes/suif.h"
#include "suifkernel/command_line_parsing.h"
#include "suifkernel/print_subsystem.h"
#include "cfgraph_module.h"

#include "cfgraph_vis.h"
#include "suif_cfgraph.h"
#include "sgraph_algs/sgraph_algs.h"
#include "suif_cfgraph_query.h"
#include "ion/ion.h"

extern "C" void EXPORT init_suif_cfgraphnodes( SuifEnv* suif );

/*    
static void print_dot(CFGraphAnnote *cfg_an, 
		      SGraph *the_super_graph,
		      ion *the_ion)
{
  PrintSubSystem *psub = cfg_an->get_suif_env()->get_print_subsystem();
  suif_vector<String> cfg_names;
  for (SNodeIter bv_iter(the_super_graph->get_node_iterator());
       bv_iter.is_valid(); bv_iter.next()) {
    SGraphNode node = bv_iter.current();
    CFGraphNode *cnode = cfg_an->get_node(node);
    if (!cnode->get_is_executable()
	&& !is_kind_of<LabelLocationStatement>(cnode->get_base()))
      continue;
    
    AnnotableObject *aobj = cnode->get_owned_object();
    if (aobj == NULL) {
      aobj = cnode->get_base();
    }
    String str = psub->print_to_string("cprint",  aobj);
    while (cfg_names.size() <= node) { cfg_names.push_back(emptyString); }
    cfg_names[node] = str;
  }
  //stderr_ion->printf("The CFG\n");
  export_named_dot(the_super_graph, the_ion,
		   &cfg_names);
  
}
*/


class SuifCFGraphPass : public PipelinablePass {
private:
  //  WalkingMaps *_maps;

  //  typedef void (*walker_init_fn)(WalkingMaps *);

  //  list<walker_init_fn> *_walker_inits;

  OptionLiteral *_verbose_output;
  OptionLiteral *_dot_output;

public:
  SuifCFGraphPass(SuifEnv *suif_env, 
		  const LString &name = "suif_cfgraph_pass");
  //  void interface_object_created(Module *producer, 
  //				const LString &interface_name);
  void initialize();
  void do_procedure_definition(ProcedureDefinition *proc_def );
  Module *clone() const;
};




//typedef ((void)(*cfgraph_map_fn))(WalkingMaps *);

SuifCFGraphPass::SuifCFGraphPass(SuifEnv *suif_env, const LString &name) :
  PipelinablePass(suif_env, name)
  //  _maps(0)
{
  //  walker_init_fn fn = cfgraph_pass_init_suif_maps;
  //  _walker_inits = new list<walker_init_fn>;
  //  set_interface("suif_cfgraph_builder",
  //		(Address)fn);
  
}

/*
void SuifCFGraphPass::interface_object_created(Module *producer, 
					       const LString &interface_name) {
  Address addr = producer->get_interface( interface_name );
  // Here's the scary part.  Convert it.
  if (addr == 0) return;
  walker_init_fn fn = (walker_init_fn) addr;
  _walker_inits->push_back(fn);
}
*/

void SuifCFGraphPass::initialize() {
  PipelinablePass::initialize();
  //SuifEnv *s = get_suif_env();

  Module::initialize();
  _command_line->set_description("Control Flow Graph builder pass");

  _verbose_output = new OptionLiteral("-v");
  _dot_output = new OptionLiteral("-dot");
  OptionSelection *sel = new OptionSelection();
  sel->add(_verbose_output);
  sel->add(_dot_output);
  OptionLoop *opt = new OptionLoop(sel, true);
  _command_line->add(opt);

  
  //ModuleSubSystem *ms = s->get_module_subsystem();
  //  ms->register_interface_listener( this, "suif_cfgraph_builder");
  //  ms->register_interface_producer( this, "suif_cfgraph_builder");
  

  //  CFGraphState *state = new CFGraphState(s);
  //  _maps = new WalkingMaps(s, "suif_cfgraph_pass_walker",
  //			  (Address) state);
  //  state->set_maps(_maps);
  
  // Initialize it
  
  
  // Now do the deferred initialization
  //  for (list<walker_init_fn>::iterator iter =
  //	 _walker_inits->begin();
  //       iter != _walker_inits->end();
  //       iter++) {
  //    walker_init_fn fn = (*iter);
    // This is an indirect function call through
    // something that had bee untyped.
    // I'd REALLY like to be able to validate it
    // before using it.
  //    (*fn)(_maps);
  //  }

  
}
 
void SuifCFGraphPass::do_procedure_definition(ProcedureDefinition *proc_def )
  {
    CFGraphQuery q(get_suif_env(), proc_def);

    //    bool verbose = _verbose_output->is_set();
    bool dot_output = _dot_output->is_set();
    if (dot_output) {
      const char *fname = 
	(String(proc_def->get_procedure_symbol()->get_name()) + ".dot").c_str();
      FILE *fp = fopen(fname, "w");
      if (!fp) {
	suif_warning("Could not open file '%s'\n", fname);
      } else {
	file_ion the_ion(fp);
	q.print_dot(&the_ion);
	fclose(fp);
      }
    }
  }

Module *SuifCFGraphPass::clone() const {
  return(Module*)this;
}


class PrintSuifCFGraphPass : public PipelinablePass {
public:
  PrintSuifCFGraphPass(SuifEnv *suif_env, 
		       const LString &name = "print_suif_cfgraph");
  void do_procedure_definition(ProcedureDefinition *proc_def );
  Module *clone() const {return (Module*)this; }
};

PrintSuifCFGraphPass::PrintSuifCFGraphPass(SuifEnv *suif_env, 
					   const LString &name) :
  PipelinablePass(suif_env, name)
{
}

void PrintSuifCFGraphPass::do_procedure_definition(ProcedureDefinition *pd) 
{
  CFGraphQuery q(get_suif_env(), pd);
  q.print_graph(stdout_ion);
}

class PrintDotSuifCFGraphPass : public PipelinablePass {
public:
  PrintDotSuifCFGraphPass(SuifEnv *suif_env, 
		       const LString &name = "print_suif_cfgraph_to_dot");
  void do_procedure_definition(ProcedureDefinition *proc_def );
  Module *clone() const {return (Module*)this; }
  void initialize();
};


PrintDotSuifCFGraphPass::PrintDotSuifCFGraphPass(SuifEnv *suif_env, 
					   const LString &name) :
  PipelinablePass(suif_env, name)
{
}

void PrintDotSuifCFGraphPass::do_procedure_definition(ProcedureDefinition *pd) 
{
  CFGraphQuery q(get_suif_env(), pd);
  q.print_dot(stdout_ion);
}

void PrintDotSuifCFGraphPass::initialize() {
  PipelinablePass::initialize();
  // add an output file -o file
  // add a procedure filter option -proc name
}
    


extern "C" void EXPORT init_suif_cfgraph(SuifEnv *suif_env) {

  // Initialize libraries
  //suif_env->require_module("suif_cfgraphnodes");
  suif_env->require_module("bit_vector");
  suif_env->require_module("suifnodes");
  suif_env->require_module("cfenodes");
  suif_env->require_module("typebuilder");
  suif_env->require_module("suifcloning");
  suif_env->require_module("suifprinter");
  init_suif_cfgraphnodes( suif_env );

  ModuleSubSystem *ms = suif_env->get_module_subsystem();

  SuifCFGraphPass *pass = new SuifCFGraphPass(suif_env);
  ms->register_module(pass);
  ms->register_module(new SuifCFGraphBuilderModule(suif_env));
  ms->register_module(new PrintSuifCFGraphPass(suif_env));
  ms->register_module(new PrintDotSuifCFGraphPass(suif_env));

}

void init_suif_cfgraph_cloning(CloneSubSystem *) {
  
}
