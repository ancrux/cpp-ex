#include "MIRS_Score.h"

MIRS_Score::MIRS_Score()
{
}

MIRS_Score::~MIRS_Score()
{
	for(int i=0; i < N_MAP; ++i)
		maps_[i].clear();
}

size_t
MIRS_Score::size()
{
	size_t n = 0;
	for(int i=0; i < N_MAP; ++i)
	{
		ACE_OS::printf("maps[%d]=%d\n", i, maps_[i].size());
		n += maps_[i].size();
	}

	return n;
}

void
MIRS_Score::insert(ACE_UINT32 ip32, char ch)
{
	// std::pair<iterator, bool> stx::btree_map< _Key, _Data, _Compare, _Traits >::insert  ( const value_type &  x   ) 
	
	ACE_UINT8 i = ip32 >> 24;
	maps_[i].insert(std::make_pair(ip32, ch));
}
