#include "MP_Queue.h"

MP_Queue::MP_Queue()
:
interval(0),
last_visited(ACE_Time_Value::zero)
{
}

MP_Queue::~MP_Queue()
{
}