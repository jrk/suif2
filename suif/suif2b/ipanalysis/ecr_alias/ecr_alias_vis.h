#ifndef ECR_ALIAS_VIS
#define ECR_ALIAS_VIS

/*
 * ******************************************************
 * *
 * * class ecr_alias_visitor
 * *
 * * Walk through the program representation and
 * * calculate the ecrs.
 * *
 * ******************************************************
 */

#include "ecr_type.h"
#include "ecr_computation.h"

#include "suifnodes/suif_forwarders.h"
#include "basicnodes/basic_forwarders.h"
#include "iokernel/iokernel_forwarders.h"
#include <common/system_specific.h>
#include "suifkernel/walking_maps.h"

class ecr_annotation_manager;

// Uses the following annotations:
extern DLLIMPORT LString k_ecr_proc;
extern DLLIMPORT LString k_ecr;

extern DLLIMPORT LString k_va_sym;
extern DLLIMPORT LString k_va_arg;
extern DLLIMPORT LString k_va_end;
extern DLLIMPORT LString k_va_start;


class EcrAliasState;


// This function will register all of the
// callbacks for nodes from the
// Basic and SUIF libraries.
// if any other library is registered,
// It should have a function
// void ecr_alias_pass_init_MODULE_maps(WalkingMaps *maps);
// That must be called
// after this module is loaded and before the
// pass execute().

void ecr_alias_pass_init_suif_maps(WalkingMaps *maps);

#endif /* ECR_ALIAS_VIS */