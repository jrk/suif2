#ifndef _TCLSUIF_TCL_TOKEN_STREAM_H_
#define _TCLSUIF_TCL_TOKEN_STREAM_H_

#include "tcl.h"
#include "suifkernel/token_stream.h"


/**
  * A TokenStream designed for tcl commands.  Tokens are taken from
  * an array of Tcl_Obj.
  */
class TclTokenStream : public TokenStream {
 public:
  TclTokenStream(int objc, Tcl_Obj * const objv[]);
  virtual ~TclTokenStream() {};
};

  

#endif // _TCLSUIF_TCL_TOKEN_STREAM_H_
