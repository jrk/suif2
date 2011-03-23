
#include "decl_groups.h"
#include "ansic.h"
#include "utils/symbol_utils.h"
#include "utils/print_utils.h"
#include "suifkernel/suif_env.h"
#include "suifprinter/suifprinter.h"

static LString k_linksuif_temp_filename("linksuif_temp_filename");

DeclGroups::DeclGroup::DeclGroup(SymbolTableObject* head, SuifEnv *se) :
  _suif_env(se),
  _head_symbol(head),
  _folks()
{
}


template<class T>
T*
get_owner_of_type(SuifObject *obj)
{
  if (!obj)
    return 0;

  obj = obj->get_parent();

  while (obj) {
    if ( obj->isKindOf( T::get_class_name() ) ) {
      return to<T>(obj);
    }
    obj = obj->get_parent();
  }

  return 0;
}


String source_file(SymbolTableObject *so)
{
  String message;

  Annote *annote;
  BrickAnnote *brickannote;
  SuifBrick *brick;
  StringBrick *stringbrick;

  annote = so->lookup_annote_by_name(k_linksuif_temp_filename); 
  if (annote) {
    brickannote = to<BrickAnnote>(annote);
    brick = brickannote->get_brick(0);
    stringbrick = to<StringBrick>(brick);
    
    message += " from ";
    message += stringbrick->get_value();
  }
  else {
    message += " (unknown origin)";
  }

  return message;
}



// return number of messages added to mbuf
//
int DeclGroups::DeclGroup::check(DeclGroups::DeclGroup *g,
				 AndMessageBuffer *mbuf,
				 SuifEnv* suif_env)
{
  Annote *annote;
  BrickAnnote *brickannote;
  SuifBrick *brick;
  StringBrick *stringbrick;

  if (g->_head_symbol->get_name().length() == 0)
    return 0;
  if (g->_head_symbol->get_name() != _head_symbol->get_name())
    return 0;
  if ((is_kind_of<Symbol>(g->_head_symbol) &&
       is_kind_of<Symbol>(_head_symbol)) ||
      (is_kind_of<Type>(g->_head_symbol) &&
       is_kind_of<Type>(_head_symbol))) {
    if (mbuf != NULL) {
      String message("linksuif error: name crash [");
      message += _head_symbol->get_name() +
	String("] between symbols\n    ") + to_id_string(g->_head_symbol);

      annote = g->_head_symbol->lookup_annote_by_name(k_linksuif_temp_filename); 
      if (annote) {
	brickannote = to<BrickAnnote>(annote);
	brick = brickannote->get_brick(0);
	stringbrick = to<StringBrick>(brick);
  
	message += " from ";
	message += stringbrick->get_value();
      }

      // PrintSubSystem *printer = suif_env->get_print_subsystem();

      // message += " (";
      // message += printer->print_to_string(g->_head_symbol);
      // message += ") ";
      
      message += " and \n    ";
      message += to_id_string(_head_symbol);

      annote = _head_symbol->lookup_annote_by_name(k_linksuif_temp_filename); 
      if (annote) {
	brickannote = to<BrickAnnote>(annote);
	brick = brickannote->get_brick(0);
	stringbrick = to<StringBrick>(brick);
  
	message += " from ";
	message += stringbrick->get_value();
      }

      // message += " (";
      // message += printer->print_to_string(_head_symbol);
      // message += ")";
      
      mbuf->add_message(message);
    }
    return 1;
  }
  return 0;
}


int DeclGroups::DeclGroup::add_to_replace_map(ReplaceMap* map)
{
  if (!is_kind_of<Symbol>(_head_symbol))
    return 0;
  int cnt = 0;
  for (suif_vector<SymbolTableObject*>::iterator it = _folks.begin();
       it != _folks.end();
       it++) {
    map->add_entry((*it), _head_symbol);
    cnt++;
  }
  return cnt;
}

ostream& DeclGroups::DeclGroup::print(ostream& out)
{
  out << to_id_string(_head_symbol) << " : ";
  for (suif_vector<SymbolTableObject*>::iterator it = _folks.begin();
       it != _folks.end();
       it++) {
    out << to_id_string((*it));
  }
  return out;
}

/*
 * This is used for some non-ANSI compatible definitions
 */

static bool is_undefined_definition(Symbol *sym) 
{
  if (!is_kind_of<VariableSymbol>(sym)) return false;
  VariableSymbol *var = to<VariableSymbol>(sym);
  if (var->get_definition() == NULL) return false;
  VariableDefinition *vardef = var->get_definition();
  ValueBlock *valblock = vardef->get_initialization();
  if (valblock == NULL) return(true);
  if (is_kind_of<UndefinedValueBlock>(valblock))
    return(true);
  return(false);
}

