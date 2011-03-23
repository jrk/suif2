// $Id: attribute_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFUTILITIES__ATTRIBUTE_UTILS_H
#define OSUIFUTILITIES__ATTRIBUTE_UTILS_H

#include "common/lstring.h"
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "osuifnodes/osuif_forwarders.h"


/*
 * Return true if the symbol 'sym' supports attributes.
 * This is the case for all OSUIF classes that subclass from
 * Symbol.
 */
extern bool supports_attributes( Symbol* sym );


/*
 * Check if the OSUIF symbol 'sym' has the attribute 'attr'.
 * If the symobl does not support attributes
 * (i.e., it is no OSUIf symbol) false is returned.
 */
extern bool has_attribute( Symbol* sym, const LString& attr );


/*
 * Add attribute 'attr' to the OSUIF symbol 'sym'.
 * If 'sym' is not an OSUIF symbol nothing is done.
 */
extern void add_attribute( Symbol* sym, const LString& attr );


#endif /* OSUIFUTILITIES__ATTRIBUTE_UTILS_H */
