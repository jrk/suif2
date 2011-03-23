// $Id: osuif_visitor_info.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "iokernel/object_factory.h"
#include "osuifprint/osuif_visitor_info.h"

OsuifVisitorInfo::OsuifVisitorInfo(PrintingMaps *map): printingMaps(map){}

void OsuifVisitorInfo::get_object_address(char *result, Address obj){
  sprintf(result, "[0x%08X]", (int)obj);
}

void OsuifVisitorInfo::do_method_type(Address vis, MethodType* method_type){
  ((OsuifVisitorInfo *)vis)->print_method_type((MethodType*)method_type, cout,
                                               false);
}

void OsuifVisitorInfo::print_method_type(MethodType* method_type, 
                                         ostream& output, bool blnStatic){
  int indent, tempIndent;
  int count = 1;

  indent = printingMaps->getIndent();

  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << " ";
  Type* resultType = method_type->get_result_type();
  
  //print out the result type and method name
  if(blnStatic)
    output << "static "; 
  output << method_type->get_name();
  output << "(";

  //print out the arguments
  Iter<QualifiedType*> argumentIter = method_type->get_argument_iterator();

  //go past the first argument
  if(!blnStatic)
    argumentIter.next();
  count = 1;
  while(argumentIter.is_valid()){
    if(count != 1) output << ",";
    QualifiedType *argument = (QualifiedType*)argumentIter.current();
    const MetaClass *mc = argument->get_meta_class();
    output << mc->get_instance_name() << endl;
    argumentIter.next();
    count++;
  }//end while
  output << ");" << endl;
}//end function print_method_type

void OsuifVisitorInfo::do_instance_method_type(Address vis,
                           InstanceMethodType* instance_method_type){
  ((OsuifVisitorInfo *)vis)->
     print_instance_method_type((InstanceMethodType*)instance_method_type, 
                                cout);
}

void OsuifVisitorInfo::print_instance_method_type(
                          InstanceMethodType* instance_method_type,
                           ostream& output){
  print_method_type(instance_method_type, cout, false);
}

void OsuifVisitorInfo::do_static_method_type(Address vis, 
                                     StaticMethodType* static_method_type){
  ((OsuifVisitorInfo *)vis)->
    print_static_method_type((StaticMethodType*)static_method_type, cout);
}

void OsuifVisitorInfo::print_static_method_type(
                                   StaticMethodType* static_method_type, 
                                   ostream& output){

  print_method_type(static_method_type, cout, true);
}//end function

void OsuifVisitorInfo::do_instance_method_symbol(Address vis,
                                 InstanceMethodSymbol* instance_method_symbol){
  ((OsuifVisitorInfo *)vis)->
    print_instance_method_symbol((InstanceMethodSymbol*)instance_method_symbol,
                                  cout);
}

void OsuifVisitorInfo::print_instance_method_symbol(
                                    InstanceMethodSymbol* instanceMethodSymbol, 
                                    ostream& output){
  int tempIndent, indent, count;
  indent = printingMaps->getIndent();

  for(tempIndent =0; tempIndent< indent; tempIndent++) output << ' ';
  Iter<LString> attrIter = instanceMethodSymbol->get_attribute_iterator();
  
  while(attrIter.is_valid()){
    output << (LString)attrIter.current() << " ";
    attrIter.next();
  }

  //print method type details
  MethodType* method_type = (MethodType*)instanceMethodSymbol->get_type();

  Type *resultType = method_type->get_result_type();
  output << "(";

  const MetaClass *mc = resultType->get_meta_class();

  output << mc->get_instance_name();
  
  output << ") ";

  output << instanceMethodSymbol->get_name();
  output << "(";

  Iter<QualifiedType*> argumentIter = method_type->get_argument_iterator();

  //go past the first argument
  argumentIter.next();
  count = 1;
  
  while(argumentIter.is_valid()){
    if(count != 1) output << ",";
    QualifiedType *argument = (QualifiedType*)argumentIter.current();
    const MetaClass *mc = argument->get_meta_class();

    output << mc->get_instance_name();
    argumentIter.next();
    count++;
  }/*end while*/

  output << ");" << endl;
}/*instance method symbol*/

void OsuifVisitorInfo::do_instance_field_symbol(Address vis, 
                             InstanceFieldSymbol* instance_field_symbol){
  ((OsuifVisitorInfo *)vis)->print_instance_field_symbol(
                          (InstanceFieldSymbol*)instance_field_symbol, cout);
}

