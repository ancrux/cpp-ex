#ifndef _AIO_SESSION_H_
#define _AIO_SESSION_H_

#include "ace/OS.h"
#include "ace/Synch.h"

#include "TProactor/Asynch_Acceptor.h"
#include "TProactor/Asynch_Connector.h"

// for Asynch_RW_Stream  stream_;
#include "Asynch_RW.h"
#include "TProactor/SSL/SSL_Asynch_Stream.h" // SSL stream
#include "Proactor_TaskPool.h"

class AIO_SSL;
class AIO_Session;
class AIO_Session_Manager;

class AIO_Session : public TRB_Service_Handler
{
friend class AIO_Session_Manager;
friend class AIO_Acceptor;
friend class AIO_Connector;

public:
	static const size_t BUFSIZE = 4096;

public:
	AIO_Session();
	virtual ~AIO_Session();

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
	// This is called to pass the new connection's addresses.
	virtual void addresses(const ACE_INET_Addr& peer, const ACE_INET_Addr& local);

public:
	virtual int read(ACE_Message_Block& mb);
	virtual int write(ACE_Message_Block& mb);
	// return > 0 if you wish to release mb yourself,
	// otherwise ( == 0 || < 0 ), mb will be free right after function call.
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);
	virtual int on_open(ACE_Message_Block& mb_open);
	virtual void on_close();
	virtual void on_timeout();
	virtual void on_resume();
	virtual void on_pause();
	//? virtual int on_read_error()
	//? virtual int on_write_error()
	// cancel a session and its read/write stream and will self-destroy
	virtual void cancel();
	virtual int is_safe_to_delete() const;

public:
	virtual void on_open_error(int err_no);

public: // start_ssl
	virtual void start_ssl() {}; // do nothing
	virtual int is_ssl() const { return ((ssl_)?1:0); };
	virtual void stop_ssl() {};

public:
	// check if timeout
	void check_timeout();
	//? do we need to distinguish read_complete_pause()/write_complete_pause()
	//? or use pause(bool is_read_complete_pause = true, bool is_write_complete_pause = true);
	void pause()
	{
		n_op_r_ += 8; n_op_w_ += 8;
		on_pause();    
	};
	int is_paused()
	{
		return ( n_op_r_ >=8 || n_op_w_ >= 8 );
	};
	virtual void resume();

public:
	inline AIO_Session_Manager* manager() const
	{
		return manager_;
	};
	inline int has_pending_io() const
	{ 
		return ( n_op_r_ > 0 || n_op_w_ > 0 );
	};
	inline void update_last_time()
	{
		last_op_ = ACE_OS::gettimeofday();
	};
	inline void set_timeout (const ACE_Time_Value& timeout)
	{
		timeout_ = timeout;
	};
	inline const ACE_Time_Value& get_timeout (void) const
	{
		return timeout_;
	};
	inline int index() const
	{
		return index_;
	};

protected:
	inline ACE_SYNCH_MUTEX& mutex()
	{
		return lock_;
	};
	inline void manager(AIO_Session_Manager* manager)
	{
		manager_ = manager;
	};
	inline void index(int i)
	{
		this->index_ = i;
	};

protected:
	int n_op_r_; // # of pending read operations
	int n_op_w_; // # of pending write operations

	ACE_HANDLE handle_; // protected member in class TRB_Asynch_RW_T ACE_HANDLE handle_;
	Asynch_RW_Stream  stream_; // read/write stream
	ACE_INET_Addr peer_; // peer IP address
	AIO_SSL* ssl_;

	ACE_Time_Value last_op_; // time of last operation
	ACE_Time_Value timeout_; // timeout

	AIO_Session_Manager* manager_; // parent acceptor that creates this session
	int index_; // index in acceptor's connection array

	ACE_SYNCH_MUTEX lock_; // session lock
};

// to be used in AIO_Session for SSL connection
// not completed yet!
class AIO_SSL
{
public:
	TRB_SSL_Asynch_Stream ssl_stream;
	int is_cancelling;
	int is_safe_to_delete;
	int ssl_type; // (ST_SERVER/ST_CLIENT) protected member in class TRB_SSL_Asynch_Stream Stream_Type type_
};

// Interface class for Session Manager
class AIO_Session_Manager
{
friend class AIO_Session;

public:
	virtual Proactor_TaskPool& task() const = 0;
	virtual int destroy_session(AIO_Session* session) = 0;
};

#endif // _AIO_SESSION_H_
