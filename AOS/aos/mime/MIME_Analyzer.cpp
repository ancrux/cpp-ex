#include "aos/mime/MIME_Analyzer.h"

#include "aos/String.h"

#include "unicode/utypes.h" /* Basic ICU data types */
#include "unicode/ucnv.h" /* C Converter API */
#include "unicode/ucsdet.h" /* C Detector API */

//#include <boost/regex.hpp>
//using namespace boost;

//#include <boost/regex/icu.hpp>
//#include <boost/xpressive/xpressive.hpp>

//#include "hash/hash_32.c"
//#include "hash/hash_32a.c"

namespace aos {

void
MIME_Analyzer::dump_body(MIME_Entity& e)
{
	if ( e.child().empty() )
	{
		//// dump body
		//::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", e.get_content_type().c_str(), e.get_encoding().c_str(), e.get_charset().c_str());

		if ( e.get_primary_type() == "text" /*&& e.get_sub_type() == "plain"*/ )
		{

			::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", e.get_content_type().c_str(), e.get_encoding().c_str(), e.get_charset().c_str());
			//if ( e.get_charset().empty() )
			//{
			//	getchar();
			//}

			int res = e.decode_body();
			AOS_UNUSE(res);
			//if ( res == 0 ) ::printf("%s", e.body().c_str());

			// charset: auto-detect
			std::string cs_guess = MIME_Util::guess_charset(e.body().c_str(), e.body().size());
			::printf("cs_guess:%s\n", cs_guess.c_str());

			// charset: name correction
			std::string from_charset(e.get_charset());
			::printf("cs_from:%s\n", from_charset.c_str());
			if ( aos::stricmp(from_charset.c_str(), "gb2312") == 0 )
				from_charset = "gbk";
			else if ( from_charset.find("utf-7") != std::string::npos )
				from_charset = "utf-7";

			UErrorCode status = U_ZERO_ERROR;
			// cs_from: major, cs_guess: minor
			if ( from_charset == "" ) from_charset = cs_guess;
			UConverter* uconv = ucnv_open(from_charset.c_str(), &status);
			//// cs_guess: major, cs_from: minor
			//if ( cs_guess == "" ) cs_guess = from_charset;
			//UConverter* uconv = ucnv_open(cs_guess.c_str(), &status);

			// if error, use default charset
			if ( status != U_ZERO_ERROR )
			{
				::printf("open status: %d\n", status);
				status = U_ZERO_ERROR; // reset status
				uconv = ucnv_open(NULL, &status);
				::printf("open status: %d charset: %s\n", status, ucnv_getDefaultName());
				//getchar();
			}

			// if success, start converting to utf-16
			if ( status == U_ZERO_ERROR )
			{
				size_t n_target = ucnv_getMinCharSize(uconv) * e.body().size() + 1;
				//::printf("buf: %d\n", n_target);
				UChar* target = new (std::nothrow) UChar[n_target];
				::int32_t len;

				//// Convert from_charset to UChar
				//// Unknown character will be convert to utf-8(EF,BF,BD) by default
				//len = ucnv_toUChars(uconv, target, n_target, e.body().c_str(), e.body().size(), &status);
				////target[len] = 0;

				// Convert from_charset to UnicodeString
				UnicodeString ustr(e.body().c_str(), (::int32_t) e.body().size(), uconv, status);

				//if ( U_FAILURE(status) ) {
				if ( status != U_ZERO_ERROR ) {
					::printf("toUChars status: %d\n", status);
					::printf("%s\n", e.body().c_str());
					getchar();
				}
				//::printf("utf-16 len: %d\n", len);
				ucnv_close(uconv);

				// if success, start converting to utf-8
				UErrorCode status = U_ZERO_ERROR;
				uconv = ucnv_open("utf-8", &status);

				if ( status == U_ZERO_ERROR )
				{
					size_t n_target = ucnv_getMaxCharSize(uconv) * e.body().size() + 1;
					e.body().resize(n_target);

					//// Convert from UChar to utf-8
					//len = ucnv_fromUChars(uconv, (char*) e.body().c_str(), e.body().size(), target, len, &status);
					//e.body().resize(len);

					// Convert from UnicodeString to utf-8
					len = ustr.extract((char*) e.body().c_str(), (::int32_t) e.body().size(), uconv, status);
					e.body().resize(len);

					//::printf("utf-8 len: %d\n", len);
					ucnv_close(uconv);
				}

				delete target;

				// if contains illegal characters
				//if ( ::strstr(e.body().c_str(), "\xEF\xBF\xBD") != 0 )
				//{
					//::printf("EFBFBD found!\n");
					//getchar();
					FILE* fp = ::fopen("d:\\_spam_\\_log_\\utf-8.txt", "a");
					::fwrite(e.body().c_str(), e.body().size(), 1, fp);
					::fwrite("\r\n", 2, 1, fp);
					//::fwrite(temp, cnv_size, 1, fp);
					//::fwrite("\r\n", 2, 1, fp);
					::fclose(fp);
				//}
			}

		}
		//::printf("[/BODY]\n");
	}
	else
	{
		// dump child
		//size_t n_boundary = e.get_boundary().size();
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			//if ( n_boundary ) ::printf("--%s\n", get_boundary().c_str());
			MIME_Analyzer::dump_body(*(*it));
		}
		//if ( n_boundary ) ::printf("--%s--\n", get_boundary().c_str());
	}
}