void OsuifVisitorInfo::print_instance_field_symbol(
                InstanceFieldSymbol* instanceFieldSymbol, ostream& output){

  int indent, tempIndent;

  indent = printingMaps->getIndent();

  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';

  Iter<LString> iterAttr = instanceFieldSymbol->get_attribute_iterator();
  while(iterAttr.is_valid()){
    output << iterAttr.current();
    iterAttr.next();
  }/*end while*/

  Type *type = instanceFieldSymbol->get_type();
  const MetaClass *mc = type->get_meta_class();
  output << mc->get_instance_name() << " " << instanceFieldSymbol->get_name();
}

void OsuifVisitorInfo::do_static_field_symbol(Address vis, 
                             StaticFieldSymbol* static_field_symbol){
  ((OsuifVisitorInfo *)vis)->print_static_field_symbol(
                          (StaticFieldSymbol*)static_field_symbol, cout);
}

void OsuifVisitorInfo::print_static_field_symbol(
                StaticFieldSymbol* staticFieldSymbol, ostream& output){

  int indent, tempIndent;

  indent = printingMaps->getIndent();

  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';

  Iter<LString> iterAttr = staticFieldSymbol->get_attribute_iterator();
  while(iterAttr.is_valid()){
    output << iterAttr.current();
    iterAttr.next();
  }//end while

  Type *type = staticFieldSymbol->get_type();
  const MetaClass *mc = type->get_meta_class();
  output << " static ";
  output << mc->get_instance_name() << " " 
         << staticFieldSymbol->get_name() << ";" ;
  output << endl;
}/*print static field symbol*/

void OsuifVisitorInfo::do_static_method_symbol(Address vis, 
                          StaticMethodSymbol* static_method_symbol){
  ((OsuifVisitorInfo *)vis)->print_static_method_symbol(
                          (StaticMethodSymbol*)static_method_symbol, cout);
}

void OsuifVisitorInfo::print_static_method_symbol(
                StaticMethodSymbol* staticMethodSymbol, ostream& output){
  int tempIndent, indent, count;
  indent = printingMaps->getIndent();

  for(tempIndent =0; tempIndent< indent; tempIndent++) output << ' ';
  Iter<LString> attrIter = staticMethodSymbol->get_attribute_iterator();
  
  output << "static " << endl;
  while(attrIter.is_valid()){
    output << (LString)attrIter.current() << " ";
    attrIter.next();
  }

  //print method type details
  MethodType* method_type = (MethodType*)staticMethodSymbol->get_type();

  Type *resultType = method_type->get_result_type();
  output << "(";

  const MetaClass *mc = resultType->get_meta_class();

  output << mc->get_instance_name();
  
  output << ") ";

  output << staticMethodSymbol->get_name();
  output << "(";

  Iter<QualifiedType*> argumentIter = method_type->get_argument_iterator();

  //go past the first argument
  count = 1;
  
  while(argumentIter.is_valid()){
    if(count != 1) output << ",";
    QualifiedType *argument = (QualifiedType*)argumentIter.current();
    const MetaClass *mc = argument->get_meta_class();

    output << mc->get_instance_name();
    argumentIter.next();
    count++;
  }//end while

  output << ");" << endl;
}

void OsuifVisitorInfo::do_class_type(Address vis, ClassType* class_type){
  ((OsuifVisitorInfo *)vis)->print_class_type((ClassType*)class_type, cout);
}


void OsuifVisitorInfo::print_class_type(ClassType* class_type, ostream& output){
  int indent, tempIndent;
  
  indent = printingMaps->getIndent();

  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';

  //print any attributes of the class
  Iter<LString> attrIterator = class_type->get_attribute_iterator();
  while(attrIterator.is_valid()){
    output << (LString)attrIterator.current() << " ";
    attrIterator.next();
  }
  output << "class ";
  
  //print class name
  output << class_type->get_name();
  output << "{" << endl;

  printingMaps->incrementIndent();
  indent = printingMaps->getIndent();

  //print instance field symbols of the class
  Iter<SymbolTableObject*> instanceFieldIter =
    class_type->get_group_symbol_table()->get_symbol_table_object_iterator();

  while(instanceFieldIter.is_valid()){
    InstanceFieldSymbol *ifs = 
            (InstanceFieldSymbol*)instanceFieldIter.current();
    printingMaps->process_a_suif_object(ifs);
    instanceFieldIter.next();
  }/*end while*/

  Iter<SymbolTableObject*> instanceMethodIter =
    class_type->get_instance_method_symbol_table()->
                get_symbol_table_object_iterator();
  
  while(instanceMethodIter.is_valid()){
    InstanceMethodSymbol *ms = 
         (InstanceMethodSymbol*)instanceMethodIter.current();
    printingMaps->process_a_suif_object(ms);
    instanceMethodIter.next();
  }

  //print class methods and fields
  Iter<SymbolTableObject*> classSymbolIter =
   class_type->get_per_class_symbol_table()->get_symbol_table_object_iterator();

  while(classSymbolIter.is_valid()){
    SymbolTableObject *sto = (SymbolTableObject*)classSymbolIter.current();
    if(sto->isA(StaticMethodSymbol::get_class_name())){
      StaticMethodSymbol *staticMethod = (StaticMethodSymbol*)sto;
      printingMaps->process_a_suif_object(staticMethod);
    }
    else if(sto->isA(StaticFieldSymbol::get_class_name())){
      StaticFieldSymbol *staticField = (StaticFieldSymbol*)sto;
      printingMaps->process_a_suif_object(staticField);
    }
    classSymbolIter.next();
  }/*end while*/

  printingMaps->decrementIndent();
  indent = printingMaps->getIndent();
  for(tempIndent =0; tempIndent < indent; tempIndent++) output << ' ';
  output << "}";
  output << endl;
}//end function

