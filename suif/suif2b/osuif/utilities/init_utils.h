// $Id: init_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFUTILITIES__INIT_UTILS_H
#define OSUIFUTILITIES__INIT_UTILS_H


/*
 * Call init_* functions for important SUIF libraries.
 */
extern void initialize_suif_libraries( SuifEnv *suif_env );

/*
 * Call init_* functions for all(?) OSUIF  and SUIF libraries.
 */
extern void initialize_osuif_libraries( SuifEnv *suif_env );


#endif
