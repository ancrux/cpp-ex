

int run_regex_test(int argc, ACE_TCHAR* argv[])
{
	Regex re("abc");

	Regex::MatchVec matches;
	re.match("abc_______abc", 4, matches);

	ACE_OS::printf("size:%d\n", matches.size());

	Regex::MatchVec::iterator iter;
	for(iter = matches.begin(); iter != matches.end(); ++iter)
	{
		ACE_OS::printf("%d:%d\n", (*iter).offset, (*iter).length);
	}

	return 0;
}
