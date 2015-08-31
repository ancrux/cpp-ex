#ifndef _AIO_CONFIG_H_
#define _AIO_CONFIG_H_

#include "ace/OS.h"

// Proactor Type (UNIX only, Win32 ignored)
enum AIO_Type
{
    DEFAULT = 0, 
    AIOCB,
    SIG,
    CB,
    SUN,
    SELECT,
    POLL,
    DEVPOLL,
    EPOLL,
    LINUX_NAIO,
    SUNPORT,
    DUMMY
};

class AIO_Config
{
public:
	static const size_t DEFAULT_THREAD = 4;
	static const size_t MAX_THREAD = 1024;
	static const size_t MAX_DEMULTIPLEXOR = 16;

public:
	AIO_Config();
	~AIO_Config();
	void init();

public:
	size_t n_thread() const { return n_thread_; };
	void n_thread(size_t n_thread)
	{
		if ( n_thread > MAX_THREAD ) n_thread = MAX_THREAD;
		n_thread_ = (n_thread)?n_thread:1;
	};

	size_t n_demultiplexor() const { return n_demultiplexor_; };
	void n_demultiplexor(size_t n_demultiplexor)
	{
		if ( n_demultiplexor > MAX_DEMULTIPLEXOR ) n_demultiplexor = MAX_DEMULTIPLEXOR;
		n_demultiplexor_ = (n_demultiplexor)?n_demultiplexor:1;
	};

	AIO_Type aio_type() const { return aio_type_; };
	void aio_type(AIO_Type aio_type) { aio_type_ = aio_type; };
	
	const ACE_Time_Value& timeout() const { return timeout_; };
	void timeout(const ACE_Time_Value& timeout) { timeout_ = timeout; };

	size_t max_aio_op() const { return max_aio_op_; };
	void max_aio_op(size_t max_aio_op) { max_aio_op_ = max_aio_op; };

	int signal_n() const { return signal_n_; };
	void signal_n(int signal_n) { signal_n_ = signal_n; };

	int leader_type() const { return leader_type_; };
	void leader_type(int leader_type) { leader_type_ = leader_type; };

	int start_aio_type() const { return start_aio_type_; };
	void start_aio_type(int start_aio_type) { start_aio_type_ = start_aio_type; };

protected:
	size_t n_thread_;
	size_t n_demultiplexor_;
	AIO_Type aio_type_;
	ACE_Time_Value timeout_;

protected:
	size_t max_aio_op_; // POSIX : > 0 max number aio operations of proactor
	int signal_n_; // POSIX : signal to interrupt (0 - PIPE Strategy, signal_n - SIGNAL Strategy)
	int leader_type_; // POSIX : leader type :  0-shared / 1-dedicated
	int start_aio_type_; // POSIX : start aio type :  1 - any thread , 0 - only leader
};

#endif
