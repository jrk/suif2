/*-------------------------------------------------------------------
 * suif_vnode.cc
 *
 */
#include "suifnodes/suif.h"
#include "suif_vnode.h"
#include "visual/visual.h"

const char* tag_suif_object;
const char* tag_code_fragment;


/*-------------------------------------------------------------------
 * init_vnode_tags
 */
void init_vnode_tags() {
  tag_suif_object = LString("suif_object").c_str();
  tag_code_fragment = LString("code_fragment").c_str();
}
/*-------------------------------------------------------------------
 * create_vnode
 */
vnode *
create_vnode(SuifObject* suifobj)
{
  vnode* vn = vman->find_vnode( suifobj );
  if ( !vn ) {
    vn = new vnode( suifobj, tag_suif_object );
  } else {
    if ( vn->get_tag() != tag_suif_object ) {
      suif_assert_message(false, ("Internal error. (`%s')", vn->get_tag()));
    }
  }
  return vn;
}

