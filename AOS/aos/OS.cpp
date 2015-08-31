#include "OS.h"

#include <cassert>

namespace aos {

const char* version()
{
	return AOS_VERSION;
}

const char* build_date()
{
	return __DATE__;
}

const char* build_time()
{
	return __TIME__;
}

const char* build_timestamp()
{
	return __TIMESTAMP__;
}

int path_exists(const char* path, int* is_link)
{
	assert(path);

	int rc = 0;
	if ( is_link ) *is_link = 0;

	ACE_stat stat;
	if ( ACE_OS::lstat(path, &stat) == 0 )
	{
		switch(stat.st_mode & S_IFMT)
		{
		case S_IFREG: // either a regular file or an executable.
			rc = 1;
			break;
		case S_IFDIR: // directory
			rc = 2;
			break;
		case S_IFLNK: // either a file or directory link, so let's find out
			if ( is_link ) *is_link = 1;
			if ( ACE_OS::stat(path, &stat) == 0 )
			{
				if ( (stat.st_mode & S_IFMT) == S_IFDIR ) // link to directory, don't recurse through symbolic directory links!
					rc = 2;
				else // link to file
					rc = 1;
				break;
			}
			// failed to stat() will fall thru to case default (return -1)
		default: // must be some other type of file (PIPE/FIFO/device)
			rc = -1;
			break;
		}
	}

	// PATH_NOT_EXISTS: 0
	// PATH_IS_FILE: 1
	// PATH_IS_DIR: 2
	// PATH_IS_UNKNOWN: -1

	return rc;
}

int mkdir(const char* path, bool recursive, mode_t mode)
{
	assert(path);

	if ( !recursive )
		return ACE_OS::mkdir(path, mode);

	int rc = 0;

	Multi_String mstr;
	mstr.explode("/\\", path); // explode() with ACE_DIRECTORY_SEPARATOR_CHAR
	if ( *path == '/' || *path == '\\' ) mstr.insert(0, ""); // because of explode(), add new empty root path for absoulte path
	size_t n = mstr.size();

	std::string dir;
	for(size_t i=0; i<n; ++i)
	{
		if ( i ) // paths except root
		{
			dir += ACE_DIRECTORY_SEPARATOR_CHAR;
			dir += mstr[i];

			//ACE_OS::printf("dir(%d, '%s')\n", path_exists(dir.c_str()), dir.c_str()); //@
			if ( path_exists(dir.c_str()) != 2 && ACE_OS::mkdir(dir.c_str(), mode) != 0 )
			{
				rc = -1;
				break;
			}
		}
		else // root path
		{
			dir += mstr[i]; // mstr[0]

			std::string root = dir;
			if ( root != "." && root != ".." )
				root += ACE_DIRECTORY_SEPARATOR_CHAR;
			char path[PATH_MAX+1];
			root = ACE_OS::realpath(root.c_str(), path);
			//ACE_OS::printf("dir(%d, '%s')\n", path_exists(root.c_str()), root.c_str()); //@
			if ( path_exists(root.c_str()) != 2 && ACE_OS::mkdir(root.c_str(), mode) != 0 )
			{
				rc = -1;
				break;
			}
		}
	}

	return rc;
}

int rmdir(const char* path, bool recursive, int stop_on_error)
{
	assert(path);

	if ( !recursive )
		return ACE_OS::rmdir(path);

	int rc = 0;

	ACE_DIRENT* d;
	ACE_Dirent dir;

	if ( dir.open(path) == 0 )
	{
		while( (d = dir.read()) != 0 )
		{
			if ( ACE_OS::strcmp(d->d_name, ".") == 0 || ACE_OS::strcmp(d->d_name, "..") == 0 )
				continue;

			std::string file(path); file += ACE_DIRECTORY_SEPARATOR_CHAR; file += d->d_name;
			if ( path_exists(file.c_str()) == 2 )
			{
				//ACE_OS::printf("D='%s'\n", file.c_str()); //@
				if ( rmdir(file.c_str(), recursive) != 0 && stop_on_error )
					break;
			}
			else
			{
				//ACE_OS::printf("F='%s'\n", file.c_str()); //@
				if ( ACE_OS::unlink(file.c_str()) != 0 ) // if failed, chmod() and try again!
				{
					//ACE_OS::printf("unlink failed=%s\nerr:%s\n", file.c_str(), ACE_OS::strerror(ACE_OS::last_error())); //@
					::chmod(file.c_str(), 0666);
					if ( ACE_OS::unlink(file.c_str()) != 0 && stop_on_error )
						break;
				}
			}
		}
		dir.close();
		rc = ACE_OS::rmdir(path);
		if ( rc != 0 ) // if failed, chmod() and try again!
		{
			::chmod(path, 0666);
			rc = ACE_OS::rmdir(path);
		}
	}
	else
		rc = -1;

	return rc;
}

} // namepsace aos
