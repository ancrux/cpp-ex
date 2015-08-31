#include "aos/mime/Envelope.h"

#include "aos/Codec.h"

//#include <boost/regex.hpp>
//using namespace boost;

namespace aos {

Envelope::Envelope(const char* evl_file)
:
fhv_(ACE_INVALID_HANDLE)
{
	if ( evl_file ) this->open(evl_file);
}

Envelope::~Envelope()
{
	this->close();
}

int
Envelope::open(const char *evl_file)
{
	// open evl
	if ( open_evl(evl_file) != 0 ) return -1;
	
	// check eml

	return -1;
}

int
Envelope::close()
{
	if ( this->close_evl() == 0 ) return 0;
	return -1;
}

int
Envelope::log(const void* buf, size_t len)
{
	ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, -1);
	return log_evl(buf, len);
}

int
Envelope::open_evl(const char *evl_file)
{
	if ( is_evl_open() && close_evl() != 0 ) return -1;
	fhv_ = ACE_OS::open(evl_file, O_BINARY | O_RDWR);
	if ( ACE_INVALID_HANDLE != fhv_ )
	{
		// clear header
		e_.reset();
		
		// process header
		ssize_t n_byte = MIME_Util::import_file_handle(e_, fhv_, MIME_Entity::Flag::HEADER);
		ACE_OS::lseek(fhv_, n_byte, SEEK_SET);

		// process log
		static const int BUF_SIZE = 4096;
		char buf[BUF_SIZE];

		aos::bcstr bstr;
		std::string line;

		char delimiter = '\n'; // line delimiter
		int c = 0; // line count
		int state = 0; // 0 == HEADER, 1 == LOG

		while(1)
		{
			// read buffer
			ssize_t n_buf = ACE_OS::read(fhv_, buf, BUF_SIZE);
			if ( n_buf <= 0 )
			{
				++c; // count for last line
				ACE_OS::printf("L:%s\n", line.c_str()); //@
				break;
			}

			// get lines
			const char* left = buf;
			size_t n_left = n_buf;
			while(1)
			{
				bstr = aos::get_line(left, n_left, delimiter);
				left += bstr.len;
				n_left -= bstr.len;
				if ( bstr.buf[bstr.len-1] == delimiter )
				{
					// line completed!
					++c; // count one line
					line.append(bstr.buf, bstr.len);
					ACE_OS::printf("L:%s", line.c_str()); //@

					// initialize for next line
					line.resize(0);
				}
				else
				{
					line.append(bstr.buf, bstr.len);
				}
		
				// buffer consumed, get next buffer!
				if ( n_left <= 0 )
					break; 
			}
		}

		// make sure write position is moved to EOF
		ACE_OS::lseek(fhv_, 0, SEEK_END);

		return 0;
	}
	return -1;
}

int
Envelope::close_evl()
{
	if ( ACE_INVALID_HANDLE != fhv_ && ACE_OS::close(fhv_) == 0 ) fhv_ = ACE_INVALID_HANDLE;
	return (ACE_INVALID_HANDLE == fhv_);
}
	
} // namespace aos
