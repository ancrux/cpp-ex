#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "aos/Config.h"

#include "aos/OS.h"
#include "aos/String.h"

#include <string>
#include <map>

namespace aos {

class AOS_API Logger
{
public:
	typedef std::map< size_t, std::pair< std::string, size_t > > LOG_FILES;

public:
	static int is_log_filename(const char* filename, const std::string& prefix, const std::string& suffix);
	static void round_tm(int rotate_time, tm& tm_val);
	static std::string log_time_format(int rotate_time);
	static int str_to_rotate_time(const char* cstr);

public:
	class Rotate
	{
	public:
		enum
		{
			NONE = 0,
			SECOND = 1,
			MINUTE = 60,
			HOUR = 3600,
			DAY = 86400,
			//WEEK = 86400 * 7,
			MONTH = 86400 * 30, // or * 31
			YEAR = 86400 * 365 // or * 366
		};
	};

public:
	Logger(
		const char* dir = ".",
		const char* prefix = "",
		const char* suffix = "log",
		int rotate_time = Logger::Rotate::NONE,
		size_t rotate_size = 0,
		ACE_UINT64 store_max_size = 0,
		ACE_UINT64 file_max_size = 0
		);
	virtual ~Logger();
	int id() { return id_; };

public:
	const char* dir() const { return mstr_[0]; };
	const char* prefix() const { return mstr_[1]; };
	const char* suffix() const { return mstr_[2]; };

public:
	ssize_t log(const char* buf, size_t len, ACE_Time_Value* time = 0, const char* time_format = "%Y-%m-%d %H:%M:%S\t");
	inline ssize_t log(const char* cstr, ACE_Time_Value* time = 0, const char* time_format = "%Y-%m-%d %H:%M:%S\t")
	{
		size_t len = ACE_OS::strlen(cstr);
		return log(cstr, len, time, time_format);
	};
	inline ssize_t log(const std::string& str, ACE_Time_Value* time = 0, const char* time_format = "%Y-%m-%d %H:%M:%S\t")
	{
		return log(str.c_str(), str.size(), time, time_format);
	};


public:
	int rotate_store(size_t request_size = 0); // rotate store
	int split_log(); // split log file

	int scan_store();
	int open_log();
	int close_log();
	LOG_FILES::iterator remove_log(LOG_FILES::iterator iter);

	void display_log_files();

protected:
	int init(size_t request_size = 0);
	int fini();
	
protected:
	ACE_HANDLE fh_; // current log file handle
	ACE_Time_Value ftime_; // current log file start time

	aos::Multi_String mstr_; // mstr_[0] = dir, mstr_[1] = prefix, mstr_[2] = suffix
	LOG_FILES files_;
	ACE_UINT64 n_total_; // total bytes of all log files
	ACE_UINT64 n_file_; // bytes of current file

	int rotate_time_; // rotation base on DAY, MONTH, YEAR, etc.
	size_t rotate_size_; // 0 for no rotate
	ACE_UINT64 store_max_size_; // 0 for no limit
	ACE_UINT64 file_max_size_; // 0 for no limit

	ACE_Thread_Mutex lock_;

	static int id_counter_;
	int id_;
};

} // namepsace aos

#endif // _LOGGER_H_
