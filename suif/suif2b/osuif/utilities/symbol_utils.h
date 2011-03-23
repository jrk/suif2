// $Id: symbol_utils.h,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#ifndef OSUIFUTILITIES__SYMBOL_UTILS_H
#define OSUIFUTILITIES__SYMBOL_UTILS_H

#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"
#include "osuifextensionnodes/osuifextension_forwarders.h"


/*
 * The is_address_taken field of 'sym' is set to true.
 * (Useful to clearly mark the code position that actually
 * take the address of Symbols.)
 */
extern void symbol_has_address_taken( Symbol* sym );


/*
 * If the is_address_taken field in 'sym' is true and
 * 'is_address_taken' is false nothing is done; otherwise
 * the is_address_taken field is set to 'is_address_taken'.
 */
extern void update_is_address_taken( Symbol* sym,
				     bool is_address_taken );


/*
 * Get the owning class of 'sym,' whereas 'sym' can be a
 * InstanceMethodType, StaticMethodType, InstanceFieldSymbol, or
 * StaticFieldSymbol.
 * If the owning class cannot be determined NULL is returned.
 */
extern ClassType* get_owning_class( Symbol* sym );


#endif /* OSUIFUTILITIES__SYMBOL_UTILS_H */