void OsuifVisitorInfo::do_inheritance_link(Address vis,
                                   InheritanceLink* inheritance_link){
  ((OsuifVisitorInfo *)vis)->
        print_inheritance_link((InheritanceLink *)inheritance_link, cout);
}

void OsuifVisitorInfo::print_inheritance_link(InheritanceLink* inheritanceLink,
                                           ostream& output){
  output << "InheritanceLink" << endl;
}

void OsuifVisitorInfo::do_static_method_call_statement(Address vis,
                             StaticMethodCallStatement* method_call_statement){
  ((OsuifVisitorInfo *)vis)->
    print_static_method_call_statement(
                  (StaticMethodCallStatement*)method_call_statement,
                  cout);
}

void OsuifVisitorInfo::print_static_method_call_statement(
              StaticMethodCallStatement *staticMethodCallStatement, 
              ostream& output){
  int indent, tempIndent, count;
  indent = printingMaps->getIndent();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';
  StaticMethodSymbol *targetSymbol = 
          staticMethodCallStatement->get_target_method();
  const ClassType *classType = targetSymbol->get_owning_class();
  output << classType->get_name() << "::" << targetSymbol->get_name();
  output << "(";
  MethodType* method_type = (MethodType*)targetSymbol->get_type();
  Iter<QualifiedType*> argumentIter = method_type->get_argument_iterator();

  //go past the first argument
  argumentIter.next();
  count = 1;
  
  while(argumentIter.is_valid()){
    if(count != 1) output << ",";
    QualifiedType *argument = (QualifiedType*)argumentIter.current();
    const MetaClass *mc = argument->get_meta_class();

    output << mc->get_instance_name();
    argumentIter.next();
    count++;
  }/*end while*/

  output << ");" << endl;
}/*StaticMethodCallStatement*/

void OsuifVisitorInfo::do_instance_method_call_statement(Address vis,
                          InstanceMethodCallStatement* method_call_statement){
  ((OsuifVisitorInfo *)vis)->
    print_instance_method_call_statement(
                  (InstanceMethodCallStatement*)method_call_statement,
                  cout);
}

void OsuifVisitorInfo::print_instance_method_call_statement(
              InstanceMethodCallStatement *instanceMethodCallStatement, 
              ostream& output){
  int indent, tempIndent, count;
  indent = printingMaps->getIndent();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';
  InstanceMethodSymbol *targetSymbol = 
          instanceMethodCallStatement->get_target_method();
  const ClassType *classType = targetSymbol->get_owning_class();
  if(classType != NULL) 
    output << classType->get_name() << "->" << targetSymbol->get_name();
  else
    output << targetSymbol->get_name(); 
  output << "(";
  MethodType* method_type = (MethodType*)targetSymbol->get_type();
  Iter<QualifiedType*> argumentIter = method_type->get_argument_iterator();

  //go past the first argument
  argumentIter.next();
  count = 1;
  
  while(argumentIter.is_valid()){
    if(count != 1) output << ",";
    QualifiedType *argument = (QualifiedType*)argumentIter.current();
    const MetaClass *mc = argument->get_meta_class();

    output << mc->get_instance_name();
    argumentIter.next();
    count++;
  }/*end while*/

  output << ");" << endl;
}/*InstanceMethodCallStatement*/

void OsuifVisitorInfo::do_procedure_definition(Address vis,
                          ProcedureDefinition* procedure_definition ){
  ((OsuifVisitorInfo *)vis)->
   print_procedure_definition((ProcedureDefinition*)procedure_definition, cout);
}

