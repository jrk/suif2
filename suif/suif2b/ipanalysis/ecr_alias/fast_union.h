#ifndef FAST_UNION_H
#define FAST_UNION_H
/*
 * (c) 1997 Stanford University
 *
 * code for the suif compiler
 * Implementation of a container class for
 * a fast union/find algorithm
 * initially implemented as a class that can be subclassed
 * with data, it should be a template
 * Initial Implementation for SUIF:
 *     David Heine
 */


#ifndef STATIC_CAST
#define STATIC_CAST(t, obj) ((t)obj)
#endif

#include "ion/ion.h"
#include "common/suif_list.h"
#include "common/suif_vector.h"
#include "suifkernel/suifkernel_messages.h"

// This is an interface that we use.
// class data from the templates must conform to the interface..
class printable {
public:
  virtual void print(ion *) const = 0;
};

template <class data> class union_find_owner;

// These are now reference counted.
template <class data> class union_find_node {
  typedef union_find_node node_t;
  //  typedef slist_tos<union_find_node *> node_tos;
  typedef list<union_find_node *> node_tos;
  typedef union_find_owner<data> owner_t;
  
  // This is in cahoots with its owner. Most
  // interesting things happen at the owner level.
  friend union_find_owner<data>;

  node_t *_parent; // the parent
  owner_t *_owner; // the owner

  unsigned _num_sub_nodes; // Must ALWAYS be valid.
  data _data;
  unsigned _id;               // identifier
  
  node_tos _pending;
  node_tos _parents;
  //slist_tos<union_find_node *> _owns;

  int _num_refs; // should neve go negative,..

  // New stuff.

  void set_num_sub_nodes(unsigned num) { _num_sub_nodes = num; }
  void set_parent(node_t *the_parent) { 
    if (the_parent != 0) {
      the_parent->add_ref();
    }
    assert(the_parent != this);
    node_t *old_parent = _parent;
    _parent = the_parent;
    
    if (old_parent != 0) {
      old_parent->remove_ref();
    }
  }

  // NO DEFAULT CONSTRUCTORS
  union_find_node(const union_find_node &other){};
  union_find_node() {};

  // For the reference counting.


  void remove_ref() {
    _num_refs--;
    if (num_refs() > 0) { return; }
    // inform parent of impending doom
    if (!is_canonical()) {
      node_t *top = find_top();
      top->move_pending_from(this);
      top->move_parents_from(this);
      
      // remove it from the parent.
      set_parent(0);
    }
    // @@@ delete any pending here.
    delete this;
  }
  void add_ref() {
    _num_refs++;
  }

  // For use by our FRIEND.
  void destroy() {
    remove_ref();
  }
  union_find_node(data d, unsigned id, owner_t *owner) :
    _parent(0),
    _owner(owner),
    _num_sub_nodes(1),
    _data(d),
    _id(id),
    _pending(),
    _parents(),
    _num_refs(1)
    {}

  ~union_find_node() { 
    suif_assert_message(num_refs() == 0, ("deleted object doesn't have 0 refs"));
  }

  unsigned num_refs() const { return(_num_refs); }



  unsigned get_id() const { return(_id); }
  void add_child(node_t *node) {
    suif_assert(this->is_canonical());
    suif_assert(node->is_canonical());
    
    this->set_num_sub_nodes(node->get_num_sub_nodes() + 
			    this->get_num_sub_nodes());
    node->set_parent(this); // done
    node->set_num_sub_nodes(0);
  }

public:
  owner_t *owner() const { return(_owner); }


  //
  // Functions for pending information
  //
  bool is_pending_empty() const { return(_pending.empty());  }
  bool is_parents_empty() const { return(_parents.empty());  }

  node_t *pop_pending() { 
    node_t *n = (*_pending.begin());
    _pending.pop_front(); 
    return(n);
  }
  void append_pending(node_t *e1) {
    if (e1 != 0) {
      e1->add_ref();
    }
    _pending.push_back(e1);
  }
  void move_pending_from(node_t *e1) { 
    while(!e1->_pending.empty()) {
      _pending.push_back(*(e1->_pending.begin()));
      e1->_pending.erase(0);
      //		      e1->_pending.pop_front());
    }
  }

  node_t *pop_parent() { 
    node_t *n = (*_parents.begin());
    _parents.pop_front(); 
    return(n);
  }
  void append_parent(node_t *e1) { 
    if (e1 != 0) {
      e1->add_ref();
    }
    _parents.push_back(e1); 
  }
  void move_parents_from(node_t *e1) { 
    while(!e1->_parents.empty()) {
      _parents.push_back(*(e1->_parents.begin()));
      e1->_parents.erase(0);
    }
  }

  bool is_canonical() const { return(this->get_parent() == 0); }
  node_t *get_parent() const { return(_parent); }

  // data is only valid at the TOP
  data get_data() const { suif_assert(is_canonical()); return(_data); }
  void set_data(data d) { suif_assert(is_canonical()); _data = d; }

  unsigned get_num_sub_nodes() const { return(_num_sub_nodes); }
  node_t *find_top() const {    // use to NOT modify structure.
    // iterate instead of recurse.
    const node_t *par = this;
    while(par->get_parent() != 0) {
      assert(par != par->get_parent());
      par = par->get_parent();
    }
    return(STATIC_CAST(node_t *,par));
  }
  node_t  *find() { // find_and_move
    node_t *top = this->find_top();
    node_t *the_node = this;
    while(the_node->get_parent() != 0) {
      node_t *old_node = the_node;
      the_node = the_node->get_parent();
      old_node->set_parent(top);
    }
    return(top);
  }



  // The data type MUST have a print function
  void print(ion *out) const; 
  /*
  void print(ion *out) const {
    out->printf("node=%u:\t",this->get_id());
    // How do I dispatch this print method in the template?
    STATIC_CAST(printable *,this->get_data())->print(out);
    out->put_s("\n");
  }
  */
};

