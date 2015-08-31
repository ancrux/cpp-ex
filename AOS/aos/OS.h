#ifndef _OS_H_
#define _OS_H_

#include "aos/Config.h"

#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Dirent.h"
#include "ace/Dirent_Selector.h"

#include "aos/Codec.h"
#include "aos/String.h"

// namepsace aos::tmp is reserved for temporary class/function

namespace aos { // namespace aos: ACE OS Extension

extern "C" AOS_API const char* version();
extern "C" AOS_API const char* build_date();
extern "C" AOS_API const char* build_time();
extern "C" AOS_API const char* build_timestamp();

// file systems APIs or a File_System class
extern "C" AOS_API int path_exists(const char* path, int* is_link = 0);
extern "C" AOS_API int mkdir(const char* path, bool recursive = true, mode_t mode = ACE_DEFAULT_DIR_PERMS);
extern "C" AOS_API int rmdir(const char* path, bool recursive = true, int stop_on_error = 0);
/* TODO:
extern "C" AOS_API int cpdir(const char* path, bool recursive = true);
int copy_file(const char* source, const char* dest);

is_file(const char* path) { return path_exists(path) == 1; }
is_dir(const char* path) { return path_exists(path) == 2; }

File_IO class
File_IO.get() // file_get_contents()
File_IO.put() // file_put_contents()
//*/

// datetime functions
// int string_to_tm(const char* cstr, struct tm& tm);
// int parse_date_string();


// OS information
class System_Stat // OS system status/statistics
{
};

} // namepsace aos

#endif // _OS_H_
