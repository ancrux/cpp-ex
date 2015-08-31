#include "Logger.h"

namespace aos {

int Logger::id_counter_ = 0;

int
Logger::is_log_filename(const char* filename, const std::string& prefix, const std::string& suffix)
{
	// check matched prefix
	if ( ACE_OS::strncasecmp(filename, prefix.c_str(), prefix.size()) == 0 )
	{
		// check matched suffix
		size_t pos = strlen(filename);
		pos -= suffix.size(); if ( pos < 0 ) pos = 0;
		const char* suff = filename + pos;
		if ( ACE_OS::strncasecmp(suff, suffix.c_str(), suffix.size()) == 0 )
		{
			return 1;
		}
	}

	return 0;
}

void
Logger::round_tm(int rotate_time, tm& tm_val)
{
	if ( rotate_time > 0 )
	{
		if ( rotate_time > Logger::Rotate::SECOND )
			tm_val.tm_sec = 0;
		if ( rotate_time > Logger::Rotate::MINUTE )
			tm_val.tm_min = 0;
		if ( rotate_time > Logger::Rotate::HOUR )
			tm_val.tm_hour = 0;
		if ( rotate_time > Logger::Rotate::DAY )
			tm_val.tm_mday = 0;
		if ( rotate_time > Logger::Rotate::MONTH )
			tm_val.tm_mon = 0;
	}
}

std::string
Logger::log_time_format(int rotate_time)
{
	std::string pat;
	pat.resize(0);

	if ( rotate_time > 0 )
	{
		pat += "%Y"; // YYYY
		if ( rotate_time < Logger::Rotate::YEAR )
			pat += "-%m"; // mm
		if ( rotate_time < Logger::Rotate::MONTH )
			pat += "-%d"; // dd
		if ( rotate_time < Logger::Rotate::DAY )
			pat += "_%H"; // HH
		if ( rotate_time < Logger::Rotate::HOUR )
			pat += "-%M"; // MM
		if ( rotate_time < Logger::Rotate::MINUTE )
			pat += "-%S"; // SS
	}

	return pat;
}

int
Logger::str_to_rotate_time(const char *cstr)
{
	if ( ACE_OS::strncasecmp(cstr, "second", 6) == 0 )
		return Logger::Rotate::SECOND;
	else if ( ACE_OS::strncasecmp(cstr, "minute", 6) == 0 )
		return Logger::Rotate::MINUTE;
	else if ( ACE_OS::strncasecmp(cstr, "hour", 4) == 0 )
		return Logger::Rotate::HOUR;
	else if ( ACE_OS::strncasecmp(cstr, "day", 3) == 0 )
		return Logger::Rotate::DAY;
	else if ( ACE_OS::strncasecmp(cstr, "month", 5) == 0 )
		return Logger::Rotate::MONTH;
	else if ( ACE_OS::strncasecmp(cstr, "year", 4) == 0 )
		return Logger::Rotate::YEAR;

	return Logger::Rotate::NONE;
}

Logger::Logger(
	const char* dir,
	const char* prefix,
	const char* suffix,
	int rotate_time,
	size_t rotate_size,
	ACE_UINT64 store_max_size,
	ACE_UINT64 file_max_size
	)
:
fh_(ACE_INVALID_HANDLE),
n_total_(0),
n_file_(0),
rotate_time_(rotate_time),
rotate_size_(rotate_size),
store_max_size_(store_max_size),
file_max_size_(file_max_size),
id_(id_counter_++)
{
	mstr_.push_back((dir)?dir:".");
	mstr_.push_back((prefix)?prefix:"");
	mstr_.push_back((suffix)?suffix:"");
	if ( mstr_.size(1) == 0 && mstr_.size(2) == 0 )
		mstr_.replace(2, "log");

	init();
}

Logger::~Logger()
{
	fini();
}

int
Logger::init(size_t request_size)
{
	this->open_log();
	this->scan_store();

	//display_log_files();

	rotate_store(request_size);

	return 0;
}

int
Logger::fini()
{
	return this->close_log();
}

int
Logger::scan_store()
{
	files_.clear();
	n_total_ = 0;

	// dirent
	ACE_Dirent dir;
	ACE_DIRENT* d;
	std::string path;

	// scan base dir
	dir.open(this->dir());
	while( (d = dir.read()) != 0 )
	{
		ACE_stat stat;

		path = this->dir();
		path += "/";
		path += d->d_name;

		// check is log filename
		if ( ACE_OS::lstat(path.c_str(), &stat) != -1 &&
			(stat.st_mode & S_IFMT) == S_IFREG &&
			Logger::is_log_filename(d->d_name, this->prefix(), this->suffix()) ) 
		{
			n_total_ += (size_t) stat.st_size;
			files_.insert(std::make_pair((size_t) stat.st_ctime, std::make_pair(d->d_name, (size_t) stat.st_size)));
			//ACE_OS::printf("SCAN ctime:%d file:%s size:%d\n", (size_t) stat.st_ctime, d->d_name, (size_t) stat.st_size);
		}
	}
	dir.close();

	//ACE_OS::printf("n_total_: %u\n", this->n_total_); //@
	//ACE_OS::printf("n_file_: %u\n", this->n_file_); //@
	//display_all_files(); //@

	return 0;
}

int
Logger::rotate_store(size_t request_size)
{
	// enforce rotate size policy
	if ( this->rotate_time_ && this->rotate_size_ )
	{
		ACE_Time_Value expired = ACE_OS::gettimeofday();
		expired -= this->rotate_time_ * this->rotate_size_;

		for(LOG_FILES::iterator iter = files_.begin();
			iter != files_.end();
			)
		{
			if ( (size_t) expired.sec() > (*iter).first )
			{
				//ACE_OS::printf("ROTATE ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second); //@
				iter = remove_log(iter);
			}
			else
			{
				break;
			}
		}

	}

	// enforce store max size policy
	if ( this->store_max_size_ )
	{
		for(LOG_FILES::iterator iter = files_.begin();
			iter != files_.end();
			)
		{
			if ( n_total_ + request_size > this->store_max_size_ )
			{
				//ACE_OS::printf("STORE_MAX ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second); //@
				iter = remove_log(iter);
			}
			else
			{
				break;
			}
		}
	}

	// if no log file, restart
	if ( files_.empty() )
	{
		open_log();
		scan_store();
	}

	return 0;
}

int
Logger::open_log()
{
	// close any previous opened log
	this->close_log();
	n_file_ = 0;

	// open log file
	this->ftime_ = ACE_OS::gettimeofday();
	time_t t_sec;
	struct tm tm_val;
	
	// day-based rotate
	t_sec = (this->ftime_).sec();
	ACE_OS::localtime_r(&t_sec, &tm_val);
	Logger::round_tm(this->rotate_time_, tm_val);

	// adjust time
	t_sec = ACE_OS::mktime(&tm_val);
	this->ftime_.sec(t_sec);
	ACE_OS::localtime_r(&t_sec, &tm_val);

	char date_buf[128];
	//ACE_OS::strftime(date_buf, 128, "%Y-%m-%d_%H-%M-%S", &tm_val);
	std::string pattern = Logger::log_time_format(this->rotate_time_);
	ACE_OS::strftime(date_buf, 128, pattern.c_str(), &tm_val);
	//ACE_OS::printf("%s\n", date_buf); //@

	std::string path(this->dir());
	path += "/";
	path += this->prefix();
	path += date_buf;
	if ( this->file_max_size_ )
	{
		ACE_UINT32 n_stamp = (ACE_UINT32) this->ftime_.msec();
		char stamp[10];
		ACE_OS::sprintf(stamp, ".%.8X", n_stamp);
		path += stamp;
	}
	path += ".";
	path += this->suffix();

	int perms = ACE_DEFAULT_OPEN_PERMS;
#ifdef ACE_WIN32
	perms = FILE_SHARE_READ | FILE_SHARE_WRITE;
#endif
	this->fh_ = ACE_OS::open(path.c_str(), O_BINARY | O_WRONLY | O_CREAT | O_APPEND, perms);
	if ( this->fh_ == ACE_INVALID_HANDLE )
	{
		return ACE_OS::last_error();
	}
	else
	{
		//this->n_file_ = ACE_OS::lseek(this->fh_, 0, SEEK_CUR);

		ACE_stat stat;
		ACE_OS::lstat(path.c_str(), &stat);
		this->n_file_ = stat.st_size;
	}

	return 0;
}

int
Logger::close_log()
{
	if ( this->fh_ != ACE_INVALID_HANDLE )
	{
		if ( ACE_OS::close(this->fh_) == 0 )
			this->fh_ = ACE_INVALID_HANDLE;
		else
			return ACE_OS::last_error();
	}

	return 0;
}

Logger::LOG_FILES::iterator
Logger::remove_log(LOG_FILES::iterator iter)
{
	if ( iter == files_.end() ) return files_.end();

	std::string path(this->dir());
	path += "/";
	path += ((*iter).second).first; // filename
	
	if ( ACE_OS::unlink(path.c_str()) == 0 )
	{
		//ACE_OS::printf("REMOVE ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second);
		n_total_ -= ((*iter).second).second;
		files_.erase(iter++);
	}

	return iter;
}

void
Logger::display_log_files()
{
	for(LOG_FILES::iterator iter = files_.begin();
		iter != files_.end();
		++iter)
	{
		//ACE_OS::printf("size:%d\n", ((*iter).second).second);
		ACE_OS::printf("ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second);
	}
}

ssize_t
Logger::log(const char* buf, size_t len, ACE_Time_Value* time, const char* time_format)
{
	ACE_Time_Value now;
	if ( !time )
	{
		now = ACE_OS::gettimeofday();
		time = &now; // time = &(ACE_OS::gettimeofday()); // cause warning: taking address of temporary on linux/unix
	}

	//? lock the whole log() or init() only
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, -1);

	// check rotate
	if ( this->rotate_time_ && time->sec() - this->ftime_.sec() >= this->rotate_time_ )
	{
		this->init();
	}
	// check file max size
	if ( this->file_max_size_ && this->n_file_ + len > this->file_max_size_ )
	{
		this->init(len);
	}
	// check store max size
	if ( this->store_max_size_ && this->n_total_ + len > this->store_max_size_ )
	{
		this->init(len);
	}

	ssize_t n_byte = 0;
	ssize_t n_write = 0;
	if ( time_format )
	{
		time_t t_sec = time->sec();
		struct tm tm_val;
		ACE_OS::localtime_r(&t_sec, &tm_val);

		char buf[128];
		size_t n_buf = ACE_OS::strftime(buf, 127, time_format, &tm_val);
		n_write = ACE_OS::write(this->fh_, buf, n_buf);
		if ( n_write > 0 )
			n_byte += n_write;
		else
		{
			//ACE_OS::printf("timestamp write err: %d\n", n_write); //@

			/*
			// try to open file and write again. if failed, return -1;
			do
			{
				this->open_log();

				n_write = ACE_OS::write(this->fh_, buf, len);
				if ( n_write > 0 ) 
				{
					n_byte += n_write;
					this->n_total_ += n_byte;
					this->n_file_ += n_byte;

					return n_byte;
				}
			}
			while(0);
			//*/

			return -1;
		}
	}

	n_write = ACE_OS::write(this->fh_, buf, len);
	if ( n_write > 0 ) 
	{
		n_byte += n_write;
		this->n_total_ += n_byte;
		this->n_file_ += n_byte;
	}
	else
	{
		//ACE_OS::printf("log buffer write err: %d\n", n_write); //@

		/*
		// try to open file and write again. if failed, return -1;
		do
		{
			if ( files_.empty() )
				break;

			LOG_FILES::iterator iter = (files_.end())--;
			std::string path((iter->second).first);

			int perms = ACE_DEFAULT_OPEN_PERMS;
#ifdef ACE_WIN32
			perms = FILE_SHARE_READ | FILE_SHARE_WRITE;
#endif
			ACE_OS::close(this->fh_);
			this->fh_ = ACE_OS::open(path.c_str(), O_BINARY | O_WRONLY | O_CREAT | O_APPEND, perms);
			if ( this->fh_ == ACE_INVALID_HANDLE )
				break;

			n_write = ACE_OS::write(this->fh_, buf, len);
			if ( n_write > 0 ) 
			{
				n_byte += n_write;
				this->n_total_ += n_byte;
				this->n_file_ += n_byte;

				return n_byte;
			}
		}
		while(0);
		//*/

		return -1;
	}

	return n_byte;
}

} // namespace aos
