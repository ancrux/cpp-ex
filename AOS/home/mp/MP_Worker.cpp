#include "MP_Worker.h"

#include "MP_Configuration.h"

#include "aos/String.h"
#include "aos/mime/MIME_Entity.h"
#include "aos/mime/MIME_Util.h"
using namespace aos;

int
MP_Worker::svc(void)
{
	ACE_OS::printf("(%d) MP_Worker starting up...\n", (int) ACE_OS::thr_self());

	//MP_Configuration cfg;
	//cfg.load("mp.ini");
	
	MP_Message* msg;
	MIME_Entity e;

	while(this->msg_queue()->dequeue_head(msg) != -1)
	{
		if ( msg->empty() )
		{
			delete msg;
			break;
		}

		if ( MIME_Util::import_file(e, msg->c_str()) != 0 )
		{
			ACE_OS::printf("import_file() failed!\n");
			delete msg;
			continue;
		}

		MIME_Util::decode_body(e);
		//e.dump_header();

		// move to next queue
		MP_Queue* queue = (MP_Queue*) msg->obj();
		if ( !(queue->next).empty() )
		{
			std::string target(queue->next);
			target += ACE::basename(msg->c_str());
			//ACE_OS::printf("target:%s\n", target.c_str());

			// move file
			int move_flags = -1; // MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH;
			if ( ACE_OS::rename(msg->c_str(), target.c_str(), move_flags) != 0 )
				ACE_OS::printf("rename() failed!\n");
		}

		//ACE_OS::printf("%s\n", msg->c_str());
		delete msg;
	}

	ACE_OS::printf("(%d) MP_Worker shutting down...\n", (int) ACE_OS::thr_self());

	return 0;
}