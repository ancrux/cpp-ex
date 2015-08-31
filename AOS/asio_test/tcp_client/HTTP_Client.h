#ifndef _ASIO_HTTP_CLIENT_H_
#define _ASIO_HTTP_CLIENT_H_

#include "aos/TCP_Client.h"

namespace asio {

class HTTP_Client : public TCP_Client
{
public:
	HTTP_Client();
	virtual ~HTTP_Client();

public:
	virtual TCP_Client_Connection* make_connection();

public:
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_ok; // ok
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_ce; // connect error
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_ct; // connect timeout
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_he; // handshake error
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_re; // read error
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_we; // write error
	ACE_Atomic_Op<ACE_Thread_Mutex, long> n_st; // socket timeout
};

} // namespace asio

#endif // _ASIO_HTTP_CLIENT_H_

