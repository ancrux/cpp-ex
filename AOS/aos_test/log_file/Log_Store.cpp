#include "Log_Store.h"

namespace mozart {

int Log_Store::id_counter_ = 0;

Log_Store::Log_Store(const char* dir, const char* prefix, const char* suffix)
:
fh_(ACE_INVALID_HANDLE),
n_total_(0),
n_file_(0),
id_(id_counter_++)
{
	if ( dir ) dir_.assign(dir);
	if ( prefix ) prefix_.assign(prefix);
	if ( suffix ) suffix_.assign(suffix);
	if ( prefix_.empty() && suffix_.empty() ) prefix_ = "log";

	this->rotate_time_ = Log_Store::Rotate::SECOND;
	this->rotate_size_ = 10;
	this->store_max_size_ = 0;
	this->file_max_size_ = 4096;

	init_();
}

Log_Store::~Log_Store()
{
	fini_();
}

int
Log_Store::log(const char* buf, size_t len)
{
	// check rotate
	ACE_Time_Value now = ACE_OS::gettimeofday();
	if ( this->rotate_time_ && now.sec() - time_.sec() >= this->rotate_time_ )
		this->init_();

	// check store max size
	if ( this->store_max_size_ && this->n_total_ + len > this->store_max_size_ )
	{
		this->init_(len);
	}

	ssize_t n_byte = ACE_OS::write(this->fh_, buf, len);
	if ( n_byte > 0 ) 
	{
		this->n_total_ += n_byte;
		this->n_file_ += n_byte;
	}
	else
	{
		ACE_OS::printf("write err: %d\n", n_byte);
	}

	return (int) n_byte;
}

int
Log_Store::scan_store()
{
	files_.clear();
	n_total_ = 0;

	// dirent
	ACE_Dirent dir;
	ACE_DIRENT* d;
	std::string path;

	// scan base dir
	dir.open(dir_.c_str());
	while( (d = dir.read()) != 0 )
	{
		ACE_stat stat;

		path = dir_;
		path += "/";
		path += d->d_name;

		// check is log filename
		if ( ACE_OS::lstat(path.c_str(), &stat) != -1 &&
			(stat.st_mode & S_IFMT) == S_IFREG &&
			Log_Store::is_log_filename(d->d_name, this->prefix_, this->suffix_) ) 
		{
			n_total_ += (size_t) stat.st_size;
			files_.insert(std::make_pair((size_t) stat.st_ctime, std::make_pair(d->d_name, (size_t) stat.st_size)));
			//ACE_OS::printf("SCAN ctime:%d file:%s size:%d\n", (size_t) stat.st_ctime, d->d_name, (size_t) stat.st_size);
		}
	}
	dir.close();

	//display_all_files();

	return 0;
}

int
Log_Store::rotate_store(size_t request_size)
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
			if ( expired.sec() > (*iter).first )
			{
				ACE_OS::printf("ROTATE ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second);
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
				ACE_OS::printf("STORE_MAX ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second);
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
Log_Store::init_(size_t request_size)
{
	this->open_log();
	this->scan_store();

	display_log_files();

	rotate_store();

	return 0;
}

int
Log_Store::fini_()
{
	return this->close_log();
}

int
Log_Store::open_log()
{
	// close any previous opened log
	this->close_log();

	// open log file
	this->time_ = ACE_OS::gettimeofday();
	time_t t_sec;
	struct tm tm_val;
	
	// day-based rotate
	t_sec = (this->time_).sec();
	ACE_OS::localtime_r(&t_sec, &tm_val);
	Log_Store::round_tm(this->rotate_time_, tm_val);

	// adjust time
	t_sec = ACE_OS::mktime(&tm_val);
	this->time_.sec(t_sec);
	ACE_OS::localtime_r(&t_sec, &tm_val);

	char date_buf[128];
	//ACE_OS::strftime(date_buf, 128, "%Y-%m-%d_%H-%M-%S", &tm_val);
	ACE_OS::strftime(date_buf, 128, Log_Store::get_file_pattern(this->rotate_time_), &tm_val);
	ACE_OS::printf("%s\n", date_buf); //@

	std::string path(dir_);
	path += "/";
	path += prefix_;
	path += date_buf;
	path += ".";
	path += suffix_;

	this->fh_ = ACE_OS::open(path.c_str(), O_CREAT | O_WRONLY | O_APPEND);
	if ( this->fh_ == ACE_INVALID_HANDLE )
	{
		return ACE_OS::last_error();
	}
	else
	{
		//this->n_file_ = ACE_OS::lseek(this->fh_, 0, SEEK_CUR);

		ACE_stat stat;
		ACE_OS::lstat(path.c_str(), &stat);

		n_file_ = stat.st_size;
	}

	return 0;
}

int
Log_Store::close_log()
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

Log_Store::LOG_FILES::iterator
Log_Store::remove_log(LOG_FILES::iterator iter)
{
	if ( iter == files_.end() ) return files_.end();

	std::string path(dir_);
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
Log_Store::display_log_files()
{
	for(LOG_FILES::iterator iter = files_.begin();
		iter != files_.end();
		++iter)
	{
		//ACE_OS::printf("size:%d\n", ((*iter).second).second);
		ACE_OS::printf("ctime:%d file:%s size:%d\n", (*iter).first, ((*iter).second).first.c_str(), ((*iter).second).second);
	}
};

int
Log_Store::is_log_filename(const char* filename, const std::string& prefix, const std::string& suffix)
{
	// check matched prefix
	if ( ACE_OS::strncasecmp(prefix.c_str(), filename, prefix.size()) == 0 )
	{
		// check matched suffix
		size_t pos = strlen(filename);
		pos -= suffix.size(); if ( pos < 0 ) pos = 0;
		const char* suff = filename + pos;
		if ( ACE_OS::strncasecmp(suffix.c_str(), suff, suffix.size()) == 0 )
		{
			return 1;
		}
	}

	return 0;
}

void
Log_Store::round_tm(int rotate_time, tm& tm_val)
{
	if ( rotate_time > 0 )
	{
		if ( rotate_time > Log_Store::Rotate::SECOND )
			tm_val.tm_sec = 0;
		if ( rotate_time > Log_Store::Rotate::MINUTE )
			tm_val.tm_min = 0;
		if ( rotate_time > Log_Store::Rotate::HOUR )
			tm_val.tm_hour = 0;
		if ( rotate_time > Log_Store::Rotate::DAY )
			tm_val.tm_mday = 0;
		if ( rotate_time > Log_Store::Rotate::MONTH )
			tm_val.tm_mon = 0;
	}
}

const char*
Log_Store::get_file_pattern(int rotate_time)
{
	static std::string pat;
	pat.resize(0);

	if ( rotate_time > 0 )
	{
		pat += "%Y"; // YYYY
		if ( rotate_time < Log_Store::Rotate::YEAR )
			pat += "-%m"; // mm
		if ( rotate_time < Log_Store::Rotate::MONTH )
			pat += "-%d"; // dd
		if ( rotate_time < Log_Store::Rotate::DAY )
			pat += "_%H"; // HH
		if ( rotate_time < Log_Store::Rotate::HOUR )
			pat += "-%M"; // MM
		if ( rotate_time < Log_Store::Rotate::MINUTE )
			pat += "-%S"; // SS
	}

	return pat.c_str();
}

} // namespace mozart