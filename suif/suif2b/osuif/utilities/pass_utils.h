// $Id: pass_utils.h,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#ifndef OSUIFUTILITIES__PASS_UTILS_H
#define OSUIFUTILITIES__PASS_UTILS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "iokernel/cast.h"
#include "common/suif_list.h"
#include "suifkernel/command_line_parsing.h" 
#include "suifkernel/token_stream.h" 
#include "suifpasses/passes.h"
#include "osuifutilities/walker_utils.h"


/**
 * Abstract pass that uses a CollectWalkerT to collect all
 * objects of a certain type T
 * After the object are collected process_suif_object() is called
 * for every one of them.
 */

template<class T>
class CollectWalkerPass : public Pass {
protected:
  FileSetBlock* _fsb;
  CollectWalkerT<T>* _walker;

public:
  CollectWalkerPass( SuifEnv* env, const LString &name) :
    Pass( env, name ),
    _walker( new CollectWalkerT<T>( env ) )
  {
    initialize_flags();
  }
 
  /// Set the default settings of the flags.
  virtual void initialize_flags() { }
  
  virtual void initialize() {
    Pass::initialize();
  }

  virtual bool parse_command_line(TokenStream *command_line_stream) {
    initialize_flags();
    return Pass::parse_command_line( command_line_stream );
  }

  OptionList* get_command_line() { return _command_line; }
  Module *clone() const { return (Module *) this; }

  virtual void do_file_set_block( FileSetBlock* fsb ) {
    suif_assert_message( fsb !=NULL,
			 ("FileSetBlock is NULL.") );
    _fsb = fsb;
    fsb->walk( *_walker );
    process_walker( _walker );
  }

  virtual void process_walker( CollectWalkerT<T>* walker ) {
    preprocess();
    for ( unsigned i = 0 ; i < walker->size() ; i++ ) {
      T* t = walker->at(i);
      process_suif_object( t );
    }
    postprocess();
  }

  virtual void preprocess() { }
  virtual void postprocess() { }
  virtual void process_suif_object(T* t) =0;
};


#endif /* OSUIFUTILITIES__PASS_UTILS_H */
