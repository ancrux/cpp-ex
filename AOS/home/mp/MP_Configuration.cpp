#include "MP_Configuration.h"

#include "ace/Configuration_Import_Export.h"

MP_Configuration::MP_Configuration()
{
}

MP_Configuration::~MP_Configuration()
{
	clear_queues();
}

void
MP_Configuration::clear_queues()
{
	for(QUEUES::iterator iter = queues_.begin(); iter != queues_.end(); ++iter)
	{
		delete iter->second;
	}
	queues_.clear();
}

void
MP_Configuration::load(const char* ini_file)
{
	// clear queues
	clear_queues();

	// load ini file
	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp iniIO(config);
	iniIO.import_config(ini_file);

	// read ini
	ACE_TString name;
	for(int i = 0;
		config.enumerate_sections(config.root_section(), i, name) == 0;
		++i)
	{
		ACE_Configuration_Section_Key sec;
		config.open_section(config.root_section(), name.c_str(), 0, sec);
		
		// read path
		ACE_TString str_path;
		config.get_string_value(sec, ACE_TEXT("path"), str_path);
		char path[PATH_MAX+1];
		char* queue_path = ACE_OS::realpath(str_path.c_str(), path);
		if ( !str_path.empty() && queue_path )
		{
			ACE_OS::printf("realpath: %s\n", queue_path); //@

			MP_Queue* q = new MP_Queue();
			q->path = queue_path;
			if ( q->path[q->path.size()-1] != ACE_DIRECTORY_SEPARATOR_CHAR )
				q->path += ACE_DIRECTORY_SEPARATOR_CHAR;

			// read next
			ACE_TString str_next;
			config.get_string_value(sec, ACE_TEXT("next"), str_next);
			char next[PATH_MAX+1];
			char* queue_next = ACE_OS::realpath(str_next.c_str(), next);
			if ( !str_next.empty() && queue_next )
			{
				q->next = queue_next;
				if ( q->next[q->next.size()-1] != ACE_DIRECTORY_SEPARATOR_CHAR )
					q->next += ACE_DIRECTORY_SEPARATOR_CHAR;
			}

			// read interval
			ACE_TString str_interval;
			config.get_string_value(sec, ACE_TEXT("interval"), str_interval);
			int interval = ACE_OS::atoi(str_interval.c_str());
			if ( interval )
				q->interval = interval;

			// add MP_Queue
			queues_.insert(std::make_pair(queue_path, q));
		}
	}
}