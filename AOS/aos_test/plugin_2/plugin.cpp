#include "plugin.h"

const char*
My_Plugin_2::plugin_name()
{
	return "rtrim()";
}

const char*
My_Plugin_2::plugin_version()
{
	return "1.1";
}

void
My_Plugin_2::process(std::string& str)
{
	rtrim(str, " \t\r\n");
}

//My_Plugin_Interface*
//My_Plugin_2::clone()
//{
//	return (My_Plugin_Interface*) new My_Plugin_2();
//}

//void
//register_plugin(My_Plugin_Manager& pm)
//{
//	My_Plugin_2* plugin = new My_Plugin_2();
//	My_Plugin_Interface* plugin_if = (My_Plugin_Interface*) plugin;
//	pm.insert(plugin_if);
//
//	::printf("register->name(): %s\n", plugin->plugin_name());
//	::printf("register->if_name(): %s\n", plugin_if->plugin_name());
//	return;
//}

//const char*
//engine_version()
//{
//	return ENGINE_VERSION;
//}

My_Plugin_Interface*
create_instance()
{
	My_Plugin_2* plugin = new My_Plugin_2();
	return (My_Plugin_Interface*) plugin;
}

std::string&
ltrim(std::string& str, const char* trim)
{
	return str.erase(0, str.find_first_not_of(trim));
}

std::string&
rtrim(std::string& str, const char* trim)
{
	return str.erase(str.find_last_not_of(trim)+1, std::string::npos);
}


