#ifndef _SMTP_SERVER_H_
#define _SMTP_SERVER_H_

#include "ace/OS.h"
#include "ace/Atomic_Op.h"
#include "ace/Task_Ex_T.h"

// for ACE_SOCK_Stream and ACE_SSL_SOCK_Stream
#include "ace/INET_Addr.h"
#include "ace/SSL/SSL_SOCK_Stream.h" //#include "ace/SOCK_Stream.h"
#include "ace/SSL/SSL_SOCK_Acceptor.h" //#include "ace/SOCK_Acceptor.h"
#include "ace/SSL/SSL_SOCK_Connector.h" //#include "ace/SOCK_Connector.h"

#include <iostream>
using namespace std;

// namespace aos {

#define USE_SSL 0

class SMTP_Server : public ACE_Task_Base // SMTP_Server
{
public:
	static const size_t MAX_CONN = 10000;

public:
	SMTP_Server();
	virtual ~SMTP_Server();

public:
	int open(const ACE_Addr& addr);
	int close();
	int start();
	int stop(const ACE_Time_Value* timeout = 0);

public:
	void disable() { shutdown_ = 1; };
	void enable() { shutdown_ = 0; };
	void max_conn(int max_conn) { max_conn_ = max_conn + 1; };
	int max_conn() const { return ((int) max_conn_.value()) - 1; };
	int n_conn() const { return ((int) this->thr_count()) - 1; };

public:
	virtual int svc();

protected:
#if USE_SSL == 1
	ACE_SSL_SOCK_Acceptor acceptor_;
#else
	ACE_SOCK_Acceptor acceptor_;
#endif
	ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> max_conn_;
	ACE_Atomic_Op<ACE_Thread_Mutex, long> shutdown_;
};

// } // namespace aos

#endif // _SMTP_SERVER_H_