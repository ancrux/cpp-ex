#include "Spool_Scanner.h"

#include "aos/OS.h"

Spool_Scanner::Spool_Scanner(Mail_Processor& mp)
:
go_(0),
mp_(mp)
{
}

Spool_Scanner::~Spool_Scanner()
{
	this->stop();
}

void
Spool_Scanner::start()
{
	this->go_ = 1;
	this->activate(THR_NEW_LWP | THR_JOINABLE, 1);
}

void
Spool_Scanner::stop()
{
	this->go_ = 0;
	this->wait();
}

int
Spool_Scanner::svc()
{
	ACE_OS::printf("scanner(%u) started...\n", ACE_OS::thr_self()); //@

	char path_spool[PATH_MAX+1];
	ACE_OS::realpath(mp_.cfg.path_spools[0].c_str(), path_spool);
	ACE_OS::printf("path_spool: %s\n", path_spool);

	ACE_Time_Value sleep_tv; sleep_tv.set(0.5);
	while( go_.value() )
	{
		do {
			ACE_DIRENT* d;
			ACE_Dirent dir;

			if ( dir.open(path_spool) != 0 )
			{
				ACE_OS::printf("open dir '%s' failed!\n", path_spool);
				break;
			}

			int n_done = 0;
			while( (d = dir.read()) != 0 )
			{
				std::string file(path_spool); file += ACE_DIRECTORY_SEPARATOR_CHAR; file += d->d_name;
				if ( aos::path_exists(file.c_str()) != 1 ) // not file
					continue;

				if ( ACE_OS::strcasecmp(".done", file.c_str() + file.size() - 5) != 0 ) // not ended with ".done"
					continue;

				//ACE_OS::printf("F='%s'\n", file.c_str()); //@

				ACE_Message_Block* mb = new ACE_Message_Block(2048);
				mb->copy(file.c_str()); // or mb->copy(file.c_str(), file.size()+1);

				mp_.putq(mb);
			}
			dir.close();
		} while(0);

		// sleep until mp's msg_queue() is empty again
		do 
		{
			ACE_OS::sleep(sleep_tv);

			ACE_OS::printf("n_busy/n_queue: %d/%d\n", mp_.busy(), mp_.msg_queue()->message_count());
		}
		while( !mp_.msg_queue()->is_empty() || mp_.busy() ); // not empty or is busy, keep idle
			
	}

	ACE_OS::printf("scanner(%u) stopped...\n", ACE_OS::thr_self()); //@

	return 0;
}