void OsuifVisitorInfo::print_procedure_definition(
            ProcedureDefinition* procedureDefinition, ostream& output){
  int count = 1;
  int indent, tempIndent;

  indent = printingMaps->getIndent();
  ProcedureSymbol *procedureSymbol =
                   procedureDefinition->get_procedure_symbol();

  CProcedureType *procedureType =
                 (CProcedureType*)procedureSymbol->get_type();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';
  Type* resultType = procedureType->get_result_type();

  const MetaClass *mc = resultType->get_meta_class();

  if(mc != NULL){
  output << mc->get_instance_name();
  }

  output << " ";

  output << procedureSymbol->get_name() << "(";

  Iter<QualifiedType*> iterArg = procedureType->get_argument_iterator();
  count = 1;

  while(iterArg.is_valid()){
    if(count != 1) output << ",";
    Type *typeArg = (Type*)iterArg.current();
    const MetaClass *mcArg = typeArg->get_meta_class();
    output << mcArg->get_instance_name();
    iterArg.next();
    count++;
  }
  output << "){" << endl ;
  printingMaps->incrementIndent();

  //process the symbol table
  SymbolTable *symbolTable = procedureDefinition->get_symbol_table();
  printingMaps->process_a_suif_object(symbolTable);

  DefinitionBlock *def_block = procedureDefinition->get_definition_block();
  printingMaps->process_a_suif_object(def_block);

  ExecutionObject *executionObj = procedureDefinition->get_body();
  printingMaps->process_a_suif_object(executionObj);

  printingMaps->decrementIndent();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';
  output << "}" << endl;
  output << endl;
}

void OsuifVisitorInfo::do_statement_list(Address vis, 
                        StatementList* statementList){
  ((OsuifVisitorInfo*) vis)->
                     print_statement_list((StatementList*)statementList, cout);
}

void OsuifVisitorInfo::print_statement_list(StatementList *statementList,
                        ostream& output){
  Iter<Statement* > iterStmt = statementList->get_statement_iterator();
  while(iterStmt.is_valid()){
    Statement *stmt = (Statement*)iterStmt.current();
    printingMaps->process_a_suif_object(stmt);
    output << endl;
    iterStmt.next();
  }//end while
}

void OsuifVisitorInfo::do_eval_statement(Address vis, 
                        EvalStatement* evalStatement){
  ((OsuifVisitorInfo*) vis)->
                     print_eval_statement((EvalStatement*)evalStatement, cout);
}

void OsuifVisitorInfo::print_eval_statement(EvalStatement *evalStatement,
                        ostream& output){
  Iter<Expression*> iterExpression = evalStatement->get_expression_iterator();
  while(iterExpression.is_valid()){
    Expression *expression = (Expression*)iterExpression.current();
    printingMaps->process_a_suif_object(expression);
    iterExpression.next();
  }//end while
}

void OsuifVisitorInfo::do_symbol_address_expression(Address vis,
                          SymbolAddressExpression* symbolAddressExpression){
  ((OsuifVisitorInfo*) vis)->
     print_symbol_address_expression(
                              (SymbolAddressExpression*)symbolAddressExpression,
                              cout);
}

void OsuifVisitorInfo::print_symbol_address_expression(
                              SymbolAddressExpression *symbolAddressExpression,
                              ostream& output){
  Symbol *symbol = symbolAddressExpression->get_addressed_symbol();
  output << symbol->get_name();
}

void OsuifVisitorInfo::do_variable_definition(Address vis,
                                    VariableDefinition* variableDefinition){
  ((OsuifVisitorInfo*)vis)->
    print_variable_definition((VariableDefinition*)variableDefinition, cout);
}/*end do_variable_definition*/

void OsuifVisitorInfo::print_variable_definition(
                              VariableDefinition* variableDefinition,
                              ostream& output){
  int indent, tempIndent;
  VariableSymbol *variableSymbol = variableDefinition->get_variable_symbol();
  Type *type = variableSymbol->get_type();

  indent = printingMaps->getIndent();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';
  output << type->get_name() << " ";
  output << variableSymbol->get_name();
  //output the initialization blocks
  ValueBlock *valueBlock = variableDefinition->get_initialization();
  //output the valueblock type need to change later
  const MetaClass *mc = valueBlock->get_type()->get_meta_class();
  output << " = new " << mc->get_instance_name() << ";";
  output << endl;
}/*print variable definition*/

