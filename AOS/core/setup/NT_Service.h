#ifndef _NT_SERVICE_H_
#define _NT_SERVICE_H_

#include "ace/NT_Service.h"
#include "ace/Singleton.h"
#include "ace/Mutex.h"

class NT_Service : public ACE_NT_Service
{
public:
	NT_Service();
	~NT_Service();
 
public:
	virtual void handle_control (DWORD control_code);
	// We override <handle_control> because it handles stop requests
	// privately.
	virtual int svc (void);
	// This is a virtual method inherited from ACE_NT_Service.

public:
	int console() const { return console_; };
	void console(int console) { console_ = console; };

protected:
	int stop_;
	int console_;
};

// Define a singleton class as a way to insure that there's only one
// Service instance in the program, and to protect against access from
// multiple threads.  The first reference to it at runtime creates it,
// and the ACE_Object_Manager deletes it at run-down.

typedef ACE_Singleton<NT_Service, ACE_Mutex> NT_SERVICE;

// Define a function to handle Ctrl+C to cleanly shut this down in console mode.

static BOOL __stdcall
ConsoleHandler(DWORD ctrlType)
{
  ACE_UNUSED_ARG(ctrlType);
  NT_SERVICE::instance()->handle_control(SERVICE_CONTROL_STOP);
  return TRUE;
}

static const ACE_TCHAR* NTSVC_NAME = ACE_TEXT("_Service_");
static const ACE_TCHAR* NTSVC_DESC = ACE_TEXT("");

#endif // _NT_SERVICE_H_
