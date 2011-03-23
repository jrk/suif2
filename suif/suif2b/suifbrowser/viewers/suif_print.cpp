#include "suif_print.h"
#include "suif_vnode.h"
#include "base_viewer.h"
#include "suifkernel/module.h"
#include "suifprinter/suifprinter.h"
#include "suifnodes/suif.h"

extern SuifPrinterModule *spm;

LString *formater::tag_list[] = {
     new LString("/scope"),
     new LString("/while"),
     new LString("/for"),
     new LString("/if"),
     0 };

const LString *formater::mark_filter[] = {
    new LString("/mrk"),
    new LString("/mrk_inst"),
    0 };

const LString *formater::null_filter[] = {
    0 };

formater::formater(SuifEnv *suif, vtty * text, int depth, int detail) :
  default_zot_print_helper(suif),SelectiveWalker(suif, SuifObject::get_class_name())
{
  _depth = depth;
  _detail = detail;
  _text = text;
  _first = true;
  _want_tag = false;
  _current_filter = null_filter;
}
	

void formater::print_zot(const SuifObject *the_zot, fstream& the_ion) {
  if ( (_depth==-1) || (_depth && _depth--) ) {
    add_indentation( 4 );
    if (the_zot == 0)
      {
        print_zot_prefix(0, the_ion);
        the_ion <<"<<0>>\n";
      }
    else
      {
	//       apply ( (SuifObject*)the_zot );
	if ( prepend_tag( the_zot ) ) {
          handle_zot_with_tag( the_zot );
        } else if ( !filter_tag( the_zot ) ) {
          vnode *vn = make_tag_begin( the_zot );
          print(the_ion, the_zot);
          //the_zot->print(the_ion);
          make_tag_end( vn );
        }
      }
    remove_indentation( 4 );
  }
}


void formater::print_zot_ref( const SuifObject *the_zot, fstream& the_ion) {
  if ( (the_zot) && ((_depth==-1) || (_depth && _depth--) ) ) {
    // create_vnode must be mutually exclusive with the X_zot's that print folds
    bool create_vnode = !prepend_tag( the_zot );
    vnode* vn;
    if ( create_vnode ) vn = make_tag_begin( the_zot );
    default_zot_print_helper::print_zot_ref( the_zot, the_ion );
    if ( create_vnode ) make_tag_end( vn );
  }
}


void formater::print_zot_prefix(const SuifObject *the_zot, fstream& the_ion) {
  inherited::print_zot_prefix( the_zot, the_ion );
}

vnode* formater::make_tag_begin( const SuifObject* the_zot, bool want_fold ) {
   if ( _first ) {
      _first = false;
      _text->fout() << "   ";
      return 0;
   }
   vnode* vn = create_vnode( (SuifObject*)the_zot );
  
   if ( want_fold ) {
     _text->tag_begin(vn, (print_fn)&formater::print_helper, get_depth(), 0);
   } else {
     _text->tag_begin( vn );
   }
   _text->fout() << "   ";
   return vn;
}

void formater::make_tag_end( vnode* vn ) {
  if ( vn ) {
    _text->tag_end( vn );
  }
}


void  formater::handle_zot(SuifObject * the_zot) {
    vnode *vn = make_tag_begin( the_zot );
    //the_zot->print(_text->fout());
    print(_text->fout(), the_zot);
    make_tag_end( vn );
}


void formater::handle_for_statement(ForStatement* the_zot) { 
  handle_zot_with_tag( the_zot );
}


void formater::handle_scope_statement(ScopeStatement* the_zot) {
  handle_zot_with_tag( the_zot );
}


void formater::handle_while_statement(WhileStatement* the_zot) {
  handle_zot_with_tag( the_zot );
}


void formater::handle_zot_with_tag( const SuifObject* the_zot) {
  vnode* vn = make_tag_begin( the_zot, true );
  if ( _detail == PRINT_FULL ) {   
     _detail = PRINT_BRIEF;
     //the_zot->print(_text->fout());
     print(_text->fout(), the_zot);
     _detail = PRINT_FULL;
    } else {
     int depth = _depth;
     _depth = 0;
     //the_zot->print(_text->fout());
     print(_text->fout(), the_zot);
     _depth = depth;
    }
  make_tag_end( vn );
}



void formater::print_helper( vtty* text, vnode* vn,
			     int depth, int detail,
			     void* pr) {
  formater f(suif_env, text, depth, detail );
  f.print_zot( (SuifObject*)vn->get_object(), text->fout());
}

bool formater::prepend_tag( const SuifObject* z ) {
  LString tag = z->get_meta_class()->get_class_name();
  int i = 0;
  for ( LString* search = tag_list[0]; search; search = tag_list[++i] ) {
    if ( *search == tag ) return true;
  }
  return false;
}


bool formater::filter_tag( const SuifObject* z ) {
  LString tag = z->get_meta_class()->get_class_name();
  int i = 0;
  for ( const LString* search = _current_filter[0]; search; search = _current_filter[++i] ) {
        if ( *(LString*) /*@@@*/ search == tag ) return true;
    //    if ( !strcmp( search->chars(), tag.chars() ) ) return true;
  }
  return false;
}

bool formater::start_of_object(
	ostream& output, 
	const ObjectWrapper &obj,
	int deref) {
    if (deref > 20)
	return false;
    Object *object = obj.get_object();
    if (!is_kind_of<SuifObject>(object)) {
	vnode_list.push_back(0);
	return true;
	}
    vnode *vn = make_tag_begin(to<SuifObject>(object),false);
    vnode_list.push_back(vn);
    return true;
    }

void formater::end_of_object(ostream& output, const ObjectWrapper &obj) {
    vnode *vn = vnode_list.back();
    vnode_list.pop_back();
    if (vn)
    	make_tag_end(vn);
    }

