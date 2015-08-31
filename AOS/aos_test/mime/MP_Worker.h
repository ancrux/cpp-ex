#include "ace/OS.h"
#include "ace/Task.h"

#include <iostream>
using namespace std;

class MP_Worker : public ACE_Task<ACE_MT_SYNCH>
{
public:
	virtual int svc();
};