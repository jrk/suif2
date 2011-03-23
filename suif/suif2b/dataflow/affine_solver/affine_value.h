#ifndef AFFINE_VALUE_H
#define AFFINE_VALUE_H

#include "ecr_alias/ecr_annote_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "common/suif_hash_map.h"
#include "affine_forwarders.h"

class BuildCStringState;

// Gotta think about this a little
// Here's what we're going to keep:
// 
//  Variable->Value Map
//  Ecr -> list<StongAffineExpression, AffineValue>
//
//  We assume that there is a mapping of 
//    varuse,vardef,store,load->ecr
//
//    All other ecr vals
// With no ECR assume all in the same bit.
//
// Maybe we need an IndirectReferenceStatement??
// concrete IndirectReferenceStatement : Statement {
//    AffineExpression * owner address;
//    AffineExpression * owner value;
// };
//class IndirectReference {
//public:
//  AffineExpression * address;
//  AffineExpression * value;
//};

// But that really isn't enough because we would LOVE to
// be able to have a summary when appropriate:
// Actually we can do this in the Kleene. Cool.
// That means that the set of the indirect references for
// an ECR should be disjoint. I'll have to shink about the
// kleene meet some more.
#include "dflowsolver/region_value.h"


class AffineValue : public SDValue { 
public:
  
  AffineValue(EcrManagerAnnote *ecr,
	      BuildCStringState *estring,
	      bool unknown_is_bottom);
  virtual ~AffineValue();
  virtual SDValue *clone() const; // make a pure copy of this value.
  virtual void print(ion *the_ion) const;
  virtual SDValue &assign(const SDValue &val);
  
  virtual bool lub_meet(const SDValue &src);
  virtual bool irregular_widen(const SDValue &src);
  virtual bool kleene(const SDValue &src);
  virtual bool enter_sub_region(SuperGraphRegion *parent_region,
				SuperGraphRegion *sub_region,
				const SDValue &sub_region_src);
  virtual bool exit_sub_region(SuperGraphRegion *sub_region,
			       SuperGraphRegion *parent_region,
			       const SDValue &sub_region_src);

  // Maybe we'll implement these later..
  virtual bool lt_eq_defined() const { return false; }
  virtual bool lt_eq(const SDValue &src) const;
  virtual bool eq_defined() const { return false; }
  virtual bool eq(const SDValue &src) const;

  // That's it for the required Value/Lattice interface.
 private:
  // useful private functions
  bool lub_meet_indirect_reference(IndirectReferenceList *irlist,
				   IndirectReferenceList *other_irlist);
  void clear();


private:
  //  typedef list<IndirectReferenceStatement *> IndirectReferenceList;
  typedef suif_hash_map<VariableSymbol*, AffineExpression *> VarMap;
  typedef suif_hash_map<EcrSetObject*, IndirectReferenceList *> EcrMap;


  EcrManagerAnnote *_ecr;
  BuildCStringState *_estring;  // class for building expression strings
  VarMap *_var_map;
  EcrMap *_ecr_map;
  bool _unknown_is_bottom; // Everything is bottom. (normally undef == undefined)
  //  bool _undef_is_undef; // or undefined is a pass-through.
};


void init_affine_value(SuifEnv *s);
  
#endif /* AFFINE_VALUE_H */
