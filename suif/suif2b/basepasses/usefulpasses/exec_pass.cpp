#include <string.h>

#include <errno.h>
#ifndef MSVC
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#else
#include <process.h>
#endif

#include "suifkernel/suif_exception.h"
#include "exec_pass.h"


ExecPass::ExecPass(SuifEnv* senv) :
  SimpleModule(senv, "exec")
{}


String ExecPass::get_help_string(void) const
{
  return String("Usage:\n  exec <command> <arg1> <arg2> ...\nexecutes a shell command in <command>");
}


typedef char* Cstr;

void ExecPass::execute(suif_vector<LString>* args)
{
/*
    Notice that fork/wait don't work on NT, hence, resorting to
    #ifdefs. The same goes for #include #ifdefs above.
*/
#ifndef MSVC
  if (fork() != 0) { // parent
    int stat;
    wait(&stat);
  } else  // child
  {
    Cstr *argv = new Cstr[args->size() + 1];
    if (argv == NULL)
      SUIF_THROW(SuifException("Out of memory"));
    {for (unsigned i = 0; i<args->size(); i++)
      argv[i] = const_cast<char*>(args->at(i).c_str());
    }
    argv[args->size()] = NULL;
    execvp(argv[0], argv);
    String errmsg;
    {for (unsigned i = 0; i<args->size(); i++)
      errmsg += args->at(i);
        SUIF_THROW(SuifException(String(strerror(errno)) + ": while executing " 
			     + errmsg));
    }
  }
#else
  Cstr *argv = new Cstr[args->size() + 1];
  if (argv == NULL)
    SUIF_THROW(SuifException("Out of memory"));
  {for (unsigned i = 0; i<args->size(); i++)
    argv[i] = const_cast<char*>(args->at(i).c_str());
  }
  argv[args->size()] = NULL;
  _spawnvp(_P_WAIT, argv[0], argv);
#endif
}
