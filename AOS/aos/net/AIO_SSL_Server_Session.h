#ifndef _AIO_SSL_SERVER_SESSION_H_
#define _AIO_SSL_SERVER_SESSION_H_

#include "AIO_Session.h"
#include "TProactor/SSL/SSL_Asynch_Stream.h"

class AIO_SSL_Server_Session : public AIO_Session
{
//friend class AIO_Session_Manager;
//friend class AIO_Acceptor;
//friend class AIO_Connector;

public:
	AIO_SSL_Server_Session(ACE_SSL_Context* context = 0, int start_ssl = 1);
	virtual ~AIO_SSL_Server_Session();

// inherited from TRB_Service_Handler
public:
	/// This is called after the new connection has been accepted.
	virtual void open(ACE_HANDLE handle, ACE_Message_Block& mb);
	/// This is called when asynchronous <read> operation from the
	/// socket completes.
	virtual void handle_read_stream(const TRB_Asynch_Read_Stream::Result& result);
	/// This is called when an asynchronous <write> to the socket
	/// completes.
	virtual void handle_write_stream(const TRB_Asynch_Write_Stream::Result& result);
	/// This is called after all I/O is done and
	/// it is safe to self-destroy 
	virtual void handle_wakeup();

public:
	virtual int read(ACE_Message_Block& mb);
	virtual int write(ACE_Message_Block& mb);
	virtual void cancel();
	virtual int is_safe_to_delete() const;
	virtual void resume();

public: // start_ssl
	virtual void start_ssl();
	virtual int is_ssl() const { return ssl_; };

protected:
	TRB_SSL_Asynch_Stream ssl_stream_;
	int is_cancelling_;
	int is_safe_to_delete_;
	int ssl_;
};

#endif // _AIO_SSL_SERVER_SESSION_H_
