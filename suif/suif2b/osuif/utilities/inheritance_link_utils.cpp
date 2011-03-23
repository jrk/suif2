// $Id: inheritance_link_utils.cpp,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $

#include "osuifnodes/osuif.h"
#include "osuifnodes/osuif_factory.h"

#include "osuifutilities/inheritance_link_utils.h"


InheritanceLink* add_inheritance_link( ClassType* ancestor,
				       ClassType* descendant )
{
  InheritanceLink* ilink = 
    ::create_inheritance_link( ancestor->get_suif_env(),
			       ancestor,
			       descendant );
  suif_assert( ilink->get_parent() == NULL );

  ancestor->append_child_classe( ilink );
  suif_assert( ilink->get_parent() == ancestor );

  descendant->append_parent_classe( ilink );
  suif_assert( ilink->get_parent() == ancestor );

  return ilink;
}
