#ifndef __IPUTILS_QUERY__
#define __IPUTILS_QUERY__

#include <suifnodes/suif_forwarders.h>
#include <basicnodes/basic_forwarders.h>
#include <suifkernel/iter.h>
#include "iputils_forwarders.h"

#ifdef MSVC
#ifdef IPUTILS_EXPORTS
#define extern extern DLLEXPORT
#else
#define extern extern DLLIMPORT
#endif
#endif

extern const LString k_entry_point;
#undef extern

class IPUtilsQuery {
public:
    IPUtilsQuery();
    static Iter<ProcedureSymbol*> get_callees(CallStatement *call);

    /**
        Returns true if \a proc_sym is an entry point to the program, i.e.
        main, etc. 
		
		The result is cached in annotation k_entry_point.
    */
    static bool is_unique_entry_point(ProcedureSymbol* proc_sym);

    /**
        Returns the unique entry point to the program, i.e.
        main, etc. If the program has none, NULL is returned.
    */
    static ProcedureSymbol* get_unique_entry_point(const FileSetBlock* fsb);
};
#endif