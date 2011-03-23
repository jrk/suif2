#ifndef VTCL_H
#define VTCL_H

//#include <sty.h>
#include "common/suif_list.h"
#include "vcommands.h"
#include <tcl.h>
#include <tk.h>
#include <stdlib.h>
#include <string.h>
#include "suifkernel/suifkernel_messages.h"

/* globals */

extern Tcl_Interp *v_interp;	// Visual interpreter

/*
 * tcl command interface class
 *
 */
#define COMMAND_INCREMENT_SIZE 500

class tcl_terminator {
};

extern tcl_terminator *tcl_end;	// end of tcl command, will call tcl to
				// interpret the command
extern char *tcl_0;

class tcl_command {
private:
  char *command;       // points to current tcl_command (or 0)
  int end_command;  // offset of the last valid element of string (the 0)
  int end_reserved_space; // the first offset past the allocation area 
                             //   (all usable offsets have to be smaller)

  Tcl_Interp *interp;

protected:

  char* reserve_space( int size_to_reserve ) {
    // space_remaining can be negative! (in the beginning)
    int space_remaining = end_reserved_space - end_command - 1;
    if ( size_to_reserve > space_remaining ) {  
      int increment_size = size_to_reserve < COMMAND_INCREMENT_SIZE ?
                         COMMAND_INCREMENT_SIZE : size_to_reserve;
      end_reserved_space = end_reserved_space + increment_size;
      if ( !command ) end_reserved_space++; // space for extra 0
      char* new_space = new char[ end_reserved_space ];
      // initialize the new string
      if ( command ) {
        strcpy( new_space, command );
      } else {
        assert( end_command == 0 );
        new_space[0]=0; 
      }
      delete [] command;
      command = new_space;
    }
    suif_assert_message( ( command[end_command]==0 ), ("End of string must be 0" ));
    return command+end_command;
  }

   void reset_string() {
    delete [] command;
    command = 0;
    end_command = 0;
    end_reserved_space = 0;
   }

public:
  tcl_command() {
    command = 0;

    reset_string();
   
    interp = 0;
  }

  ~tcl_command() {
    reset_string();
  }

  void set_interp(Tcl_Interp *tcl_interp) { 
    interp = tcl_interp; 
  }

  Tcl_Interp *get_interp(void) { 
    return interp; 
  }

  /*
   * create command
   */
  void create_command(char *cmdName, Tcl_CmdProc *proc,
		      ClientData clientData,
		      Tcl_CmdDeleteProc *deleteProc) {
    Tcl_CreateCommand(interp, cmdName, proc, clientData, deleteProc);
  }

  int link_var(char *varName, char *addr, int type) {
    return (Tcl_LinkVar(interp, varName, addr, type));
  }
  void unlink_var(char *varName) {
    Tcl_UnlinkVar(interp, varName);
  }

  /*
   * Evaluating tcl command
   *
   */

  int operator << (const tcl_terminator *) {
    assert( command );

    // ATTENTION: This might be invoked recursively!!
    //    => reinitialize this object before invoking eval
    char* current_command = command;
    command = 0;
    reset_string();

    int result = eval( current_command );
  
    delete [] current_command;

    return result;
  }

  tcl_command &operator << (const char *string) {
    int length = strlen( string );
    char* start = reserve_space( length );
    sprintf( start, "{%s} ", string);
    end_command += length + 3;
    return *this;
  }
  tcl_command &operator << (const void *data) {
    const int buffer_size = 20;
    char buffer[ buffer_size ];
    sprintf( buffer, "%p ", data );
    int len = strlen( buffer );
    suif_assert_message( (len<=buffer_size), ("Pointer to string conversion has unexpected size (too large).\n") );
    strcat( reserve_space( len ), buffer );
    end_command += len;
    return *this;
  }
  tcl_command &operator << (int data) {
    const int buffer_size = 40;
    char buffer[ buffer_size ];
    sprintf( buffer, "%d ", data );
    int len = strlen( buffer );
    suif_assert_message(( len<=buffer_size), ("Int to string conversion has unexpected size (too large).\n" ));
    strcat( reserve_space( len ), buffer );
    end_command += len;
    return *this;
  }
  tcl_command &operator << ( double data ) {
    const int buffer_size = 40;
    char buffer[ buffer_size ];
    sprintf( buffer, "%f ", data );
    int len = strlen( buffer );
    suif_assert_message( (len<=buffer_size), ("Double to string conversion has unexpected size (too large).\n") );
    strcat( reserve_space( len ), buffer );
    end_command += len;
    return *this;
  }
  int eval(char *string) {
    int result = Tcl_Eval(interp, string);
    if (result != TCL_OK) {
      v_warning("Tcl command `%s': (%d) %s",
		string, result, interp->result);
    }
    return result;
  }
  int eval_file(char *filename) {
    return (Tcl_EvalFile(interp, filename));
  }

  /*
   * Get results from tcl
   */
  
  int operator >> (int &data) {
    data = atoi(interp->result);
    return TCL_OK;
  }
  int operator >> (char *&data) {
    strcpy(data, interp->result);
    return TCL_OK;
  }
  int operator >> (void *&data) {
    if (sscanf(interp->result, "%p", &data) != 1) {
      return TCL_ERROR;
    } else {
      return TCL_OK;
    }
  }
  char *result() {
    return interp->result;
  }
  
  /*
   * Misc
   */
  void print(FILE *fd = stdout) {
    fprintf(fd, "%s\n", command ? command : "<NO COMMAND>");
  }
};

extern tcl_command tcl;

#endif
