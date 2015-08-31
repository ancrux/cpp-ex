
#include "aos/Plugin.h"

#include <string>

#define ENGINE_VERSION "1.0.0"

// user-defined plugin interface: to be inherited
// IMPORTANT!!
// on win32: derived class of My_Plugin_Interface can be in the same class name
// BUT on linux: derived class CANNOT be the same class name, otherwise, only
// the first one loaded exists in the system. The subsequent class with the same
// name will be ignored!
class My_Plugin_Interface : public aos::plugin::Plugin_Interface
{
public:
	static const char* Version; // Interface Version: can replace ENGINE_VERSION

public:
	virtual void process(std::string& str) = 0;

//public: // if user wishes to clone multiple Plugin Interface Instances, implement this function
//	virtual My_Plugin_Interface* clone() = 0;

public:
	virtual ~My_Plugin_Interface()
	{
		::printf("DTOR: ~My_Plugin_Interface\n");
	};

protected:
	My_Plugin_Interface() {};
};

//template class aos::plugin::Plugin_Manager_T< My_Plugin_Interface >;
//typedef aos::plugin::Plugin_Manager_T< My_Plugin_Interface > My_Plugin_Manager;

