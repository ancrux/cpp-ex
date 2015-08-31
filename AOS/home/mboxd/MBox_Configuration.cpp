#include "MBox_Configuration.h"

#include "ace/Configuration_Import_Export.h"

#include <iostream>
using namespace std;

int
MBox_Configuration::load(const char* path)
{
	// load ini file
	ACE_Configuration_Heap config;
	config.open();

	ACE_Ini_ImpExp ini_io(config);
	ini_io.import_config(path);

	///*
	ACE_Configuration_Section_Key sec;
	ACE_TString val;

	val.fast_clear();
	config.expand_path(config.root_section(), ACE_TEXT("Path"), sec);
	config.get_string_value(sec, ACE_TEXT("Prefix"), val);
	if ( !val.is_empty() ) {
		val += '/';
		this->path_prefix = val.c_str();
	}
	//::printf("[Path] Prefix=%s\n", path_prefix.c_str()); //@

	val.fast_clear();
	config.expand_path(config.root_section(), ACE_TEXT("Path"), sec);
	config.get_string_value(sec, ACE_TEXT("Mailbox"), val);
	if ( !val.is_empty() )
		this->path_mbox_base = this->path_prefix + val.c_str();
	//::printf("[Path] Mailbox=%s\n", path_mbox_base.c_str()); //@

	val.fast_clear();
	config.expand_path(config.root_section(), ACE_TEXT("Path"), sec);
	config.get_string_value(sec, ACE_TEXT("Spool"), val);
	if ( !val.is_empty() ) {
		this->path_spools.clear();
		this->path_spools.push_back(this->path_prefix + val.c_str());
	}
	//::printf("[Path] Spool=%s\n", path_spools[0].c_str()); //@
	//*/

	/*
	// ---------------------------
	// iterate through entire file (not including sub-section iteration)
	// ---------------------------
	ACE_Configuration_Section_Key sec;
	sec = config.root_section();
	ACE_TString sec_name;
	for(int sec_index = 0;
		config.enumerate_sections(sec, sec_index, sec_name) == 0;
		++sec_index)
	{
		cout << "sec_index:" << sec_index << endl;
		cout << "sec_name:" << sec_name.c_str() << endl;
		cout << "------" << endl;

		ACE_Configuration_Section_Key sub_sec;
		config.open_section(sec, sec_name.c_str(), 0, sub_sec); 
		ACE_TString val_name;
		ACE_Configuration::VALUETYPE val_type;
		for(int val_index = 0;
			config.enumerate_values(sub_sec, val_index, val_name, val_type) == 0;
			++val_index)
		{
			cout << "[" << val_index << "]";
			cout << val_name;
			cout << "(Type:" << val_type << ")";
			ACE_TString value;
			config.get_string_value(sub_sec, val_name.c_str(), value);
			cout << "=" << value.c_str() << endl;
		}
	}
	//*/

	return 0;
}
