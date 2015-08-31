#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "aos/Config.h"

#include "ace/DLL.h"

#include <set>
#include <vector>

namespace aos {
namespace plugin {

// base plugin interface, to be inherited
class Plugin_Interface
{
public:
	virtual const char* plugin_name() = 0;
	virtual const char* plugin_version() = 0;

public:
	virtual ~Plugin_Interface() {};

protected:
	Plugin_Interface() {};
};

template< class Interface >
class Plugin_Manager_T
{
public:
	Plugin_Manager_T();
	~Plugin_Manager_T();

public:
	void insert(Interface* plugin);
	Interface* at(size_t idx)
	{
		return plugins_.at(idx);
	};
	size_t size() const
	{
		return plugins_.size();
	}

protected:
	std::vector< Interface* > plugins_;
};

template< class Interface >
Plugin_Manager_T< Interface >::Plugin_Manager_T()
{
}

template< class Interface >
Plugin_Manager_T< Interface >::~Plugin_Manager_T()
{
	::printf("Plugin_Manager_T dtor\n");
	for(size_t i = 0; i < plugins_.size(); ++i)
	{
		//Interface* iface = plugins_.at(i);
		//delete iface;

		delete plugins_.at(i);
	}

	plugins_.clear();
}

template< class Interface >
void
Plugin_Manager_T< Interface >::insert(Interface* plugin)
{
	plugins_.push_back(plugin);
}

// let the plugin to register itself to plugin manager by calling
// void register_plugin(PluginManager& pm); // self-register function
// other required functions:
// const char* engine_version(); // engine version when plugin is built
// const char* plugin_version(); // plugin version

//// user-defined plugin interface: to be inherited
//// IMPORTANT!!
//// on win32: derived class of My_Plugin_Interface can be in the same class name
//// BUT on linux: derived class CANNOT be the same class name, otherwise, only
//// the first one loaded exists in the system. The subsequent class with the same
//// name will be ignored!
//class My_Plugin_Interface : public aos::plugin::Plugin_Interface
//{
//public:
//	static const char* Version; // Interface Version: can replace ENGINE_VERSION
//
//public:
//	virtual void process(std::string& str) = 0;
//
//public: // if user wishes to clone multiple Plugin Interface Instances, implement this function
//	virtual My_Plugin_Interface* clone() = 0;
//
//public:
//	virtual ~My_Plugin_Interface() {};
//
//protected:
//	My_Plugin_Interface() {};
//};

template< class Interface >
class Plugin_T 
{
public:
	Plugin_T();
	~Plugin_T();

public:
	int	open(const ACE_TCHAR* dll_name, int open_mode = ACE_DEFAULT_SHLIB_MODE, int close_handle_on_destruction = 1);
	int close(); 

public:
	typedef Interface* fnCreate(void);
	Interface* create_instance(); //? create_instance();

//public: // need destroy_plugin if destructor cannot call properly and is likely to cause memory-leak
//	typedef void fnDestroy(Interface*);
//	void destroy_plugin(Interface* plugin);

protected:
	ACE_DLL dll_;
	fnCreate* fn_create_;
	//fnDestroy* fn_destroy_;

protected:
	//std::set< Interface* > plugins_;
};

template< class Interface >
Plugin_T< Interface >::Plugin_T()
:
fn_create_(0)
{
}

template< class Interface >
Plugin_T< Interface >::~Plugin_T()
{
	this->close();
}

template< class Interface >
int
Plugin_T< Interface >::open(const ACE_TCHAR *dll_name, int open_mode, int close_handle_on_destruction)
{
	int rc = dll_.open(dll_name, open_mode, close_handle_on_destruction);
	if ( rc != -1 )
	{
		fn_create_ = (fnCreate*) dll_.symbol("create_instance");
	}

	if ( !fn_create_ )
		rc = -1;

	return rc ;
}

template< class Interface >
int
Plugin_T< Interface >::close()
{
	//for(std::set< Interface* >::iterator iter = plugins_.begin();
	//	iter != plugins_.end();
	//	++iter)
	//{
	//	delete *iter;
	//}
	//plugins_.clear();
	
	fn_create_ = 0;
	return dll_.close();
}

template< class Interface >
Interface*
Plugin_T< Interface >::create_instance()
{
	return (fn_create_)?fn_create_():0;
}

} // namespace plugin
} // namespace aos

#endif // _PLUGIN_H_

