#include "Mailbox_SQLite.h"

#include "ace/OS.h"
#include "ace/UUID.h"

#include "aos/mime/MIME_Util.h"

#include "sqlite/sqlite3.h"

namespace aos {

ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long>
Mailbox_SQLite::counter_(0);

Mailbox_SQLite::Mailbox_SQLite(const char* base_dir)
:
base_dir_(base_dir)
{
	// Initialize the UUID Generator
	ACE_Utils::UUID_GENERATOR::instance()->init();
}

Mailbox_SQLite::~Mailbox_SQLite()
{
}

// ID generator

std::string
Mailbox_SQLite::generate_mailbox_id()
{
	// performance: 1,000,000 calls take 4562ms on Win32
	// 110ms for ACE_OS::gettimeofday(); 47ms for ACE_OS::time();

	// Generate UUID as Mailbox ID
	ACE_Utils::UUID uuid; 
	ACE_Utils::UUID_GENERATOR::instance()->generate_UUID(uuid);

	return (uuid.to_string())->c_str();
}

std::string
Mailbox_SQLite::generate_mail_id(const char* eml_path)
{
	// performance: 1,000,000 calls take 2140ms on Win32
	// without ACE_OS::snprintf() only takes 203ms

	// Generate tv.sec()_tv.usec()_counter as Mail ID
	++counter_;
	ACE_Time_Value tv = ACE_OS::gettimeofday();

	char buf[64];
	// printf format "%16.16x" (8-byte) doesn't work, use "%8.8x" (4-byte) instead!
	ACE_OS::snprintf(buf, 63, "%8.8x_%8.8x_%8.8x", (ACE_UINT32) tv.sec(), (ACE_UINT32) tv.usec(), (ACE_UINT32) counter_.value()); // bottle-neck

	return std::string(buf);
}

// Mailbox

std::string
Mailbox_SQLite::create_mailbox()
{
	std::string mailbox_id = this->generate_mailbox_id();
	
	std::string path = base_dir_ + ACE_DIRECTORY_SEPARATOR_CHAR;
	path += mailbox_id;

	return (ACE_OS::mkdir(path.c_str()) == 0)?mailbox_id:"";
}

int
Mailbox_SQLite::destroy_mailbox(const std::string& mailbox_id)
{
	std::string path = base_dir_ + ACE_DIRECTORY_SEPARATOR_CHAR;
	path += mailbox_id;

	return (ACE_OS::rmdir(path.c_str()) == 0)?0:-1;
}

// Mail Address

int
Mailbox_SQLite::link_address_to_mailbox(const std::string& address, const std::string& mailbox_id)
{
	// sqlite
	std::string addr_path(base_dir_);
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "mbox";
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "addr.db";

	sqlite3* db;

	int rc_open = ::sqlite3_open_v2(addr_path.c_str(), &db, SQLITE_OPEN_READWRITE, 0);
	if ( rc_open != 0 )
	{
		// initialize addr db
		::sqlite3_close(db);
		::sqlite3_open(addr_path.c_str(), &db);

		std::string stmt;
		stmt =	"CREATE TABLE addr (" \
				"email VARCHAR(255)," \
				"mbox_id VARCHAR(255)," \
				"PRIMARY KEY(email, mbox_id)" \
				");";
		::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);
	}

	std::string stmt;
	stmt =	"INSERT INTO addr (email, mbox_id)" \
			"VALUES ('" + address + "', '" + mailbox_id + "')";
	int rc_exec = ::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);

	::sqlite3_close(db);

	return rc_exec;
}

std::string
Mailbox_SQLite::unlink_address_to_mailbox(const std::string& address)
{
	// sqlite
	std::string addr_path(base_dir_);
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "mbox";
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "addr.db";

	sqlite3* db;

	int rc_open = ::sqlite3_open_v2(addr_path.c_str(), &db, SQLITE_OPEN_READWRITE, 0);
	if ( rc_open != 0 )
	{
		::sqlite3_close(db);
		return "";
	}

	std::string stmt;
	stmt =	"DELETE FROM addr " \
			"WHERE email = '" + address + "'";
	int rc_exec = ::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);

	::sqlite3_close(db);

	return (rc_exec == 0)?address:"";
}

