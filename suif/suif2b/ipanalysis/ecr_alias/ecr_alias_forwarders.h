#ifndef ECR_ALIAS_FORWARDERS_H
#define ECR_ALIAS_FORWARDERS_H

#include "common/i_integer.h"

#include "basicnodes/basic_forwarders.h"
#include "suifkernel/suifkernel_forwarders.h"

class ecr_computation;

template <class data> class union_find_node;
class ecr_type;
typedef union_find_node<ecr_type *> ecr_node;

void remove_ecr_ref(ecr_node *);
void add_ecr_ref(ecr_node *);
IInteger get_ecr_nodeset_id(ecr_node *node);
ecr_node *find_top_ecr(ecr_node *node);
ecr_node *find_the_tau_pointed_to(ecr_node *node);
ecr_node *find_the_lambda_pointed_to(ecr_node *node);
bool is_ecr_bottom(ecr_node *node);
ecr_node *find_ecr(ecr_node *node);


ProcedureDefinition *find_proc_from_suif_object(SuifObject *s);

bool is_attribute_annote(AnnotableObject *the_azot, const LString & k_tmp);
void add_attribute_annote(SuifEnv *s, AnnotableObject *the_azot, 
			  const LString & k_tmp);
void append_an_annote(AnnotableObject *the_azot, Annote *an);
Annote *take_an_annote(AnnotableObject *the_azot, const LString &k_tmp);


class FileSetMgr;
class EcrAliasState;

//typedef union_find_owner<ecr_type *> ecr_owner;

#endif /* ECR_ALIAS_FORWARDERS_H */
