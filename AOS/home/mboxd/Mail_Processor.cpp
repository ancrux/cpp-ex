#include "Mail_Processor.h"

#include "aos/OS.h"
#include "aos/String.h"
#include "aos/mime/MIME_Util.h"
using namespace aos;

#include "sqlite/KompexSQLitePrerequisites.h"
#include "sqlite/KompexSQLiteDatabase.h"
#include "sqlite/KompexSQLiteStatement.h"
#include "sqlite/KompexSQLiteException.h"
using namespace Kompex;

void
Mail_Processor::start(int n_thread)
{
	n_busy_thread_ = 0;

	if ( n_thread < 1 )
		n_thread = ACE_OS::num_processors_online();

	this->activate(THR_NEW_LWP | THR_JOINABLE, n_thread);
}

void
Mail_Processor::stop()
{
	size_t n_thread = this->thr_count();
	for(size_t i=0; i<n_thread; ++i)
		this->putq(new ACE_Message_Block());

	this->wait();

	n_busy_thread_ = 0;
}

int
Mail_Processor::svc(void)
{
	ACE_OS::printf("mp(%u) started...\n", ACE_OS::thr_self()); //@

	const std::string dir_sep(1, ACE_DIRECTORY_SEPARATOR_CHAR);

	// get configuration
	char path[PATH_MAX+1];
	ACE_OS::realpath(cfg.path_mbox_base.c_str(), path);
	std::string path_mbox_base = path;

	ACE_Message_Block* mb;
	MIME_Entity e;

	while( this->getq(mb) != -1 )
	{
		Scope_Free< ACE_Message_Block* > mb_freer(mb);
		Scope_Counter< ACE_Atomic_Op<ACE_Thread_Mutex, long> > busy_counter(n_busy_thread_);
		//++n_busy_thread_;

		if ( mb->size() == 0 )
		{
			//delete mb;
			//--n_busy_thread_;
			break;
		}
		//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) %s\n"), (mb->base())));

		// get mbox_id
		aos::Multi_String mstr;
		mstr.explode('.', ACE::basename(mb->base()));
		std::string mbox_id = mstr[0];
		std::string path_mbox_user = path_mbox_base + dir_sep + mbox_id;
		std::string path_mbox_user_mail = path_mbox_user + dir_sep + "mail";
		
		// open mbox.db
		std::string path_mbox_user_db = path_mbox_user + dir_sep + "mbox.db";
		ACE_OS::printf("mbox_user_db: %s\n", path_mbox_user_db.c_str()); //@

		try {
		SQLiteDatabase* db = new SQLiteDatabase(path_mbox_user_db.c_str(), SQLITE_OPEN_READWRITE, 0);
		SQLiteStatement* stmt = new SQLiteStatement(db);

		// parse *.done
		FILE* fp = ACE_OS::fopen(mb->base(), "rb");
		static const int LINE_MAX = 1024;
		char line[LINE_MAX+1];
		while( ACE_OS::fgets(line, LINE_MAX, fp) )
		{
			if ( line[0] != '@' ) continue;

			mstr.explode_token(" \r\n", line);

			if ( ACE_OS::strcasecmp("@mail", mstr[0]) == 0 )
			{
				std::string m_id = ACE::dirname(mstr[1], '.');
				std::string m_folder = mstr[2];
				std::string path_spool_mail = ACE::dirname(mb->base()) + dir_sep + mstr[1]; // "c:/app/data/spool/";
				
				aos::MIME_Entity e;
				int n_byte = e.import_file(path_spool_mail.c_str(), MIME_Entity::Flag::ALL);
				if ( n_byte < 0 ) continue;
				// filesize
				ACE_OFF_T m_size = ACE_OS::filesize(path_spool_mail.c_str());
				//ACE_OS::printf("mail@%s\n", path_spool_mail.c_str()); //@

				aos::MIME_Header_Util hu;
				aos::MIME_Header_List::iterator it;

				std::string m_sender;
				it = e.find_header("from", e.header().begin());
				if ( it != e.header().end() ) {
					m_sender = hu.copy_field_value(*(*it));
					hu.parse_encoded(m_sender);
					hu.build_utf8_decoded(m_sender);
				}

				std::string m_subject;
				it = e.find_header("subject", e.header().begin());
				if ( it != e.header().end() ) {
					m_subject = hu.copy_field_value(*(*it));
					hu.parse_encoded(m_subject);
					hu.build_utf8_decoded(m_subject);
					//ACE_OS::printf("Subject: %s\n", m_subject.c_str()); //@
				}

				std::string m_date;
				it = e.find_header("date", e.header().begin());
				if ( it != e.header().end() ) {
					m_date = hu.copy_field_value(*(*it));
				}
				MIME_Date date(m_date);
				time_t m_gmt_time = date.gmt_mktime();

				// move eml to mbox/mbox_id/mail
				std::string path_user_mail = path_mbox_user_mail + dir_sep + m_id + ".eml";
				int move_error = 0;
				move_error = ACE_OS::rename(path_spool_mail.c_str(), path_user_mail.c_str());
				if ( move_error == 0 ) {
					// if move ok, insert summary into mbox_db
					try {
						// use Capital 'U'nread for newly coming message
						// will change to 'u'nread for re-mark later on
						stmt->Sql("INSERT INTO mbox (m_unseen, m_id, m_folder, m_sender, m_subject, m_date, m_size) VALUES ('U', ?, ?, ?, ?, datetime(?, 'unixepoch'), ?);"); // insert GMT time
						stmt->BindString(1, m_id);
						stmt->BindString(2, m_folder);
						stmt->BindString(3, m_sender);
						stmt->BindString(4, m_subject);
						stmt->BindInt64(5, m_gmt_time);
						stmt->BindInt64(6, m_size);
						stmt->ExecuteAndFree();
					}
					catch(SQLiteException& e) {
						//e.Show();
						(void) e;
					}
				}
			}
		}
		if ( fp ) ACE_OS::fclose(fp);

		delete stmt;
		delete db;
		}
		catch(SQLiteException& e)
		{
			e.Show();
		}

		// check error flag, if no error, backup *.done
		static const int MAX_RENAME_TRY = 256;
		for(int i = 0; i < MAX_RENAME_TRY; ++i) {
			char file_ext[PATH_MAX+1]; // new *.done file extension
			ACE_OS::snprintf(file_ext, PATH_MAX, ".%8.8x_%d.done", (ACE_UINT32) ACE_OS::time(), i);
			std::string path_done_bak = path_mbox_user_mail + dir_sep + mbox_id + file_ext;
			//ACE_OS::printf("done_bak: %s\n", path_done_bak.c_str()); //@

			if ( path_exists(path_done_bak.c_str()) != 0 ) // file already exists
				continue;

			int move_error = 0;
			move_error = ACE_OS::rename(mb->base(), path_done_bak.c_str());
			if ( move_error == 0 )
				break;
		}


		//delete mb;
		//--n_busy_thread_;
	}

	ACE_OS::printf("mp(%u) stopped...\n", ACE_OS::thr_self()); //@

	return 0;
}
