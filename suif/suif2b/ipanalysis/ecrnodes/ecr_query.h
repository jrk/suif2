#ifndef __ECR_QUERY__
#define __ECR_QUERY__

#include "ecr_forwarders.h"
#include <basicnodes/basic_forwarders.h>
#include <suifnodes/suif_forwarders.h>
#include <suifkernel/iter.h>

#include <oosplay/oosplay.h>
typedef oosplay_node<EcrSetTauObject*> TauNode;
typedef oosplay_tree<EcrSetTauObject*, TauNode> EcrSetTauObjectSet;
typedef oosplay_node<ProcedureSymbol*> ProcNode;
typedef oosplay_tree<ProcedureSymbol*, ProcNode> ProcedureSymbolSet;

class EcrAliasQuery {
public:
  EcrAliasQuery();

  static EcrSetTauObject* get_ecr_tau(LoadExpression *le);

  static EcrSetTauObject* get_ecr_tau(StoreStatement* ss);

  static EcrSetTauObject* get_ecr_tau(VariableSymbol *sym);

  EcrSetTauObjectSet *get_mod_set(SuifEnv *suifenv, ProcedureSymbol *sym);
  EcrSetTauObjectSet *get_ref_set(SuifEnv *suifenv, ProcedureSymbol *sym);
  EcrSetTauObjectSet *get_lmod_set(SuifEnv *suifenv, ProcedureSymbol *sym);
  EcrSetTauObjectSet *get_lref_set(SuifEnv *suifenv, ProcedureSymbol *sym);
};
#endif
