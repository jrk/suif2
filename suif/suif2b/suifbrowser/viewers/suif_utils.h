/*
 * suif_utils.h
 */

#ifndef SUIF_UTILS_H
#define SUIF_UTILS_H
class vnode;

#include <fstream.h>
#include "basicnodes/basic_forwarders.h"
#include "suifnodes/suif_forwarders.h"
#include "common/suif_list.h"
#include "common/i_integer.h"
#include "common/formatted.h"
#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_walker.h"

#include "suifprinter/suifprinter.h"

list<ProcedureSymbol*> *
get_procedure_list( FileSetBlock *file_set, 
                    bool flag_export=true, 
                    bool flag_static=true, 
                    bool flag_nested=false, 
                    bool flag_defs_only = true );


ProcedureDefinition* get_procedure_definition( SuifObject* node );
bool is_same_procedure( ProcedureDefinition* p1, SuifObject* p2 );
FileBlock* get_file_block( SuifObject* node );
SuifObject* last_zot( SuifObject* z );
IInteger get_line_number(Annote* z );
String get_file_name( Annote* z );

class stopping_suif_visitor : public SuifWalker {
protected:
  bool stop;
public:
  stopping_suif_visitor(SuifEnv *suif_env):SuifWalker(suif_env) { stop=false; }
  virtual bool stop_iterating() const { return stop; }
  ApplyStatus operator() (SuifObject *zot) { return Continue; }

};

bool iterate_over(SuifObject *start_zot, 
                  stopping_suif_visitor *vis, 
                  s_count_t start_comp = 0, 
                  bool do_root = true );

bool iterate_next( SuifObject *start_zot, 
                      stopping_suif_visitor *vis, 
                      SuifObject *stop_at=0, 
                      s_count_t start_comp = 0 );

bool iterate_right_to_left( SuifObject *start_zot, 
                               stopping_suif_visitor *vis, 
                               s_count_t start_comp = 0, 
                               bool do_root = true );

bool iterate_previous( SuifObject *start_zot, 
                          stopping_suif_visitor *vis, 
                          SuifObject *stop_at=0, 
                          bool check_self = false );

FileBlock *get_file_block( SuifObject* node );

IInteger find_source_line( SuifObject *tn, String &file_name );

SuifObject *map_line_to_tree_node( SuifObject *root, int line );

FileBlock *find_file_block( String src_file);

class suif_utils {
public:
  static String  get_path( FileBlock* fb );
  static bool is_absolute( String file );
  static bool is_same_file( String name1, String name2 );
  static String get_source_file( FileBlock* fb );
  static SuifObject* get_zot( vnode* vn );
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Copied from suifx/zot_class.h: Should be appropriately replaced later.
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//	We cheat here. We should be implementing this as a moudle but
// 	for now we do not do that - we treat it as a class and (elsewhere)
//	we init the module we derive from, not this one
class zot_print_helper : public SuifPrinterModule
  {
protected:
    zot_print_helper(SuifEnv *env) : SuifPrinterModule(env)  {SuifPrinterModule::init(); }

public:
    virtual ~zot_print_helper(void)  { }

    virtual void print_zot(const SuifObject *the_zot, fstream&the_ion) = 0;
    virtual void print_zot_ref(const SuifObject *the_zot, fstream&the_ion) = 0;
    virtual int indentation_level(void) const = 0;
    virtual void add_indentation(int additional_amount) = 0;
    virtual void remove_indentation(int removed_amount) = 0;
    virtual void print_zot_prefix(const SuifObject *the_zot, fstream&the_ion) = 0;
    virtual void print_zot_continuation_prefix(const SuifObject *the_zot,
                                               fstream&the_ion) = 0;
    FormattedText formatted_text;

  };

class default_zot_print_helper : public zot_print_helper
  {
private:
    int _current_indentation;

public:
    default_zot_print_helper(SuifEnv *env) : zot_print_helper(env),_current_indentation(0)  { }
    virtual ~default_zot_print_helper(void)  { }

    void print_zot(const SuifObject *the_zot, fstream&the_ion);
    void print_zot_ref(const SuifObject *the_zot, fstream&the_ion);
    int indentation_level(void) const  { return _current_indentation; }
    void add_indentation(int additional_amount);
    void remove_indentation(int removed_amount);
    void print_zot_prefix(const SuifObject *the_zot, fstream&the_ion);
    void print_zot_continuation_prefix(const SuifObject *the_zot, fstream&the_ion);
  };

#endif // SUIF_UTILS_H
