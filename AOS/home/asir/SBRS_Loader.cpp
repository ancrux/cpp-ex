#include "SBRS_Loader.h"

SBRS_Loader::SBRS_Loader(SBRS_MAPS& maps)
:
stop_(0),
maps_(maps),
index_(0)
{
}

SBRS_Loader::~SBRS_Loader()
{
	this->stop();
}

void
SBRS_Loader::start(int n_thread)
{
	this->index_ = 0;
	this->stop_ = 0;
	this->activate(THR_NEW_LWP | THR_JOINABLE, n_thread);
}

void
SBRS_Loader::stop()
{
	this->stop_ = 1;
	this->wait();
	this->index_ = 0;
}

int
SBRS_Loader::svc()
{
	int idx = -1;
	{
		ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, -1);
		idx = index_;
		++index_;
	}
	if ( idx < 0 ) return -1;

	ACE_OS::printf("thr(%u) start with idx/size=%d/%d...\n", ACE_OS::thr_self(), idx, maps_.size()); //@

	const char* path = "sbrs.dat";

	do
	{
		ACE_OS::printf("open '%s'\n", path);
		ACE_HANDLE fh = ACE_OS::open(path, O_BINARY | O_RDONLY);
		if ( fh == ACE_INVALID_HANDLE )
		{
			ACE_OS::printf("open '%s' failed!\n", path);
			break;
		}

		size_t n_map = maps_.size();
		ACE_UINT32 ipv4;

		const int BUF_SIZE = 20000;
		unsigned char buf[BUF_SIZE];
		//ACE_OS::printf("stop=%d\n", stop_.value()); //@
		while(!stop_.value())
		{
			ssize_t n_read = ACE_OS::read(fh, buf, BUF_SIZE);
			if ( n_read <= 0 ) break;
			//ACE_OS::printf("read %d bytes...\n", n_read);

			for(int i=0; i < n_read; i += 5)
			{
				const unsigned char* item = buf + i;
				ipv4 = (*item) * 16777216 + (*(item+1)) * 65536 + (*(item+2)) * 256 + (*(item+3));
				if ( (*(item+3)) % n_map == idx )
				{
					(maps_[idx])->insert(std::make_pair(ipv4, *(item+4)));
					//(maps_[idx])->insert(ipv4, *(item+4));
				}
				//ACE_OS::printf("ip=%u, score=%d\n", ipv4, *(item+4));
			}
		}

		ACE_OS::close(fh);
	}
	while(0);

	ACE_OS::printf("thr(%u) stop...\n", ACE_OS::thr_self()); //@

	return 0;
}
