// $Id: walker_utils.h,v 1.1.1.1 2000/06/08 00:10:04 afikes Exp $

#ifndef OSUIFUTILITIES__WALKER_UTILS_H
#define OSUIFUTILITIES__WALKER_UTILS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "iokernel/cast.h"
#include "common/suif_list.h"
#include "suifkernel/group_walker.h"


/**
 * Abstract walker that is used as a base-class for CollectWalkerT
 */
class CollectWalker : public SelectiveWalker {
protected:
  list<SuifObject *> _hit_list;

public:
  CollectWalker( SuifEnv* env, const LString& the_type ) :
    SelectiveWalker( env, the_type) { }

  list<SuifObject*> get_hit_list() { return _hit_list; }

  ApplyStatus operator () (SuifObject* obj) =0;
};


/**
 * This walker collects all SUIF objects of a certain type T
 * in a list.
 */
template<class T>
class CollectWalkerT : public CollectWalker {
public:
  CollectWalkerT( SuifEnv* env ) :
    CollectWalker( env, T::get_class_name() ) { }

  // iterator interface
  unsigned size() { return _hit_list.size(); }
  T* at( int i ) { return to<T>( _hit_list[i] ); }
  T* operator[]( int i ) { return at(i); }
  
  ApplyStatus operator () (SuifObject* obj) {
    T* hit = to<T>( obj );
    _hit_list.push_back( hit );
    return Walker::Continue;
  }
};


#endif /* OSUIFUTILITIES__WALKER_UTILS_H */