void
MIME_Analyzer::get_url(MIME_Entity& e, std::set< std::string >& url_set)
{
	url_set.clear();

	if ( e.child().empty() )
	{
		//::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", e.get_content_type().c_str(), e.get_encoding().c_str(), e.get_charset().c_str());

		if ( e.get_primary_type() == "text" )
		{
			int res = e.decode_body();
			AOS_UNUSE(res); //@

			//static const regex re(
			//	"("
			//	"https?://([a-z0-9][a-z0-9_-]*)(\\.[a-z0-9][a-z0-9_-]*)+"
			//	"(:\\d+)?"
			//	"(/?[a-z0-9:#@%/;$(){}~_?&=\\+\\\\\\.-]*)"
			//	")"
			//	, regex::icase);

			
			/* TODO: replace boost with pcre
			static const regex re(
				"("
				"https?://[a-z0-9:#@%/;$(){}~_?&=\\+\\\\\\.-]+"
				")"
				, regex::icase);

			smatch match;
		
			std::string& body = e.body();
			std::string::const_iterator start = body.begin();
			std::string::const_iterator end = body.end();

			while ( regex_search(start, end, match, re) )
			{
				std::string url;
				url.assign(match[1].first, match[1].second);

				// .w3.org
				// .yahoo.
				// .opera.com
				// .google.
				// .pctools.com
				// .163.com
				
				if ( url.find(".w3.org") == std::string::npos &&
					url.find(".yahoo.") == std::string::npos &&
					url.find(".google.") == std::string::npos &&
					url.find(".opera.com") == std::string::npos &&
					url.find(".pctools.com") == std::string::npos
					)
					url_set.insert(url);
				
				start = match[0].second;
			}
			//*/
		}

	}
	else
	{
		size_t n_boundary = e.get_boundary().size();
		AOS_UNUSE(n_boundary); //@
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			MIME_Analyzer::get_url(*(*it), url_set);
		}
	}


	/*
	static const regex re("https?://.+\\s", regex::icase);
	smatch match;
	std::string boundary;

	for(MIME_Header_List::iterator it = header_.begin();
		it != header_.end();
		++it)
	{
		if ( aos::strnicmp((*it)->c_str(), "content-type:", 13) == 0
			&& regex_search(*(*it), match, re) )
		{
			boundary.assign(match[1].first, match[1].second);
			
			if ( boundary.size() > 2 )
			{
				if ( (*(boundary.begin()) == '"' && *(boundary.rbegin()) == '"') )
				{
					boundary.erase(boundary.end()-1, boundary.end());
					boundary.erase(boundary.begin(), boundary.begin()+1);
				}
			}
			break;
		}
	}

	return boundary;
	//*/
}

