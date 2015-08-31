
int run_mbox_api_test(int argc, ACE_TCHAR* argv[])
{
	using namespace aos;

	int rc = 0;

	Mailbox_SQLite mbox;

	//std::string mbox_id = mbox.create_mailbox();
	//mbox.link_address_to_mailbox("angus@email-home.com", "0000-0000-0000");

	for(int i=0; i< 100; ++i)
		mbox.insert_mail("d:/_send_/in.eml");

	return rc;
}

/*
int run_mailbox_test(int argc, ACE_TCHAR* argv[])
{
	using namespace aos;
	using namespace aos::mailbox;

	int rc;
	
	Mailbox mbox;
	rc = mbox.open("D:/_send_/post/@");
	//rc = mbox.open("D:/_send_/temp");
	//rc = mbox.open("D:/_spam_/2009-04-13");
	//rc = mbox.open("D:/_unread_");
	//rc = mbox.open("D:/_send_/test2");
	rc = mbox.list();

	return 0;
}

int run_mail_entry_test(int argc, ACE_TCHAR* argv[])
{
	using namespace aos;
	using namespace aos::mailbox;
	
	Mail_Entry entry;

	//entry.info.push_back("ID20091212082036");
	//entry.info.push_back("You know me pretty well, this is the subject you don't understand!");
	//entry.info.push_back("\"Angus Liu\" <angus@email-home.com>");
	//entry.info.push_back("\"Noname\" <spam@email-home.com>");
	//entry.info.push_back("\"Noname\" <spam@email-home.com>");
	//entry.info.push_back("\"Noname\" <spam@email-home.com>");
	//entry.info.push_back("\"Noname\" <spam@email-home.com>");

	static const int n = 200;
	static const char* addr = "\"Noname Noname Noname\" <spam@email-home.com>";
	ACE_OS::printf("strlen(addr): %d\n", ACE_OS::strlen(addr));

	for(int i=0; i < n; ++i)
	{
		entry.info.push_back(addr);
	}

	ACE_OS::printf("sizeof: %d\n", sizeof(entry));
	ACE_OS::printf("offsets: %d\n", entry.info.offsets_capacity() * sizeof(size_t));
	ACE_OS::printf("strings: %d\n", entry.info.strings_capacity() * sizeof(char));
	ACE_OS::printf("strings: %d\n", entry.info.strings_size() * sizeof(char));


	//std::string* str_arr = new std::string[n];
	//for(int i=0; i < n; ++i)
	//{
	//	str_arr[i].assign(addr);
	//}
	//ACE_OS::printf("string array: %d\n", (str_arr[0].capacity() * sizeof(char) + sizeof(std::string)) * n);


	return 0;
}
//*/

