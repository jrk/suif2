// $Id: type_utils.h,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#ifndef OSUIFUTILITIES__TYPE_UTILS_H
#define OSUIFUTILITIES__TYPE_UTILS_H

#include "osuifnodes/osuif_forwarders.h"


/*
 * A class type is complete if both instance fields and methods
 * are complete.
 */
extern bool is_complete_class_type( ClassType* ctype );


/*
 * The instance field offsets and alignments of 'ctype' are set.
 * All offsets and alignments are computed from scratch. Existing
 * settings are ignored.
 * The alignment of 'ctype' is the longest field alignment.
 *
 * Note that this function is also useful to align a StructType.
 */
extern void do_class_type_layout( StructType* ctype );


/*
 * Return true if the result type of 'ptype' is void.
 */
extern bool is_void_result_type( CProcedureType* ptype );


#endif
