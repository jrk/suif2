#include "cfgraph_module.h"
#include "suifkernel/cascading_map.h"
#include "cfgraph_vis.h"
#include "basicnodes/basic.h"


SuifCFGraphBuilderModule::
SuifCFGraphBuilderModule(SuifEnv *suif_env, 
			 const LString &name) :
  Module(suif_env, name),
  _fns(NULL)
{}
void SuifCFGraphBuilderModule::initialize() {
  _fns = new CascadingMap<CFGraphBuilderDispatch>(get_suif_env(), NULL);
  cfgraph_pass_init_suif_maps(this);
}
Module *SuifCFGraphBuilderModule::clone() const {
  return ((Module*)this); 
}

void SuifCFGraphBuilderModule::build_cfgraph(ProcedureDefinition *pd) {
  CFGraphState *cstate = new CFGraphState(get_suif_env(), this,
					  pd);
  
  //  cstate->set_working_procedure_definition(pd);
  
  CFGraphNode *the_entry = cstate->get_cfgraph_entry_node(pd);
  CFGraphNode *the_exit = cstate->get_cfgraph_exit_node(pd);

  CFGraphBuilderReturn bodyval = cstate->build(pd->get_body());

  //CFGraphBuilderReturn retval = 
    cstate->connect_entry_exit(the_entry, bodyval, the_exit);

  // Now we need to fix up the
  // extra labels:
  cstate->fix_up_labels();

  // Now compute the df.
  //cstate->build_dominance_frontier();
  cstate->finish_annotes();

  // Save it all.
  //cstate->clear_working_procedure_definition();
  
  delete cstate;
  //return(state);
}

CFGraphBuilderReturn::CFGraphBuilderReturn(CFGraphNode *entry, 
					   CFGraphNode *exit) :
  _entry(entry),
  _exit(exit)
{}
CFGraphBuilderReturn::CFGraphBuilderReturn() : _entry(0), _exit(0) {}


CFGraphBuilderReturn SuifCFGraphBuilderModule::
build(CFGraphState *state, 
      ExecutionObject *obj) const {
  CFGraphBuilderReturn ret;
  if (obj == NULL) {  return(ret); }
  CFGraphBuilderDispatch fn = _fns->lookup(obj);
  if (!fn) return(ret);
  ret = (*fn)(state, obj);
  return(ret);
}

void SuifCFGraphBuilderModule::
add_builder_fn(const LString &name, 
	       CFGraphBuilderDispatch fn) {
  _fns->assign(name, fn);
}
