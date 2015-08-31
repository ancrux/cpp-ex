#include "MP_Worker.h"

#include "aos/String.h"
#include "aos/mime/MIME_Analyzer.h"
using namespace aos;

int
MP_Worker::svc(void)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT ("(%t) starting up \n")));
	ACE_Message_Block* mb;

	MIME_Entity e;

	int n_eml_http_1 = 0;
	int n_eml_http_n = 0;
	int n_eml_phone = 0;
	FILE* fp;
	fp = ::fopen("d:\\_spam_\\http.log", "w");

	// pattern hash
	std::set< ACE_UINT32 > pat_set;
	int n_dup = 0;

	while(this->msg_queue()->dequeue_head(mb) != -1)
	{
		if ( ACE_OS::strlen(mb->base()) == 0 )
		{
			ACE_DEBUG(( 
				LM_DEBUG, 
				ACE_TEXT("(%t) shutting down\n")
				));
			break;
		}

		///*
		ACE_DEBUG(( 
			LM_DEBUG, 
			ACE_TEXT("(%t) %s\n"), 
			(mb->base())
			));
		//*/

		//::printf("open eml:%s\n", mb->base());
		fwrite(mb->base(), mb->size()-1, 1, fp); // mb->size()-1 to trim '\0' at the end
		fwrite("\n", 1, 1, fp);

		FILE* fp2 = ::fopen("d:\\_spam_\\_log_\\utf-8.txt", "a");
		//fwrite("\xEF\xBB\xBF", 3, 1, fp2);
		::fwrite(mb->base(), mb->size()-1, 1, fp2); // mb->size()-1 to trim '\0' at the end
		::fwrite("\r\n", 2, 1, fp2);
		::fclose(fp2);

		e.import_file(mb->base());
		//e.dump();
		//e.dump_body();

		std::set< std::string > url_set;
		//MIME_Analyzer::get_url(e, url_set);
		//MIME_Analyzer::get_phone(e);
		MIME_Analyzer::dump_body(e); // also log utf-8 txt in this function
		//getchar();

		///*
		// handle dup pattern hash
		ACE_UINT32 hash_val = MIME_Analyzer::get_hash(e);
		if ( pat_set.find(hash_val) != pat_set.end() )
		{
			std::string dup_file(ACE::basename(mb->base(), '/'));
			char prefix[10];
			::sprintf(prefix, "%.8X_", hash_val);
			prefix[9] = 0;
			dup_file = prefix + dup_file;
			dup_file = "d:/_spam_/_dup_/" + dup_file;

			//ACE_OS::rename(mb->base(), dup_file.c_str());
			ACE_OS::unlink(mb->base());

			++n_dup;
			//::printf("%s\n", dup_file.c_str());
		}
		else
		{
			pat_set.insert(hash_val);
		}
		//*/

		// display url_set
		if ( !url_set.empty() )
		{
			//printf("n_url: %d\n", url_set.size());
			std::set< std::string >::iterator iter;
			for(iter = url_set.begin(); iter != url_set.end(); ++iter)
			{
				//printf("%s\n", (*iter).c_str());
				fwrite((*iter).c_str(), (*iter).size(), 1, fp);
				fwrite("\n", 1, 1, fp);
			}
			fwrite("\n", 1, 1, fp);

			if ( url_set.size() == 1 ) ++n_eml_http_1;
			else ++n_eml_http_n;
		}
		else
		{
			//if ( MIME_Util::get_phone(e) > 0 ) ++n_eml_phone;
		}
		
		/*
		ACE_DEBUG(( 
			LM_DEBUG, 
			ACE_TEXT("(%t) %s\n"), 
			e.get_content_type().c_str()
			));
		//*/

		delete mb;

		//ACE_OS::sleep(1);
		//getchar();
	}

	printf("n_eml_http_1:%d\n", n_eml_http_1);
	printf("n_eml_http_n:%d\n", n_eml_http_n);
	printf("n_eml_http_total:%d\n", n_eml_http_1 + n_eml_http_n);
	printf("n_eml_phone:%d\n", n_eml_phone);
	fclose(fp);

	printf("n_dup:%d\n", n_dup);

	return 0;
}