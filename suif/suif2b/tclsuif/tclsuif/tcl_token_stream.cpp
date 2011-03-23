#include "tcl_token_stream.h"
#include "common/suif_vector.h"


TclTokenStream::TclTokenStream(int objc, Tcl_Obj * const objv[]) :
  TokenStream()
{
  for (int i=objc-1; 0<=i; i--) {
    _tokens->push_back(Token(String(),
			     String(Tcl_GetStringFromObj(objv[i], 0)),
			     String(" ")));
  }
}

