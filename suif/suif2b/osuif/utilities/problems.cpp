// $Id: problems.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include <strstream.h>
#include <stdarg.h>
#include <stdio.h>

#include "suifkernel/suif_env.h"
#include "suifkernel/suifkernel_messages.h"

#include "osuifutilities/problems.h"


void OsuifProblems::error( const char* format, ... ) {
  String str = "ERROR: ";

  va_list ap;
  va_start( ap, format );

  fprintf( stderr, str.c_str() );
  vfprintf( stderr, format, ap );
  fprintf( stderr, "\n" );

  abort();
}


void OsuifProblems::warning( const char* format, ... ) {
  String str = "WARNING: ";
    
  va_list ap;
  va_start( ap, format );

  fprintf( stderr, str.c_str() );
  vfprintf( stderr, format, ap );
  fprintf( stderr, "\n" );

  va_end( ap );
}
