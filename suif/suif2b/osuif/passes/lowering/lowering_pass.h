// $Id: lowering_pass.h,v 1.1.1.1 2000/06/08 00:10:02 afikes Exp $

#ifndef OSUIF_LOWERING__OSUIF_LOWERING_PASS_H
#define OSUIF_LOWERING__OSUIF_LOWERING_PASS_H

#include "suifkernel/suifkernel_forwarders.h"
#include "suifkernel/suif_env.h"
#include "suifkernel/suif_object.h"
#include "suifpasses/suifpasses.h"
#include "basicnodes/basic.h"
#include "suifnodes/suif.h"

#include "osuifutilities/walker_utils.h"
#include "osuifutilities/search_utils.h"
#include "osuifnodes/osuif.h"


// @@@ 2DO:
// - lower all entries in the per_class_symbol_table
// - `lowering-name' annote
// - `lowering-symtab' annote


class LoweringPass : public Pass
{
 private:
  GroupWalker* _gwalker;
  list<CollectWalker* > _walkers;

  bool _lower_instance_methods;
  bool _lower_static_methods;
  bool _lower_static_fields;
  bool _lower_per_class_symbol_table;

  String _symtab;
  String _name_mangling;
  bool _not_use_annote_name;
  bool _verbose;
  
public:
  LoweringPass( SuifEnv* env, const LString &name= "lower_osuif_symbols" ); 
  virtual ~LoweringPass()  { }

  /// Set the default settings of the flags.
  virtual void initialize_flags();

  virtual void append_walkers();
  virtual void append_walker(CollectWalker* walker);

  virtual void process_walkers();
  virtual void process_objects( list<SuifObject* > obj_list );
  virtual void lower_stos(list<SymbolTableObject* > sto_list);

  /// Determine the destination symtab for symbol depending on the flags.
  virtual SymbolTable* destination_symtab( SymbolTableObject* sym );

  /// Determine the mangled name depending on the flags.
  virtual const LString mangled_sto_name( SymbolTableObject* sto );
  
  virtual void initialize();
  virtual bool parse_command_line(TokenStream *command_line_stream);

  Module *clone() const { return (Module *) this; }
  
  virtual void do_file_set_block( FileSetBlock* fsb );

  OptionList* get_command_line() { return _command_line; }

  
  void set_lower_static_fields_flag(bool the_new_flag) {
    _lower_static_fields = the_new_flag;
  }

  void set_lower_instance_methods_flag(bool the_new_flag) {
    _lower_instance_methods = the_new_flag;
  }

  void lower_static_methods_flag(bool the_new_flag) {
    _lower_static_methods = the_new_flag;
  }

};


#endif /* OSUIF_LOWERING__OSUIF_LOWERING_PASS_H */
