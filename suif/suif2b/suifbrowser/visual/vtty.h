/*--------------------------------------------------------------------
 * vtty.h
 *
 */

#ifndef VTTY_H
#define VTTY_H

#include "vwidget.h"
#include "vtcl.h"
#include "vpipe.h"

class vnode;
class binding;

/* text styles */

enum text_style {
  BOLD_BEGIN = 1,
  BOLD_END = 2,
  ITALIC_BEGIN = 4,
  ITALIC_END = 8
  };

enum detail_kind {
  PRINT_BRIEF = 0,
  PRINT_FULL
};

typedef void (*print_fn)(class vtty *text, vnode *object, int depth,
			 detail_kind detail, void *client_data);

/*
 * text widget
 */

class vtty : public vwidget {

protected:
  vpipe *text_pipe;		// text pipe

public:
  vtty(vwidget *par);
  ~vtty(void);
  virtual void destroy(void);
  virtual int kind(void) { return WIDGET_TTY; }
  
  /* text I/O */
  fstream& fout(void) { return text_pipe->fout();}
  virtual void update(void) = 0;
  virtual void clear(void) = 0;

  /* attributes */
  virtual void tag_style(text_style style) = 0;
  virtual void *tag_begin(vnode *obj) = 0;
  virtual void *tag_begin(vnode *obj, print_fn fn, int d,
			  void *client_data) = 0;
  virtual void tag_end(vnode *obj) = 0;

  /* viewing */
  virtual void view(int row, int col) = 0;

private:
  /* override stupid defaults; do not implement */
  vtty &operator=(const vtty&);
  vtty(const vtty&);
};

#endif
