#include "ecr_query.h"
#include "ecr.h"
#include "ecr_constants.h"
#include "iputils/iputils.h"
#include "suifnodes/suif.h"
#include <suifkernel/group_walker.h>
#include <suifkernel/utilities.h>
#include <suifkernel/iter.h>
#include <iputils/iputils_query.h>

EcrAliasQuery::EcrAliasQuery() {}

EcrSetTauObject*
EcrAliasQuery::get_ecr_tau(LoadExpression *le) {

    EcrRefAnnote* ann = 
        to<EcrRefAnnote>(le->peek_annote(k_ecr_load_from));

    return(ann->get_ecr_set()->get_points_to());
}

EcrSetTauObject*
EcrAliasQuery::get_ecr_tau(StoreStatement* ss) {

    EcrRefAnnote* ann = 
        to<EcrRefAnnote>(ss->peek_annote(k_ecr_store_to));

    return(ann->get_ecr_set()->get_points_to());
}

EcrSetTauObject*
EcrAliasQuery::get_ecr_tau(VariableSymbol *sym) {

    EcrRefAnnote* ann = 
        to<EcrRefAnnote>(sym->peek_annote(k_ecr_variable));

    return(ann->get_ecr_set());
}


void add(EcrSetTauObjectSet *t, EcrSetTauObject *v) {
  TauNode *node;
  if (!t->lookup(v, &node)) {
    t->insert_node(new TauNode(v));
  }
}

void add(EcrSetTauObjectSet *t, EcrSetTauObjectSet *toadd) {
  class AddOne: public IterClosure<TauNode> {
  public:
    EcrSetTauObjectSet *_add_to;
    AddOne(EcrSetTauObjectSet *add_to): _add_to(add_to) {};
    void apply(TauNode *n) {
      add(_add_to, n->get_key1());
    }
  } add_one(t);
  toadd->iterate(&add_one);
}

typedef oosplay_node_elt<ProcedureSymbol*, EcrSetTauObjectSet*> ModRefMapNode;
typedef oosplay_tree<ProcedureSymbol*, ModRefMapNode> ModRefMap;

ModRefMap proc_to_lmod;
ModRefMap proc_to_mod;
ModRefMap proc_to_lref;
ModRefMap proc_to_ref;

EcrSetTauObjectSet *
EcrAliasQuery::get_lmod_set(SuifEnv *suifenv, ProcedureSymbol *sym) {
  ModRefMapNode *lookup;
  if (proc_to_lmod.lookup(sym, &lookup)) {
    return lookup->get_elt();
  }
  else {
    EcrSetTauObjectSet *retval = new EcrSetTauObjectSet;
    proc_to_lmod.insert_node(new ModRefMapNode(sym, retval));

    Iter<SuifObject> obj_iter = 
      object_iterator<SuifObject>(sym->get_definition());
    for (; obj_iter.is_valid(); obj_iter.next()) {
      SuifObject *x = &obj_iter.current();
      if (is_kind_of<StoreVariableStatement>(x)) {
	StoreVariableStatement *svstmt = to<StoreVariableStatement>(x);
	EcrSetTauObject *t = get_ecr_tau(svstmt->get_destination());
	add(retval, t);
      }
      else if (is_kind_of<StoreStatement>(x)) {
	StoreStatement *sstmt = to<StoreStatement>(x);
	EcrSetTauObject *t = get_ecr_tau(sstmt);
	add(retval, t);
      }
    }
    return retval;
  }
}

EcrSetTauObjectSet *
EcrAliasQuery::get_mod_set(SuifEnv *suifenv, ProcedureSymbol *sym) {
  ModRefMapNode *lookup;
  if (proc_to_mod.lookup(sym, &lookup)) {
    return lookup->get_elt();
  }
  else {
    EcrSetTauObjectSet *mod_set = new EcrSetTauObjectSet;
    proc_to_mod.insert_node(new ModRefMapNode(sym, mod_set));

    add(mod_set, get_lmod_set(suifenv, sym));
    int changed = 1;
    while (changed) {
      int count = mod_set->tree_size();
      Iter<CallStatement> call_iter = 
	object_iterator<CallStatement>(sym->get_definition());
      for (; call_iter.is_valid(); call_iter.next()) {
	Iter<ProcedureSymbol*> callee_iter = 
	  IPUtilsQuery::get_callees(&call_iter.current());
	for (; callee_iter.is_valid(); callee_iter.next()) {
	  add(mod_set, get_mod_set(suifenv, callee_iter.current()));
	}
      }
      changed = count != mod_set->tree_size();
    }
    return mod_set;
  }
}

EcrSetTauObjectSet *
EcrAliasQuery::get_lref_set(SuifEnv *suifenv, ProcedureSymbol *sym) {

  ModRefMapNode *lookup;
  if (proc_to_lref.lookup(sym, &lookup)) {
    return lookup->get_elt();
  }
  else {
    EcrSetTauObjectSet *retval = new EcrSetTauObjectSet;
    proc_to_lref.insert_node(new ModRefMapNode(sym, retval));
    Iter<LoadVariableExpression> obj_iter = 
      object_iterator<LoadVariableExpression>(sym->get_definition());
    for (; obj_iter.is_valid(); obj_iter.next()) {
      LoadVariableExpression *lvexpr = &obj_iter.current();
      add(retval, get_ecr_tau(lvexpr->get_source()));
    }
    return retval;
  }
}

EcrSetTauObjectSet *
EcrAliasQuery::get_ref_set(SuifEnv *suifenv, ProcedureSymbol *sym) {
  ModRefMapNode *lookup;
  if (proc_to_ref.lookup(sym, &lookup)) {
    return lookup->get_elt();
  }
  else {
    EcrSetTauObjectSet *ref_set = new EcrSetTauObjectSet;
    proc_to_ref.insert_node(new ModRefMapNode(sym, ref_set));

    add(ref_set, get_lref_set(suifenv, sym));
    int changed = 1;
    while (changed) {
      int count = ref_set->tree_size();
      Iter<CallStatement> call_iter = 
	object_iterator<CallStatement>(sym->get_definition());
      for (; call_iter.is_valid(); call_iter.next()) {
	Iter<ProcedureSymbol*> callee_iter = 
	  IPUtilsQuery::get_callees(&call_iter.current());
	for (; callee_iter.is_valid(); callee_iter.next()) {
	  add(ref_set, get_ref_set(suifenv, callee_iter.current()));
	}
      }
      changed = count != ref_set->tree_size();
    }
    return ref_set;
  }
}
