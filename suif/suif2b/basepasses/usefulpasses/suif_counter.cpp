#include "suif_counter.h"
#include "iokernel/object_factory.h"
#include "suifkernel/suif_exception.h"
#include "suifkernel/suif_object.h"

NodeCounter::NodeCounter(const MetaClass *mc) :
  _meta_class(mc),
  _parent(NULL),
  _children(),
  _direct_count(0)
{
}

NodeCounter::~NodeCounter(void)
{
  while (!_children.empty()) {
    NodeCounter *b = _children.back();
    _children.pop_back();
    delete b;
  }
}


MetaClass* NodeCounter::get_meta_class(void) const
{
  return (MetaClass*)_meta_class;
}


void NodeCounter::set_parent(NodeCounter* new_parent)
{
  if (_parent == NULL) {
    _parent = new_parent;
    return;
  }
  if (_parent == new_parent)
    return;
  SUIF_THROW(SuifDevException(__FILE__, __LINE__,
			      String("A NodeCounter for ") +
			      _meta_class->get_instance_name() +
			      " already has parent " +
			      _parent->_meta_class->get_instance_name() +
			      " cannot be re-parent to " +
			      new_parent->_meta_class->get_instance_name()));
}


unsigned NodeCounter::get_direct_count(void) const
{
  return _direct_count;
}


unsigned NodeCounter::get_indirect_count(void) const
{
  unsigned cnt = 0;
  for (unsigned childid = 0; childid < _children.size(); childid++) {
    NodeCounter *child = _children[childid];
    cnt += child->get_direct_count() + child->get_indirect_count();
  }
  return cnt;
}


void NodeCounter::add_child(NodeCounter* child)
{
  _children.push_back(child);
  child->set_parent(this);
}


unsigned NodeCounter::get_child_count(void) const
{
  return _children.size();
}

NodeCounter *NodeCounter::get_nth_child(unsigned n) const
{
  return _children[n];
}

unsigned NodeCounter::add_direct_count(void)
{
  _direct_count++;
  return _direct_count;
}









SuifCounter::SuifCounter(SuifEnv* env) :
  AllWalker(env),
  _root_counter(new NodeCounter(env->get_object_factory()->
				find_meta_class(SuifObject::get_class_name()))),
  _NodeCounter_map()
{
  _NodeCounter_map.enter_value(env->get_object_factory()->
			       find_meta_class(SuifObject::get_class_name()),
			       _root_counter);
}



// pre-condition : no NodeCounter for mc exists yet.
// sife effect   : NodeCounter for all ancestors of mc will be created also.
//
NodeCounter *SuifCounter::add_node_counter(const MetaClass* mc)
{
  NodeCounter *new_nc = new NodeCounter(mc);
  MetaClass *parent_mc = mc->get_link_meta_class();
  NodeCounter *parent_nc = get_node_counter(parent_mc);
  parent_nc->add_child(new_nc);
  _NodeCounter_map.enter_value(mc, new_nc);
  return new_nc;
}




Walker::ApplyStatus SuifCounter::operator()(SuifObject *obj)
{
  if (obj == NULL) return Walker::Continue;
  const MetaClass *mc = obj->get_meta_class();
  NodeCounter *nc = get_node_counter(mc);
  nc->add_direct_count();
  return Walker::Continue;
}


void SuifCounter::count(SuifObject *obj)
{
  if (obj == NULL) return;
  obj->walk(*this);
}


NodeCounter* SuifCounter::get_node_counter(const MetaClass *mc)
{
  suif_map<const MetaClass*,NodeCounter*>::iterator it =
    _NodeCounter_map.find(mc);
  if (it != _NodeCounter_map.end())
    return (*it).second;
  return add_node_counter(mc);
}



static void print_result_gut(NodeCounter *nc, unsigned indent, ostream& out)
{
  for (unsigned i = 0; i < indent; i++ ) {
    out << "  ";
  }
  out << nc->get_meta_class()->get_instance_name();
  out << " : ";
  out << nc->get_direct_count() + nc->get_indirect_count();
  out << endl;
  {for (unsigned i = 0; i < nc->get_child_count(); i++) {
    print_result_gut(nc->get_nth_child(i), indent+1, out);
  }}
}



ostream& SuifCounter::print_result(ostream& out) const
{
  print_result_gut(_root_counter, 0, out);
  return out;
}
