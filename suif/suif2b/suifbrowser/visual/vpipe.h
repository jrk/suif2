/* vpipe.h */

#ifndef VPIPE_H
#define VPIPE_H

#include <stdio.h>
#include <fstream.h>

class vpipe {
private:
  fstream fstr;
  int readpos;
  int writepos;
public:
  vpipe(void);
  ~vpipe(void);

  fstream& fout(void) { return fstr; }
  void clear(void);
  int size(void);
  int read(char *ptr, int size); // read from pipe, returns # of bytes
};
#endif // VPIPE_H
