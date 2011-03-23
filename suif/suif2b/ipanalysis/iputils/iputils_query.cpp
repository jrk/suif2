#include "iputils_query.h"
#include "iputils_factory.h"
#include "iputils.h"
#include "utils/expression_utils.h"
#include "suifnodes/suif.h"
#include "basicnodes/basic.h"
#include "iokernel/object_wrapper.h"
#include "basicnodes/basic_factory.h"
#include "suifkernel/utilities.h"

const LString k_entry_point("entry_point");
const LString k_call_target("call_target");

IPUtilsQuery::IPUtilsQuery() {}

Iter<ProcedureSymbol*>
IPUtilsQuery::get_callees(CallStatement *call) {
  CallTargetAnnote *ann = to<CallTargetAnnote >(call->peek_annote(k_call_target));

  if(ann == NULL){
    ann = create_call_target_annote(call->get_suif_env(), k_call_target);
    ProcedureSymbol* sym = get_procedure_target_from_call(call);
    if (!sym) {
      return(Iter<ProcedureSymbol*>(new EmptyIterator()));
    }else{
      ann->append_target(sym);
    }
    call->append_annote(ann);
  }
  
  suif_assert(ann);
  return ann->get_target_iterator();
};

bool IPUtilsQuery::is_unique_entry_point(ProcedureSymbol* proc_sym){
    if(proc_sym->peek_annote(k_entry_point) == NULL){
        bool _is_main =
            proc_sym->get_name()==LString("main") ||
            proc_sym->get_name()==LString("__MAIN") ||
            proc_sym->get_name()==LString("MAIN_");

        if(_is_main){
            GeneralAnnote *an = create_general_annote(
                proc_sym->get_suif_env(), k_entry_point);
            proc_sym->append_annote(an);
        }
        return _is_main;
    }else{
        return true;
    }
};

ProcedureSymbol* IPUtilsQuery::get_unique_entry_point(const FileSetBlock* fsb){
    for (Iter<ProcedureDefinition> iter = object_iterator<ProcedureDefinition>(fsb);
        iter.is_valid(); iter.next())
    {
        ProcedureDefinition* proc_def = &iter.current();
        ProcedureSymbol* proc_sym = proc_def->get_procedure_symbol();

        if(IPUtilsQuery::is_unique_entry_point(proc_sym)){
            return proc_sym;
        }
    }
    //Panic?
    return NULL;
};
