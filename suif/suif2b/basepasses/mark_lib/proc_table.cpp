/*
 * file : ProcTable.cpp
 *
 */

#include "proc_table.h"
#include <stdlib.h>
#include <fstream.h>
#include "suifkernel/suifkernel_messages.h"


StaticProcTable::StaticProcTable(String filename) :
  ProcTable(),
  _proc_names()
{
  if (filename.length() == 0) {
    char* nci = getenv("NCIHOME");
    suif_assert_message(nci != 0, ("Environment vairbale NCIHOME not defined."));
    filename = String(nci) + "/suif/suif2b/basepasses/mark_lib/system_proc.txt";
  };  
  ifstream fin(filename);
  suif_assert_message(!fin.fail(), ("Cannot open file [%s].", filename.c_str()));
  char ch;
  String line;
  while ((ch = fin.get()) && ch != EOF) {
    if (ch == '\n')
      process_input_line(line);
    else
      line += ch;
  }
  fin.close();
}

void StaticProcTable::process_input_line(String line)
{
  line.truncate_to_last(' ');
  line.truncate_to_last('\t');
  line.trim_to_first(' ');
  line.trim_to_first('\t');
  _proc_names.push_back(line);
}

bool StaticProcTable::is_in_table(ProcedureSymbol* psym)
{
  LString pname = psym->get_name();
  return _proc_names.is_member(pname);
}
