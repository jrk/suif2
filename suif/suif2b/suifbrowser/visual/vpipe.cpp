/* vpipe */

#include "vpipe.h"
#include "vcommands.h"
#include <stdlib.h>
#include <unistd.h>


#define MIN(x, y) (((x) < (y)) ? (x) : (y))


/*-------------------------------------------------------------------
 *
 */ 
vpipe::vpipe(void)
{
  /* create pipe */
  FILE *pipefd = tmpfile();
  if (pipefd == 0) {
    v_error(1, "Cannot open tmp file");
  }
  readpos = writepos = 0;
  fstr.attach(fileno(pipefd));
}

vpipe::~vpipe(void) 
{
  //fclose(pipefd);
}

/*-------------------------------------------------------------------
 * read from pipe
 *
 */
int vpipe::read(char *ptr, int size)
{
  writepos = fstr.tellp();

  if (readpos == writepos) {
    return (0);
  }

  fstr.seekg(readpos);
  
  int s = MIN(size - 1, writepos - readpos);
  int result = s;
  fstr.read(ptr, s);
  
  readpos += result;
  if (writepos == readpos) {
    readpos = writepos = 0;
  }
  fstr.seekp(writepos);
  
  ptr[result] = 0;		// mark end of string
  
  return (result);
}

/*-------------------------------------------------------------------
 * vpipe::clear
 */
void vpipe::clear(void)
{
  readpos = writepos = 0;
  fstr.seekg(0); fstr.seekp(0);
}

/*-------------------------------------------------------------------
 * size of pipe data
 */
int vpipe::size(void)
{
  int orig_pos = fstr.tellp();
  fstr.seekp(0);
  int size = fstr.tellp();
  fstr.seekp(orig_pos);
  return (size);
}