std::string
Mailbox_SQLite::get_mailbox_from_address(const std::string& address)
{
	// sqlite
	std::string addr_path(base_dir_);
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "mbox";
	addr_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	addr_path += "addr.db";

	sqlite3* db;

	int rc_open = ::sqlite3_open_v2(addr_path.c_str(), &db, SQLITE_OPEN_READWRITE, 0);
	if ( rc_open != 0 )
	{
		::sqlite3_close(db);
		return "";
	}

	std::string stmt;
	stmt =	"SELECT mbox_id FROM addr " \
			"WHERE email = '" + address + "'";
	int rc_exec = ::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);

	::sqlite3_close(db);

	return (rc_exec == 0)?address:"";
}


// Mail

std::string
Mailbox_SQLite::insert_mail(const char* eml_path, const std::string& mailbox_ids)
{
	aos::MIME_Entity e;
	e.import_file(eml_path, MIME_Entity::Flag::ALL);

	aos::MIME_Header_Util hu;
	aos::MIME_Header_List::iterator it;

	std::string m_from;
	it = e.find_header("from", e.header().begin());
	if ( it != e.header().end() )
	{
		m_from = *(*it);
		hu.parse_encoded(m_from);
		hu.build_utf8_decoded(m_from);
	}
	//ACE_OS::printf("%s\n", m_from.c_str());

	std::string m_to;
	it = e.find_header("to", e.header().begin());
	if ( it != e.header().end() )
	{
		m_to = *(*it);
		hu.parse_encoded(m_to);
		hu.build_utf8_decoded(m_to);
	}
	//ACE_OS::printf("%s\n", m_to.c_str());

	std::string m_cc;
	it = e.find_header("cc", e.header().begin());
	if ( it != e.header().end() )
	{
		m_cc = *(*it);
		hu.parse_encoded(m_cc);
		hu.build_utf8_decoded(m_cc);
	}
	//ACE_OS::printf("%s\n", m_cc.c_str());

	std::string m_subject;
	it = e.find_header("subject", e.header().begin());
	if ( it != e.header().end() )
	{
		m_subject = *(*it);
		hu.parse_encoded(m_subject);
		hu.build_utf8_decoded(m_subject);
	}
	//ACE_OS::printf("%s\n", m_subject.c_str());

	std::string m_date;
	it = e.find_header("date", e.header().begin());
	if ( it != e.header().end() )
	{
		m_date = *(*it);
	}
	//ACE_OS::printf("%s\n", m_date.c_str());

	// sqlite
	std::string mbox_path(base_dir_);
	mbox_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	mbox_path += "mbox";
	mbox_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	//mbox_path += "00000000-0000-0000-0000-000000000000";
	//mbox_path += ACE_DIRECTORY_SEPARATOR_CHAR;
	mbox_path += "mbox.db";

	sqlite3* db;

	int rc_open = ::sqlite3_open_v2(mbox_path.c_str(), &db, SQLITE_OPEN_READWRITE, 0);
	if ( rc_open != 0 )
	{
		// initialize mbox db
		::sqlite3_close(db);
		::sqlite3_open(mbox_path.c_str(), &db);

		std::string stmt;
		stmt =	"CREATE TABLE meta (" \
				"m_id CHAR(26)," \
				"m_path VARCHAR(255)," \
				"m_tags VARCHAR(255)," \
				"m_sender VARCHAR(255)," \
				"m_subject VARCHAR(255)," \
				"m_attach VARCHAR(255)," \
				"m_time DATETIME," \
				"m_size INTEGER," \
				"PRIMARY KEY(m_id, m_path)" \
				");";
		::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);
	}

	// insert mail meta
	std::string m_id = this->generate_mail_id(eml_path);

	//std::string stmt;
	//stmt =	"INSERT INTO meta (m_id, m_sender, m_subject) VALUES " \
	//		"('" + m_id + "', '" + m_from + "', '" + m_subject + "')";
	//::sqlite3_exec(db, stmt.c_str(), 0, 0, 0);

	::sqlite3_close(db);
	
	return "";
}

int
Mailbox_SQLite::remove_mail(const std::string& mailbox_id, const std::string& mail_ids)
{
	return 0;
}

} // namespace aos

