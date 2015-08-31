#include "AIO_Config.h"

AIO_Config::AIO_Config()
:
n_thread_(1),
n_demultiplexor_(1),
aio_type_(DEFAULT),
timeout_(0),
max_aio_op_(0),
signal_n_(0),
leader_type_(0),
start_aio_type_(1)
{
	init();
}

AIO_Config::~AIO_Config()
{
}

void
AIO_Config::init()
{
	size_t n_processor = (size_t) ACE_OS::num_processors_online();
	if ( n_processor < DEFAULT_THREAD )
		n_processor = DEFAULT_THREAD;
	
	n_thread_ = n_processor; // at least DEFAULT_THREAD

#if defined(sun)
	aio_type_ = POLL;
#else
	aio_type_ = SELECT;
#endif /*sun*/
}
