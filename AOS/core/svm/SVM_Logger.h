#ifndef _SVM_LOGGER_H_
#define _SVM_LOGGER_H_

#include "ace/OS.h"
#include "ace/Singleton.h"
#include "ace/Null_Mutex.h"

#include "aos/Logger.h"

class SVM_Logger : public aos::Logger
{
public:
	SVM_Logger() : aos::Logger(".", "svm_", "log", aos::Logger::Rotate::DAY, 30, 10 * 1000 * 1000) {};
	virtual ~SVM_Logger() {};
};

typedef ACE_Singleton< SVM_Logger, ACE_Null_Mutex > The_SVM_Logger;
#define SVM_LOG (The_SVM_Logger::instance())
//static inline SVM_Logger* SVM_LOGGER() { return The_SVM_Logger::instance(); };

#endif // _SVM_LOGGER_H_


