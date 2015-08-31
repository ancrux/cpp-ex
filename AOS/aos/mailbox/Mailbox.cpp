#include "aos/mailbox/Mailbox.h"

#include "aos/mime/MIME_Util.h"

namespace aos {
namespace mailbox {

Mailbox::Mailbox()
{
}

Mailbox::~Mailbox()
{
	this->close();
}

int
Mailbox::open(const char* e_addr)
{
	addr_.assign(e_addr);

	int rc = select();
	if ( rc != 0 ) addr_.clear();

	return rc;
}

int
Mailbox::close()
{
	addr_.clear();
	mbox_.clear();
	return 0;
}

int 
Mailbox::select(const char* mbox)
{
	std::string path(addr_);
	if ( ACE_OS::strlen(mbox) > 0 )
	{
		path += ACE_DIRECTORY_SEPARATOR_CHAR;
		path += mbox;
	}

	ACE_stat stat;
	if ( ACE_OS::lstat(path.c_str(), &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFDIR )
	{
		mbox_ = mbox;
		return 0;
	}

	mbox_.clear();
	return -1;
}

int
Mailbox::count()
{
	int count = -1;

	std::string path(addr_);
	if ( !mbox_.empty() )
	{
		path += ACE_DIRECTORY_SEPARATOR_CHAR;
		path += mbox_;
	}

	// open dir
	ACE_Dirent dir;
	ACE_DIRENT* d;

	if ( dir.open(path.c_str()) == -1 )
		return count;

	count = 0;
	while( (d = dir.read()) != 0 )
	{
		std::string file(path);
		file += ACE_DIRECTORY_SEPARATOR_CHAR;
		file += d->d_name;

		ACE_stat stat;
		if ( ACE_OS::lstat(file.c_str(), &stat) != -1
			&& (stat.st_mode & S_IFMT) == S_IFREG
			&& *(d->d_name) != '.' ) 
		{
			++count;
		}
	}
	dir.close();

	return count;
}

int
Mailbox::list()
{
	int count = -1;

	std::string path(addr_);
	if ( !mbox_.empty() )
	{
		path += ACE_DIRECTORY_SEPARATOR_CHAR;
		path += mbox_;
	}

	// open dir
	ACE_Dirent dir;
	ACE_DIRENT* d;

	if ( dir.open(path.c_str()) == -1 )
		return count;

	count = 0;
	while( (d = dir.read()) != 0 )
	{
		std::string file(path);
		file += ACE_DIRECTORY_SEPARATOR_CHAR;
		file += d->d_name;

		ACE_stat stat;
		if ( ACE_OS::lstat(file.c_str(), &stat) != -1
			&& (stat.st_mode & S_IFMT) == S_IFREG
			&& *(d->d_name) != '.' ) 
		{
			//std::string data;
			//data.resize(stat.st_size);
			//FILE* fp = ::fopen(file.c_str(), "rb");
			//::fread((char*) data.c_str(), stat.st_size, 1, fp);
			
			MIME_Entity e;

			for(int i = 0; i < 1; ++i)
			{
				//e.import_data((char*) data.c_str(), stat.st_size, MIME_Entity::Flag::ALL);
				e.import_file(file.c_str(), MIME_Entity::Flag::ALL);
				//e.import_file(file.c_str(), MIME_Entity::Flag::HEADER);
				//e.import_file(file.c_str(), MIME_Entity::Flag::NO_BODY);
				MIME_Util::decode_body(e);

				continue;
			}

			MIME_Header_List::iterator it = e.find_header("subject", e.header().begin());
			if ( it != e.header().end() )
			{
				//for(int i=0;i<10000;++i)
				//{

				MIME_Header_Util hu;
				hu.parse_encoded(*(*it), 1, 1);
				//std::string guess_cs = MIME_Util::guess_charset((*it)->c_str(), (*it)->size());
				//hu.map_charset_decoded("gb18030");

				std::string utf8;
				UnicodeString uhdr;

				//hu.build_unicode_decoded(uhdr);
				hu.build_utf8_decoded(utf8);

				//size_t colon = utf8.find_first_of(':');
				//hu.parse_address(utf8.substr(colon+1));

				//}

				//::wprintf(L"%s", uhdr.getTerminatedBuffer());
				//ACE_OS::printf("%s", (*it)->c_str());

				////hu.build_address(utf8);

				UErrorCode status = U_ZERO_ERROR;
				std::string hdr = MIME_Util::from_utf8(utf8, "big5", status);
				//std::string hdr = MIME_Util::from_unicode(uhdr, "big5", status);
				ACE_OS::printf("%s\n", hdr.c_str());
				
			}

			++count;
			//ACE_OS::printf("%s\n", file.c_str()); //@
		}
	}
	dir.close();

	return count;
}

} // namespace mailbox
} // namespace aos

