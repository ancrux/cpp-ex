#include "MP_Enqueuer.h"

#include "MP_Message.h"
#include "MP_Configuration.h"

#include "ace/Configuration_Import_Export.h"
#include "ace/Dirent.h"

MP_Enqueuer::MP_Enqueuer(MP_Worker& worker)
:
worker_(worker),
stop_(0)
{
}

MP_Enqueuer::~MP_Enqueuer()
{
}

int
MP_Enqueuer::svc(void)
{
	ACE_OS::printf("(%d) MP_Enqueuer starting up...\n", (int) ACE_OS::thr_self());

	MP_Configuration cfg;
	cfg.load("mp.ini");

	ACE_Time_Value sleep_tv; sleep_tv.set(0.01);
	ACE_Time_Value t1, t2;
	size_t n_per_batch = 0;
	size_t n_per_batch_byte = 0;
	while( !stop_ )
	{
		if ( n_per_batch == 0 )
			t1 = ACE_OS::gettimeofday();

		size_t n_per_scan = 0;
		size_t n_per_scan_byte = 0;
		for(MP_Configuration::QUEUES::iterator iter = cfg.queues().begin();
			iter != cfg.queues().end();
			++iter)
		{
			// check for interval and last visited time
			if ( (iter->second)->interval &&
				(iter->second)->last_visited != ACE_Time_Value::zero &&
				ACE_OS::gettimeofday() - (iter->second)->last_visited < ACE_Time_Value((iter->second)->interval) )
					continue;

			ACE_DIRENT* d;
			ACE_Dirent dir;
			std::string folder((iter->second)->path);

			if ( dir.open(folder.c_str()) != 0 )
				continue;

			//if ( folder[folder.size()-1] != ACE_DIRECTORY_SEPARATOR_CHAR )
			//	folder += ACE_DIRECTORY_SEPARATOR_CHAR;

			while( (d = dir.read()) != 0 )
			{
				std::string file = folder + d->d_name;

				ACE_stat stat;
				if ( ACE_OS::lstat(file.c_str(), &stat) == -1 || (stat.st_mode & S_IFMT) == S_IFDIR )
					continue;

				if ( ACE_OS::strcasecmp(file.c_str() + file.size() - 4, ".eml") != 0 )
					continue;

				++n_per_scan;
				n_per_scan_byte += stat.st_size;
				if ( n_per_scan % 10000 == 0 && stop_ )
					break;

				MP_Message* msg = new MP_Message(file);
				msg->obj(iter->second); // MP_Queue*
				worker_.msg_queue()->enqueue_tail(msg);
			}
			dir.close();

			// update last visited time
			(iter->second)->last_visited = ACE_OS::gettimeofday();
		}
		n_per_batch += n_per_scan;
		n_per_batch_byte += n_per_scan_byte;

		// wait util finished
		while( !stop_  && !worker_.msg_queue()->is_empty() )
		{
			ACE_OS::sleep(sleep_tv);
		}
		// calculate speed
		if ( n_per_scan )
		{
			t2 = ACE_OS::gettimeofday();
			double n_sec = double(t2.msec()-t1.msec())/1000;
			if ( n_sec > 0.0 )
			{
				double msg_rate = n_per_batch/n_sec;
				double byte_rate = n_per_batch_byte/n_sec;
				ACE_OS::printf("rate:%.3f msg/sec, %.3f byte/sec in %.3f sec(s). n_read: %d\n", msg_rate, byte_rate, n_sec, n_per_scan); //@
			}
		}
		// rest for a while if all queues are empty
		if ( !stop_ && n_per_scan == 0 )
		{
			if ( n_per_batch ) ACE_OS::printf("n_per_batch:%d\n", n_per_batch); //@
			n_per_batch = 0;
			n_per_batch_byte = 0;
			ACE_OS::sleep(sleep_tv);
		}
	}

	ACE_OS::printf("(%d) MP_Enqueuer shutting down...\n", (int) ACE_OS::thr_self());

	return 0;
}