#ifndef _MIRS_H_
#define _MIRS_H_

#include "ace/OS.h"
#include "ace/Atomic_Op.h"
#include "ace/Synch_T.h"

#include "stx/btree_map" // namespace stx
typedef stx::btree_map< ACE_UINT32, char > MIRS_MAP;

class MIRS_Score
{
public:
	static const int N_MAP = 256;

public:
	MIRS_Score();
	~MIRS_Score();

public:
	size_t size();
	void insert(ACE_UINT32 ip32, char ch);

protected:
	MIRS_MAP maps_[N_MAP];
	ACE_Thread_Mutex locks_[N_MAP];
};

#endif // _MIRS_H_