// The ID is no longer exported.

template <class data> class union_find_owner {
private:
  typedef union_find_node<data> node_t;
  friend class union_find_node<data>;

  suif_vector<node_t *> _node_list; // not indexable.
  list<unsigned> _free_list; // free nodes.

  node_t  *get_ecr_by_id(unsigned i) const {
    return(_node_list[i]);
  }
  
public:
  union_find_owner() {};
  node_t *new_node(data d) {
    unsigned i = 0;
    if (!_free_list.empty()) {
      i = (*_free_list.begin());
      _free_list.pop_front();
    } else {
      i = _node_list.size();
    }
    node_t *new_t = new node_t(d, i, this);
    while (i >= _node_list.size()) { _node_list.push_back(0); }
    _node_list[i] = new_t;
    //    assert(i+1 == _node_list.size());
    return(new_t);
  }

  void add_ref(node_t *node) {
    node->add_ref();
  }

  unsigned max_node_num() const { return(_node_list.size() + 1); }

  void remove_ref(node_t *node) {
    unsigned id = node->get_id();
    unsigned num_refs = node->num_refs();
    suif_assert_message(num_refs > 1, ("No live references")); // we always have one live ref.
    node->destroy();
    num_refs--;
    if (num_refs == 1) {
      // We'll remove it as well.
      suif_assert(_node_list[id] == node);
      _node_list[id] = 0;
      node->destroy();
      _free_list.push_front(id);
    }
  }

  node_t  *fast_union(node_t *node1,
		      node_t *node2) {
    this->validate();
    node1 = this->find(node1);
    node2 = this->find(node2);
    // re-implemented to keep the first node allocated.
    // undone.
    //    if (node1->get_id() > node2->get_id()) {
    if (node1->get_num_sub_nodes() < node2->get_num_sub_nodes()) {
      node_t *tmp = node1;
      node1 = node2;
      node2 = tmp;
    }
    node1->add_child(node2);
    node1->move_pending_from(node2);
    node1->move_parents_from(node2);
    // remove node 2 from the list
    // @@@ this is NOT a fast algorithm right now
    this->validate();
    return(node1);
  }

  node_t  *find(node_t  *node) {
    this->validate();
    node_t *top = node->find();
    this->validate();
    return(top);
  }

  unsigned get_nodeset_id(const node_t *node) const {
    node_t *top = node->find_top();
    return(top->get_id());
  }

  void validate(node_t *t) {
    /*
    while (!t->is_canonical()) {
      suif_assert(t->get_num_nodes() == 0,
                      "non-zero number of nodes in canonical count");
      suif_assert(t->is_pending_empty(),
		      "non-canonical nodes must have 0 pending");
      suif_assert(t->is_parents_empty(),
		      "non-canonical nodes must have 0 parents");
      
      t = t->get_parent();
    }
    // all nodes must lead to a canonical one.
    suif_assert(t->get_num_nodes() != 0,
                    "0 nodes in a canonical node");
    suif_assert(t->get_num_nodes() <= _node_list.size(),
	            "Too many nodes in a canonical node");
    */
    return;
  }

  void validate() {
    /*
    unsigned i;
    for (i = 0; i< _node_list.size(); i++) {
      node_t *t = _node_list[i];
      this->validate(t);
    }
    */
  }

  // the returned list is owned OUTSIDE
  // a union will invalidate the list...
  /*
  handle_tos<node_t *> get_top_list() {
    slist_tos<node_t *> *tops = new slist_tos<node_t *>;

    unsigned i;
    for (i = 0; i< _node_list.size(); i++) {
      node_t *t = _node_list[i];
      node_t *top = this->find(t); // t->find_top();
      if (t == top) {
	tops->append(t);
      }
    }
    return(tops);
  }
  */

  void print(ion *out) const {
    out->put_s("Dumping the canonical nodes\n");
    out->put_s("---------------------------\n");
    unsigned i;
    for (i = 0; i< _node_list.size(); i++) {
      node_t *t = _node_list[i];
      if (t != 0) {
	node_t *top = t->find();
	if (t == top) {
	  t->print(out);
	}
      }
    }
  }
    
  
};


#endif /* FAST_UNION_H */
