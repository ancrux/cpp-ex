#ifndef _LOG_STORE_H_
#define _LOG_STORE_H_

#include "ace/OS.h"
#include "ace/Dirent.h"

#include <string>
#include <map>

namespace mozart {

class Log_Store
{
public:
	typedef std::map< size_t, std::pair< std::string, size_t > > LOG_FILES;

public:
	static int is_log_filename(const char* filename, const std::string& prefix, const std::string& suffix);
	static void round_tm(int rotate_time, tm& tm_val);
	static const char* get_file_pattern(int rotate_time);

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
			MONTH = 86400 * 30,
			YEAR = 86400 * 365
		};
	};

public:
	Log_Store(const char* dir, const char* prefix = "", const char* suffix = "log");
	virtual ~Log_Store();
	int id() { return id_; };

public:
	int log(const char* buf, size_t len);
	int log(const char* cstr)
	{
		size_t len = ACE_OS::strlen(cstr);
		return log(cstr, len);
	};
	int log(const std::string& str)
	{
		return log(str.c_str(), str.size());
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
	int init_(size_t request_size = 0); // init_
	int fini_(); // fini_

protected:
	ACE_HANDLE fh_; // current log file handle
	ACE_Time_Value time_; // current log file start time

	std::string dir_; // log base directory
	std::string prefix_; // log file prefix
	std::string suffix_; // log file suffix
	LOG_FILES files_;
	//size_t n_total_; // total bytes of all log files
	ACE_UINT64 n_total_; // total bytes of all log files
	size_t n_file_; // bytes of current file

	int rotate_time_; // rotation base on DAY, MONTH, YEAR, etc.
	size_t rotate_size_; // 0 for no rotate
	//size_t store_max_size_; // 0 for no limit
	ACE_UINT64 store_max_size_; // 0 for no limit
	size_t file_max_size_; // 0 for no limit

	//char path[MAXPATHLEN+1];

	static int id_counter_;
	int id_;
};

} // namepsace mozart

#endif // _LOG_STORE_H_