void OsuifVisitorInfo::do_basic_symbol_table(Address vis,
                            BasicSymbolTable* basicSymbolTable){
  ((OsuifVisitorInfo*)vis)->
                print_basic_symbol_table((BasicSymbolTable*)basicSymbolTable,
                                         cout);
}/*end basic_symbol_table*/

void OsuifVisitorInfo::print_basic_symbol_table(
                            BasicSymbolTable* basicSymbolTable,
                            ostream& output){
  Iter<SymbolTableObject*> iterSto =
                basicSymbolTable->get_symbol_table_object_iterator();  
  while(iterSto.is_valid()){
    SymbolTableObject *sto = (SymbolTableObject*)iterSto.current();
    if(sto->isKindOf(ClassType::get_class_name()))
         printingMaps->process_a_suif_object(sto);
    iterSto.next();
  }/*end while*/
}/*end print*/

void OsuifVisitorInfo::do_definition_block(Address vis,
                            DefinitionBlock* definitionBlock){
  ((OsuifVisitorInfo*)vis)->
           print_definition_block((DefinitionBlock*)definitionBlock, cout);
}/*end definitionBlock*/

void OsuifVisitorInfo::print_definition_block(DefinitionBlock *definitionBlock,
                            ostream &output){
  Iter<VariableDefinition*> iterVariable = 
                        definitionBlock->get_variable_definition_iterator();
  while(iterVariable.is_valid()){
    VariableDefinition *varDef = (VariableDefinition*)iterVariable.current();
    printingMaps->process_a_suif_object(varDef);
    iterVariable.next();
  }/*end while*/

  Iter<ProcedureDefinition*> iterProcedure =
                        definitionBlock->get_procedure_definition_iterator();
  while(iterProcedure.is_valid()){
    ProcedureDefinition *procDef =(ProcedureDefinition*)iterProcedure.current();
    printingMaps->process_a_suif_object(procDef);
    iterProcedure.next();
  }/*end while*/
}

void OsuifVisitorInfo::do_call_statement(Address vis,
                            CallStatement* callStatement){
  ((OsuifVisitorInfo*) vis)->
                   print_call_statement((CallStatement*)callStatement,cout);
}

void OsuifVisitorInfo::print_call_statement(CallStatement *callStatement,
                                  ostream& output){
  int tempIndent, indent;
  indent = printingMaps->getIndent();
  for(tempIndent = 0; tempIndent < indent; tempIndent++) output << ' ';

  Expression *callee_address = callStatement->get_callee_address();
  printingMaps->process_a_suif_object(callee_address);
  output << "(";

  Iter<Expression*> argIter = callStatement->get_argument_iterator();
  while(argIter.is_valid()){
    Expression* argument = (Expression*)argIter.current();

    printingMaps->process_a_suif_object(argument);

    argIter.next();
  }/*end while*/
  output << ")";
}


void OsuifVisitorInfo::initOsuifObjects(){
  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_method_type,
                     MethodType::get_class_name());
                                  
  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_static_method_type,
                     StaticMethodType::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_instance_method_type,
                     InstanceMethodType::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_instance_method_symbol,
                     InstanceMethodSymbol::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_instance_field_symbol,
                     InstanceFieldSymbol::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_static_field_symbol,
                     StaticFieldSymbol::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_static_method_symbol,
                     StaticMethodSymbol::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_class_type,
                     ClassType::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                     (VisitMethod)OsuifVisitorInfo::do_inheritance_link,
                     InheritanceLink::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
                 (VisitMethod)OsuifVisitorInfo::do_static_method_call_statement,
                 StaticMethodCallStatement::get_class_name());

  printingMaps->register_process_visit_method((Address)this, 
              (VisitMethod)&OsuifVisitorInfo::do_instance_method_call_statement,
              InstanceMethodCallStatement::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
             (VisitMethod)OsuifVisitorInfo::do_procedure_definition,
             ProcedureDefinition::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
             (VisitMethod)OsuifVisitorInfo::do_statement_list,
              StatementList::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                     (VisitMethod)OsuifVisitorInfo::do_eval_statement,
                     EvalStatement::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                     (VisitMethod)OsuifVisitorInfo::do_call_statement,
                     CallStatement::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                     (VisitMethod)OsuifVisitorInfo::do_basic_symbol_table,
                     BasicSymbolTable::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                    (VisitMethod)OsuifVisitorInfo::do_variable_definition,
                    VariableDefinition::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                    (VisitMethod)OsuifVisitorInfo::do_symbol_address_expression,
                    SymbolAddressExpression::get_class_name());

  printingMaps->register_process_visit_method((Address)this,
                    (VisitMethod)OsuifVisitorInfo::do_definition_block,
                    DefinitionBlock::get_class_name());

}