bool DeclGroups::DeclGroup::add_symbol(Symbol* newsym)
{
  ANSIC ansi;

  if (!is_kind_of<Symbol>(_head_symbol)) return false;
  Symbol* head = to<Symbol>(_head_symbol);

  if (!ansi.is_compatible_type(head->get_type(), newsym->get_type())) {
    cerr << ansi.get_warnings().c_str();
    cerr << "linksuif error: incompatible types for symbol "
	 << head->get_name().c_str()
	 << "\n    ";
    String source = source_file(head);
    cerr << source.c_str() << " and";
    source = source_file(newsym);
    cerr << source.c_str() << "\n";
    return false;
  }

  bool switch_head = false;
  if (is_definition_symbol(head) && is_definition_symbol(newsym)) {
    if (!is_undefined_definition(newsym) && !is_undefined_definition(head)) {
      cerr << "linksuif error: multiple definitions of symbol "
           << head->get_name().c_str()
           << "\n    ";
      String source = source_file(head);
      cerr << source.c_str() << " and";
      source = source_file(newsym);
      cerr << source.c_str() << "\n";
      return false;
    }
    if (is_undefined_definition(newsym)) {
      switch_head = true;
    }
  } else {
    if (is_definition_symbol(newsym)) {
      switch_head = true;
    }
  }
  
  if (switch_head) {
    _folks.push_back(_head_symbol);
    _head_symbol = newsym;
    newsym->set_is_address_taken(newsym->get_is_address_taken() ||
				 head->get_is_address_taken());
  } else {
    _folks.push_back(newsym);
    head ->set_is_address_taken(newsym->get_is_address_taken() ||
				head->get_is_address_taken());
  }
  return true;
}

bool DeclGroups::DeclGroup::add_group(GroupType *newgroup)
{
  ANSIC ansi;

  if (!is_kind_of<GroupType>(_head_symbol)) return false;
  GroupType *head = to<GroupType>(_head_symbol);
  if (!ansi.is_compatible_type(head, newgroup))
    return false;
  if (newgroup->get_is_complete()) {
    _folks.push_back(_head_symbol);
    _head_symbol = newgroup;
  } else {
    _folks.push_back(newgroup);
  }
  return true;
}


bool DeclGroups::DeclGroup::add_enumerated(EnumeratedType *etype)
{
  ANSIC ansi;

  if (!is_kind_of<EnumeratedType>(_head_symbol)) return false;
  EnumeratedType* head = to<EnumeratedType>(_head_symbol);
  if (!ansi.is_compatible_type(head, etype))
    return false;
  _folks.push_back(etype);
  return true;
}


bool DeclGroups::DeclGroup::add(SymbolTableObject* newobj)
{
  if (_head_symbol->get_name() != newobj->get_name())
    return false;
  if (is_kind_of<Symbol>(newobj))
    return add_symbol(to<Symbol>(newobj));
  if (is_kind_of<GroupType>(newobj))
    return add_group(to<GroupType>(newobj));
  if (is_kind_of<EnumeratedType>(newobj))
    return add_enumerated(to<EnumeratedType>(newobj));
  return false;
}



// DeclGroups
  

DeclGroups::DeclGroups(SuifEnv *se) :
  _groups(),
  _suif_env(se)
{
}

DeclGroups::~DeclGroups(void)
{
  for(suif_hash_map<LString,suif_vector<DeclGroup*>* >::iterator iter =
	_groups.begin();
      iter != _groups.end();
      iter++ ) {
    suif_vector<DeclGroup*>* g = (*iter).second;
    while (!g->empty()) {
      delete g->back();
      g->pop_back();
    };
    delete g;
  }
}

void DeclGroups::add(SymbolTableObject *sym)
{
  LString name = sym->get_name();
  suif_hash_map<LString,suif_vector<DeclGroup*>*>::iterator p =
    _groups.find(name);
  if (p == _groups.end()) {
    suif_vector<DeclGroup*> *g = new suif_vector<DeclGroup*>();
    g->push_back(new DeclGroup(sym, _suif_env));
    _groups.enter_value(name, g);
    return;
  }
  suif_vector<DeclGroup*>* g = (*p).second;
  for (suif_vector<DeclGroup*>::iterator it = g->begin();
       it != g->end();
       it++) {
    if ((*it)->add(sym))
      return;
  }
  g->push_back(new DeclGroup(sym, _suif_env));
}


int DeclGroups::check(AndMessageBuffer* mbuf, SuifEnv *suif_env)
{
  int cnt = 0;
  for (suif_hash_map<LString,suif_vector<DeclGroup*>* >::iterator giter =
	 _groups.begin();
       giter != _groups.end();
       giter++) {
    suif_vector<DeclGroup*> *g = (*giter).second;
    for (unsigned i = 0; i<g->length(); i++)
      for (unsigned j = i+1; j<g->length(); j++)
	cnt += g->at(i)->check(g->at(j), mbuf, suif_env);
  }
  return cnt;
}


int DeclGroups::add_to_replace_map(ReplaceMap* map)
{
  int cnt = 0;
  for (suif_hash_map<LString,suif_vector<DeclGroup*>* >::iterator giter =
	 _groups.begin();
       giter != _groups.end();
       giter++) {
    suif_vector<DeclGroup*> *g = (*giter).second;
    for (unsigned i = 0; i<g->length(); i++)
      cnt += g->at(i)->add_to_replace_map(map);
  }
  return cnt;
}


ostream& DeclGroups::print(ostream& out)
{
  out << "DeclGroups: {" << endl;
  for (suif_hash_map<LString,suif_vector<DeclGroup*>* >::iterator giter =
	 _groups.begin();
       giter != _groups.end();
       giter++) {
    out << (*giter).first << ": ";
    suif_vector<DeclGroup*> *g = (*giter).second;
    for (unsigned i = 0; i<g->length(); i++)
      g->at(i)->print(out);
    out << endl;
  }
  out << "}" << endl;
  return out;
}
