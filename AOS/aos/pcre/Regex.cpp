#include "aos/pcre/Regex.h"
#include "aos/Exception.h"

#include <sstream>
#include <cassert>

#define PCRE_STATIC 1
#include "pcre.h"

namespace aos {

const int Regex::OVEC_SIZE = 64;

Regex::Regex(const std::string& pattern, int options, bool study): _pcre(0), _extra(0)
{
	const char* error;
	int offs;
	_pcre = pcre_compile(pattern.c_str(), options, &error, &offs, 0);
	if (!_pcre)
	{
		std::ostringstream msg;
		msg << error << " (at offset " << offs << ")";
		throw aos::Exception(msg.str());
	}
	if (study)
		_extra = pcre_study(_pcre, 0, &error);
}


Regex::~Regex()
{
	if (_pcre)  pcre_free(_pcre);
	if (_extra) pcre_free(_extra);
}


int Regex::match(const std::string& subject, std::string::size_type offset, Match& mtch, int options) const
{
	assert (offset <= subject.length());

	int ovec[OVEC_SIZE];
	int rc = pcre_exec(_pcre, _extra, subject.c_str(), int(subject.size()), int(offset), options & 0xFFFF, ovec, OVEC_SIZE);
	if (rc == PCRE_ERROR_NOMATCH)
	{
		mtch.offset = std::string::npos;
		mtch.length = 0;
		return 0;
	}
	else if (rc == PCRE_ERROR_BADOPTION)
	{
		throw aos::Exception("bad option");
	}
	else if (rc == 0)
	{
		throw aos::Exception("too many captured substrings");
	}
	else if (rc < 0)
	{
		std::ostringstream msg;
		msg << "PCRE error " << rc;
		throw aos::Exception(msg.str());
	}
	mtch.offset = ovec[0] < 0 ? std::string::npos : ovec[0];
	mtch.length = ovec[1] - mtch.offset;
	return rc;
}


int Regex::match(const std::string& subject, std::string::size_type offset, MatchVec& matches, int options) const
{
	assert (offset <= subject.length());

	matches.clear();

	int ovec[OVEC_SIZE];
	int rc = pcre_exec(_pcre, _extra, subject.c_str(), int(subject.size()), int(offset), options & 0xFFFF, ovec, OVEC_SIZE);
	if (rc == PCRE_ERROR_NOMATCH)
	{
		return 0;
	}
	else if (rc == PCRE_ERROR_BADOPTION)
	{
		throw aos::Exception("bad option");
	}
	else if (rc == 0)
	{
		throw aos::Exception("too many captured substrings");
	}
	else if (rc < 0)
	{
		std::ostringstream msg;
		msg << "PCRE error " << rc;
		throw aos::Exception(msg.str());
	}
	matches.reserve(rc);
	for (int i = 0; i < rc; ++i)
	{
		Match m;
		m.offset = ovec[i*2] < 0 ? std::string::npos : ovec[i*2] ;
		m.length = ovec[i*2 + 1] - m.offset;
		matches.push_back(m);
	}
	return rc;
}


bool Regex::match(const std::string& subject, std::string::size_type offset) const
{
	Match mtch;
	match(subject, offset, mtch, RE_ANCHORED | RE_NOTEMPTY);
	return mtch.offset == offset && mtch.length == subject.length() - offset;
}


bool Regex::match(const std::string& subject, std::string::size_type offset, int options) const
{
	Match mtch;
	match(subject, offset, mtch, options);
	return mtch.offset == offset && mtch.length == subject.length() - offset;
}


int Regex::extract(const std::string& subject, std::string& str, int options) const
{
	Match mtch;
	int rc = match(subject, 0, mtch, options);
	if (mtch.offset != std::string::npos)
		str.assign(subject, mtch.offset, mtch.length);
	else
		str.clear();
	return rc;
}


int Regex::extract(const std::string& subject, std::string::size_type offset, std::string& str, int options) const
{
	Match mtch;
	int rc = match(subject, offset, mtch, options);
	if (mtch.offset != std::string::npos)
		str.assign(subject, mtch.offset, mtch.length);
	else
		str.clear();
	return rc;
}


int Regex::split(const std::string& subject, std::string::size_type offset, std::vector<std::string>& strings, int options) const
{
	MatchVec matches;
	strings.clear();
	int rc = match(subject, offset, matches, options);
	strings.reserve(matches.size());
	for (MatchVec::const_iterator it = matches.begin(); it != matches.end(); ++it)
	{
		if (it->offset != std::string::npos)
			strings.push_back(subject.substr(it->offset, it->length));
		else
			strings.push_back(std::string());
	}
	return rc;
}


int Regex::subst(std::string& subject, std::string::size_type offset, const std::string& replacement, int options) const
{
	if (options & RE_GLOBAL)
	{
		int rc = 0;
		std::string::size_type pos = substOne(subject, offset, replacement, options);
		while (pos != std::string::npos)
		{
			++rc;
			pos = substOne(subject, pos, replacement, options);
		}
		return rc;
	}
	else
	{
		return substOne(subject, offset, replacement, options) != std::string::npos ? 1 : 0;
	}
}


std::string::size_type Regex::substOne(std::string& subject, std::string::size_type offset, const std::string& replacement, int options) const
{
	if (offset >= subject.length()) return std::string::npos;

	int ovec[OVEC_SIZE];
	int rc = pcre_exec(_pcre, _extra, subject.c_str(), int(subject.size()), int(offset), options & 0xFFFF, ovec, OVEC_SIZE);
	if (rc == PCRE_ERROR_NOMATCH)
	{
		return std::string::npos;
	}
	else if (rc == PCRE_ERROR_BADOPTION)
	{
		throw aos::Exception("bad option");
	}
	else if (rc == 0)
	{
		throw aos::Exception("too many captured substrings");
	}
	else if (rc < 0)
	{
		std::ostringstream msg;
		msg << "PCRE error " << rc;
		throw aos::Exception(msg.str());
	}
	std::string result;
	std::string::size_type len = subject.length();
	std::string::size_type pos = 0;
	std::string::size_type rp = std::string::npos;
	while (pos < len)
	{
		if (ovec[0] == pos)
		{
			std::string::const_iterator it  = replacement.begin();
			std::string::const_iterator end = replacement.end();
			while (it != end)
			{
				if (*it == '$' && !(options & RE_NO_VARS))
				{
					++it;
					if (it != end)
					{
						char d = *it;
						if (d >= '0' && d <= '9')
						{
							int c = d - '0';
							if (c < rc)
							{
								int o = ovec[c*2];
								int l = ovec[c*2 + 1] - o;
								result.append(subject, o, l);
							}
						}
						else
						{
							result += '$';
							result += d;
						}
						++it;
					}
					else result += '$';
				}
				else result += *it++;
			}
			pos = ovec[1];
			rp = result.length();
		}
		else result += subject[pos++];
	}
	subject = result;
	return rp;
}


bool Regex::match(const std::string& subject, const std::string& pattern, int options)
{
	int ctorOptions = options & (RE_CASELESS | RE_MULTILINE | RE_DOTALL | RE_EXTENDED | RE_ANCHORED | RE_DOLLAR_ENDONLY | RE_EXTRA | RE_UNGREEDY | RE_UTF8 | RE_NO_AUTO_CAPTURE);
	int mtchOptions = options & (RE_ANCHORED | RE_NOTBOL | RE_NOTEOL | RE_NOTEMPTY | RE_NO_AUTO_CAPTURE | RE_NO_UTF8_CHECK);
	Regex re(pattern, ctorOptions, false);
	return re.match(subject, 0, mtchOptions);
}


} // namespace aos
