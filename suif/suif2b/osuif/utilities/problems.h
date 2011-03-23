// $Id: problems.h,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#ifndef UTILITIES__PROBLEMS_H
#define UTILITIES__PROBLEMS_H

#include "common/i_integer.h"
#include "suifkernel/suifkernel_messages.h"


/**
 * This class defines error and warning functionality, which
 * is implemented on top of SUIF.
 *
 * This class cannot be instanciated.
 */
class OsuifProblems {
private:
  OsuifProblems();

public:
  /// Print error message to stderr and abort.
  static void error(const char* format, ...);

  /// Print warning message to stderr.
  static void warning(const char* format, ...);


  /// Print "Not implemented!" error message and abort.
  static void not_implemented() { error("Not implemented!"); }

  /// Print "Code has not been tested!" warning message.
  static void not_tested() { warning("Code has not been tested!"); }

  /// Print "Obsolete code executed!" warning message.
  static void obsolete() { warning("Obsolete code executed!"); }
};

// Define assertions that are guaranteed to fail.
#define osuif_fatal suif_assert( false )
#define osuif_fatal_message( params ) suif_assert_message ( false, (params) )


#endif
