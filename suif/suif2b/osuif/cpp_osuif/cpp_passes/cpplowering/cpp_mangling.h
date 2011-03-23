#ifndef __CPPLOWERING_CPP_MANGLING
#define __CPPLOWERING_CPP_MANGLING

#include "common/MString.h"
#include "common/suif_list.h"
#include "utils/simple_module.h"
#include "


/**	Various forms of mangler can be derived from the basic mangler
 * 	by overriding virtual functions. You instantiate this as a pass
 *	so that a mangler can be selected with an import command
 */

class CppMangler : SimpleModule {
    public:
	
	Module* clone() const {return this;}
	void execute();

	virtual function mangle(
	
	

#endif
