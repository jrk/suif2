#ifndef CFGRAPH_MODULE_H
#define CFGRAPH_MODULE_H

#include "suif_cfgraph_forwarders.h"
#include "cfgraph_forwarders.h"
#include "suifkernel/module.h"
#include "basicnodes/basic_forwarders.h"

class CFGraphBuilderReturn {
private:
  CFGraphNode *_entry;
  CFGraphNode *_exit;
public:
  CFGraphBuilderReturn(CFGraphNode *entry, CFGraphNode *exit);
  CFGraphBuilderReturn();
  CFGraphNode *get_entry() const { return(_entry); }
  CFGraphNode *get_exit() const { return(_exit); }
};

typedef CFGraphBuilderReturn (*CFGraphBuilderDispatch)(CFGraphState *,
						       ExecutionObject *);

class SuifCFGraphBuilderModule : public Module {
  CascadingMap<CFGraphBuilderDispatch> *_fns;
public:
  SuifCFGraphBuilderModule(SuifEnv *suif_env, 
			   const LString &name = "suif_cfgraph_builder");
  void initialize();
  Module *clone() const;
  /*CFGraphState * */
  /* Place the annotation on the pd */
  /* Should NOT be called if there already is an annotation on the pd */
  void build_cfgraph(ProcedureDefinition *pd);
  void add_builder_fn(const LString &name, CFGraphBuilderDispatch fn);

  CFGraphBuilderReturn build(CFGraphState *, ExecutionObject *) const;
};

#endif /* CFGRAPH_MODULE_H */