int
MIME_Analyzer::get_phone(MIME_Entity& e)
{
	int count = 0;

	if ( e.child().empty() )
	{
		//::printf("[BODY ctype='%s' encoding='%s' charset='%s']\n", e.get_content_type().c_str(), e.get_encoding().c_str(), e.get_charset().c_str());

		if ( e.get_primary_type() == "text" )
		{
			int res = e.decode_body();
			AOS_UNUSE(res); //@

			/* TODO: replace boost with pcre
			// BIG# 0: utf-16(10,FF) utf-8(EF,BC,90)
			// BIG# 1: utf-16(11,FF) utf-8(EF,BC,91)
			// BIG# 9: utf-16(19,FF) utf-8(EF,BC,99)
			static const regex re(
				"^[^0-9]*"
				"("
				"0(\\s*[^0-9:>]{0,2}\\s*)(\\d(\\s*[^0-9:>]{0,2}\\s*)){7,12}\\d"
				")"
				"\\s*[^0-9:>]{0,2}$"
				, regex::icase);
			smatch match;
		
			std::string& body = e.body();
			std::string::const_iterator start = body.begin();
			std::string::const_iterator end = body.end();

			while ( regex_search(start, end, match, re) )
			{
				std::string url;
				url.assign(match[1].first, match[1].second);
				::printf("%s\n", url.c_str());
				
				start = match[0].second;
				++count;
			}
			//*/
		}

	}
	else
	{
		//size_t n_boundary = e.get_boundary().size();
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			count += MIME_Analyzer::get_phone(*(*it));
		}
	}

	return count;
}

//Fnv32_t
//MIME_Analyzer::hash2(MIME_Entity& e, Fnv32_t hash_val)
//{
//	if ( e.child().empty() )
//	{
//		//hash_val = fnv_32_buf((void*) e.body().c_str(), e.body().size(), hash_val);
//		hash_val = fnv_32a_buf((void*) e.body().c_str(), e.body().size(), hash_val);
//	}
//	else
//	{
//		for(MIME_Entity_List::iterator it = e.child().begin();
//			it != e.child().end();
//			++it)
//		{
//			hash_val = MIME_Analyzer::hash(*(*it), hash_val);
//		}
//	}
//
//	return hash_val;
//}
//
//Fnv32_t
//MIME_Analyzer::get_hash2(MIME_Entity& e)
//{
//	//Fnv32_t hash_val = FNV1_32_INIT;
//	//return MIME_Analyzer::hash(e, hash_val);
//
//	Fnv32_t hash_val = FNV1_32A_INIT;
//	return MIME_Analyzer::hash2(e, hash_val);
//}

ACE_UINT32
MIME_Analyzer::hash(MIME_Entity& e, ACE_UINT32 hash_val)
{
	if ( e.child().empty() )
	{
		//hash_val = fnv_32_buf((void*) e.body().c_str(), e.body().size(), hash_val);
		////hash_val = fnv324_str("more data", hash_val);

		aos::hash::FNV_1a<32> fnv32(hash_val);
		hash_val = fnv32(e.body());
	}
	else
	{
		for(MIME_Entity_List::iterator it = e.child().begin();
			it != e.child().end();
			++it)
		{
			hash_val = MIME_Analyzer::hash(*(*it), hash_val);
		}
	}

	return hash_val;
}

ACE_UINT32
MIME_Analyzer::get_hash(MIME_Entity& e)
{
	//Fnv32_t hash_val = FNV1_32_INIT;
	//return MIME_Analyzer::hash(e, hash_val);

	ACE_UINT32 hash_val = aos::hash::FNV_1a<32>::INIT;
	return MIME_Analyzer::hash(e, hash_val);
}

} // namespace aos

