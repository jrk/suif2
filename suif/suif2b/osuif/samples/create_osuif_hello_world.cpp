// $Id: create_osuif_hello_world.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "suifkernel/suif_env.h"
#include "suifkernel/module_subsystem.h"
#include "typebuilder/type_builder.h"
#include "basicnodes/basic.h"
#include "basicnodes/basic_factory.h"
#include "suifnodes/suif.h"
#include "suifnodes/suif_factory.h"

#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"

#include <iostream.h>

#define BITS_PER_BYTE 8

const char *HELLO_STRING="hello world";

void build_tree(SuifEnv *suif);

int main(int argc, char* argv[]){
 SuifEnv* suif = new SuifEnv;
 suif->init();
 build_tree(suif);
 suif->write("osuif_hello_world.suif");
 return 0;
}

static BasicObjectFactory *basic_of;
static SuifObjectFactory *suif_of;
static OsuifObjectFactory *osuif_of;
static TypeBuilder* tb;

PointerType* pointer_to(Type *type){
 return suif_of->create_pointer_type(sizeof(void *), sizeof(void *), type);
}

void build_tree(SuifEnv *suif){
 init_basicnodes(suif);
 init_suifnodes(suif);
 init_osuifnodes(suif);
 init_typebuilder(suif);

 basic_of = 
   (BasicObjectFactory *)suif->get_object_factory(
                               BasicObjectFactory::get_class_name());
 suif_assert_message(basic_of, ("initialization error in basic"));
 
 suif_of = 
   (SuifObjectFactory *)suif->get_object_factory( 
                               SuifObjectFactory::get_class_name());
 suif_assert_message(suif_of, ("initialization error in suif"));
 
 osuif_of = 
   (OsuifObjectFactory *)suif->get_object_factory( 
                               OsuifObjectFactory::get_class_name());
 suif_assert_message(osuif_of, ("initialization error in osuif"));

 tb =
   (TypeBuilder *)suif->get_object_factory(TypeBuilder::get_class_name());
 suif_assert_message(tb, ("initialization error in typebuilder"));

 //create the external symbol table for the file_set_block
 BasicSymbolTable *external_symbol_table = 
   basic_of->create_basic_symbol_table(NULL);

 suif_assert_message(external_symbol_table, ("External Symbol table is NULL"));

 //create the file_set_symbol_table for the file_set_block
 BasicSymbolTable *file_set_symbol_table = 
   basic_of->create_basic_symbol_table(NULL);

 suif_assert_message(file_set_symbol_table, ("FileSymbol table is NULL"));

 //create file set block
 FileSetBlock *the_file_set_block = 
   basic_of->create_file_set_block(external_symbol_table, file_set_symbol_table);
 suif->set_file_set_block(the_file_set_block);

 //objects to create file block
 DefinitionBlock *file_def_block = basic_of->create_definition_block();

 BasicSymbolTable *file_symbol_table = 
   basic_of->create_basic_symbol_table(file_set_symbol_table);

 //instantiate the file block
 FileBlock *the_file_block = 
   basic_of->create_file_block("hello_world.cc", file_symbol_table, 
                               file_def_block);

 //add file block to file set
 the_file_set_block->append_file_block(the_file_block);

 IntegerType *argc_type =
   tb->get_integer_type(sizeof(int)*BITS_PER_BYTE,
			sizeof(int)*BITS_PER_BYTE, true);
 QualifiedType *q_argc_type = tb->get_qualified_type(argc_type);
 
 IntegerType *char_type = 
   tb->get_integer_type(sizeof(char)*BITS_PER_BYTE,
			sizeof(char)*BITS_PER_BYTE,true);
 QualifiedType *q_char_type = tb->get_qualified_type(char_type);
 
 // VoidType *void_type = suif_of->create_void_type(sizeof(int), sizeof(int));
 // QualifiedType *q_void_type = tb->get_qualified_type(void_type);
 
 PointerType *char_ptr_type = tb->get_pointer_type(q_char_type);
 QualifiedType *q_char_ptr_type = tb->get_qualified_type(char_ptr_type);

 PointerType *argv_type = tb->get_pointer_type(q_char_ptr_type);    
 QualifiedType *q_argv_type = tb->get_qualified_type(argv_type);

 CProcedureType *main_type = 
   suif_of->create_c_procedure_type(argc_type, false, true, sizeof(int));

 main_type->set_result_type(argc_type);
 main_type->append_argument(q_argc_type);
 main_type->append_argument(q_argv_type);
 
 //create the procedure symbol main with definition null, set the definition
 ProcedureSymbol *main_symbol =
   basic_of->create_procedure_symbol(main_type, "main", true);

 //add main to external symbol table since it is neccessary for linkage
 the_file_set_block->get_external_symbol_table()->add_symbol(main_symbol);
 
 //construct the statement list
 StatementList *main_body = basic_of->create_statement_list();

 BasicSymbolTable* main_symbol_table = 
   basic_of->create_basic_symbol_table(file_set_symbol_table);

 DefinitionBlock *main_def_block = basic_of->create_definition_block();
 ProcedureDefinition *main_definition = 
   basic_of->create_procedure_definition(main_symbol, main_body, 
                                        main_symbol_table, main_def_block);

 //set the definition in main symbol
 main_symbol->set_definition(main_definition);

 ParameterSymbol *argc_symbol = 
   basic_of->create_parameter_symbol( q_argv_type, "argc", false );
 main_definition->get_symbol_table()->add_symbol(argc_symbol);
 main_definition->append_formal_parameter(argc_symbol);

 ParameterSymbol *argv_symbol =
   basic_of->create_parameter_symbol( q_argc_type, "argv", false );
 main_definition->get_symbol_table()->add_symbol(argv_symbol);
 main_definition->append_formal_parameter(argv_symbol);
 
 //objects needed for the creation of class type
 //the symbol tables of this class donot have an explicit super scope
 //because they are not derived
 
 ClassType *test_class_type = 
   osuif_of->create_class_type( IInteger(sizeof(int)),
				sizeof(int), 
				basic_of->create_definition_block(),
				"Test",
				true, // is_complete
				osuif_of->create_instance_field_symbol_table(),
				true, // methods_are_complete
				osuif_of->create_per_class_symbol_table(), 
				osuif_of->create_instance_method_symbol_table() );

 //add only to the file block symbol table
 the_file_block->get_symbol_table()->add_symbol(test_class_type);

 QualifiedType *q_test_class_type =
   tb->get_qualified_type( test_class_type );
 
 //create a method type called displayMethod
 InstanceMethodType *display_method_type = 
   osuif_of->create_instance_method_type( argc_type, // result_type
					  false, // has_varargs
					  true, // arguments_known
					  sizeof(int) );

 //set result type for this method
 display_method_type->set_result_type(argc_type);

 //set the receiver on method type to the reciever class
 //display_method_type->set_receiver_type(pointer_to(test_class_type));

 //create the method symbol
 InstanceMethodSymbol *display_method_symbol = 
   osuif_of->create_instance_method_symbol( display_method_type,
					    "display" );
 
 //append the public attribute
 display_method_symbol->append_attribute("public");

 //add the method symbol to the instance method symbol lookup table 
 test_class_type->get_instance_method_symbol_table()->add_symbol(display_method_symbol);

 //add the string hello world to the instance variable
 Expression *i0 = basic_of->create_int_constant(0,0);
 Expression *i1 = basic_of->create_int_constant(0,strlen(HELLO_STRING) + 1);

 ArrayType *string_literal_type =
   tb->get_array_type(IInteger((strlen(HELLO_STRING) + 1) * sizeof(char)),
		      (int)sizeof(char)*BITS_PER_BYTE,
		      q_char_type,i0,i1);

 QualifiedType *q_string_literal_type =
   tb->get_qualified_type(string_literal_type);
    
 StaticFieldSymbol *hello_str_symbol = 
   osuif_of->create_static_field_symbol( q_string_literal_type, 
					 "hello_str" );

 MultiValueBlock *string_literal_initialization =
   suif_of->create_multi_value_block(string_literal_type);

 for( unsigned char_num = 0; char_num <= strlen(HELLO_STRING); ++char_num){
   string_literal_initialization->add_sub_block(
    char_num, 
    suif_of->create_expression_value_block(
      basic_of->create_int_constant(char_type, HELLO_STRING[char_num])));
 }

 VariableDefinition *string_literal_definition =
   basic_of->create_variable_definition(hello_str_symbol, 
                            sizeof(char)*BITS_PER_BYTE,
                            string_literal_initialization);

 //set definition of the symbol
 hello_str_symbol->set_definition(string_literal_definition);

 //append an attribute
 hello_str_symbol->append_attribute("private");

 test_class_type->get_group_symbol_table()->add_symbol(hello_str_symbol);

 //instantiation of the class with no initial value
 VariableSymbol *test_class_symbol = 
   basic_of->create_variable_symbol(q_test_class_type, "test", true);

 UndefinedValueBlock *undefined_block = 
   suif_of->create_undefined_value_block(test_class_type);

 VariableDefinition *test_class_definition = 
   basic_of->create_variable_definition(test_class_symbol, 
                                        sizeof(int), undefined_block);

 main_definition->get_definition_block()->append_variable_definition(test_class_definition);

 //add the method call instruction to the statement list 
 Expression* display_address_op = suif_of->create_symbol_address_expression( 
                      argc_type, display_method_symbol);  

 InstanceMethodCallStatement *display_call =
   osuif_of->create_instance_method_call_statement( NULL, // destination
						    display_address_op,
						    display_method_symbol,
						    true // is_dispatched
						    );

 //append argument
 Expression* receiver =
   suif_of->create_symbol_address_expression( pointer_to(test_class_type),
					      test_class_symbol);
 display_call->append_argument(receiver);

 main_body->append_statement(display_call);
 
 //add the variable definition  to main procedure definition
 main_definition->get_symbol_table()->add_symbol(test_class_symbol);

 the_file_block->get_definition_block()->append_procedure_definition(main_definition);

 StatementList *display_body = basic_of->create_statement_list();

 BasicSymbolTable* display_symbol_table = 
   basic_of->create_basic_symbol_table(file_symbol_table);

 DefinitionBlock* display_def_block = basic_of->create_definition_block();

 ProcedureDefinition *display_definition =
   basic_of->create_procedure_definition(display_method_symbol, display_body, 
                            display_symbol_table, display_def_block); 

 display_method_symbol->set_definition(display_definition);

 CProcedureType *printf_type = 
   suif_of->create_c_procedure_type(argc_type, false, true, 
                                        sizeof(int) * BITS_PER_BYTE);

 printf_type->append_argument( q_char_ptr_type );

 ProcedureSymbol *printf_symbol =
   basic_of->create_procedure_symbol(printf_type, "printf", true); 
 the_file_set_block->get_external_symbol_table()->add_symbol(printf_symbol);

 Expression* printf_address_op = suif_of->create_symbol_address_expression(
                                pointer_to(printf_type), printf_symbol);

 Expression* printf_argument_op = suif_of->create_symbol_address_expression(
                                 pointer_to(char_type), hello_str_symbol);

 CallStatement *printf_call =
   suif_of->create_call_statement( NULL, 
				   printf_address_op );

 printf_call->append_argument(printf_argument_op);

 display_body->append_statement(printf_call);

 the_file_block->get_definition_block()->append_procedure_definition(display_definition);
 
 FormattedText fd;
 the_file_set_block->print(fd);
 cout<< fd.get_value() << endl;
}

