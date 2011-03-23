#ifndef DAVE_MAP_H
#define DAVE_MAP_H

#include "common/machine_dependent.h"
#include "common/common_forwarders.h"

// map the node class to an integer that
// is allocated when we find a new one
template <class node_t> 
class dave_map {
  typedef suif_hash_map<node_t, size_t> map_t;
  typedef suif_vector<node_t> vect_t;

  map_t *_map;
  vect_t *_vect;
 public:
  dave_map() { 
    _map = new suif_hash_map<node_t, size_t>();
    _vect = new vect_t();
  }
  ~dave_map() { delete _map; delete _vect; }
  size_t lookup_id(node_t node) const { 
    typename map_t::iterator iter = _map->find(node);
    assert(!(iter == _map->end()));
    return((*iter).second);
  }
  size_t retrieve_id(node_t node) const { 
    typename map_t::iterator iter = _map->find(node);
    if (iter == _map->end()) {
      size_t id = _vect->size();
      _vect->push_back(node);
      _map->enter_value(node, id);
      return(id);
    }
    return((*iter).second);
  }
  node_t get_node(size_t id) const {
    return((*_vect)[id]);
  }
  size_t size() const { return(_vect->size()); }
};
  
  
#endif /* DAVE_MAP_H */
