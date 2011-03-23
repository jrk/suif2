#include "common/system_specific.h"
#include "unowned.h"

#include "iokernel/cast.h"
#include "suifkernel/group_walker.h"
#include "suifkernel/suif_object.h"
#include "iokernel/object_stream.h"
#include "common/suif_hash_map.h"

#include <stdio.h>


class Unowned : public SelectiveWalker {
public:
	Unowned(SuifEnv *pEnv, suif_hash_map<Address, FormattedText*> *pMap, Address pRoot) :
		SelectiveWalker(pEnv, SuifObject::get_class_name()),
		m_pMap(pMap),
		m_pRoot(pRoot)
	  { }
	bool is_walkable(Address address, bool is_owned, MetaClass *_meta);
	ApplyStatus operator()(SuifObject *x);
private:
    suif_hash_map<Address, FormattedText*> *m_pMap;
	Address m_pRoot;
};

bool
Unowned::is_walkable(Address address, bool is_owned, MetaClass *_meta)
{
	FormattedText *pText = new FormattedText;
	SuifObject *pSO = (SuifObject *)address;

	if (pSO->get_parent() == NULL && address != m_pRoot) {
		pSO->print(*pText);
		m_pMap->enter_value(address, pText);
	}
	return is_owned;	
}

Walker::ApplyStatus 
Unowned::operator()(SuifObject *x) 
{
	//SuifObject *pSO = to<SuifObject>(x);
	//SuifObject *pParent = pSO->get_parent();
	return Walker::Continue;
}

void UnownedPass::do_file_set_block(FileSetBlock *pFSB)
{
	suif_hash_map<Address, FormattedText*> Map;
	// I pass the FileSetBlock pointer because we don't want
	// to note if this is unowned...they are always unowned.
	Unowned walker(get_suif_env(), &Map, pFSB);

	pFSB->walk(walker);
	suif_hash_map<Address, FormattedText*>::iterator iter = Map.begin();
	while (iter != Map.end()) { 
		printf("Unowned object at: %#x\n",
		       (unsigned int)((*iter).first));
		printf("%s\n", (const char*)(((*iter).second)->get_value()));
		iter++;
	}

	
}

UnownedPass::UnownedPass(SuifEnv *pEnv, const LString &name) :
	Pass(pEnv, name)
{
}

