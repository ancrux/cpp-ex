#ifndef _SOCK_STREAM_H_
#define _SOCK_STREAM_H_

#include "ace/SSL/SSL_SOCK_Stream.h"

namespace aos {

class SOCK_Stream
{
public:
	virtual ~SOCK_Stream() {};

public:
	virtual int control(int cmd, void* arg) const = 0;
	virtual ACE_HANDLE get_handle(void) const = 0;
	virtual void set_handle(ACE_HANDLE handle) = 0;
	virtual int set_option(int level, int option, void* optval, int optlen) const = 0;
	virtual int get_option(int level, int option, void* optval, int* optlen) const = 0;
	virtual int get_local_addr(ACE_Addr& addr) const = 0;
	virtual int get_remote_addr(ACE_Addr& addr) const = 0;
	virtual ssize_t recv(void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const = 0;
	virtual ssize_t recv_n(void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const = 0;
	virtual ssize_t send(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const = 0;
	virtual ssize_t send_n(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const = 0;
	virtual int close() = 0;
};

class Socket_Stream;
class SSL_Socket_Stream;

class Socket_Stream : public SOCK_Stream
{
public:
	virtual ~Socket_Stream() {};

public:
	virtual int control(int cmd, void* arg) const
	{
		return stream_.control(cmd, arg);
	};
	virtual ACE_HANDLE get_handle(void) const
	{
		return stream_.get_handle();
	};
	virtual void set_handle(ACE_HANDLE handle)
	{
		return stream_.set_handle(handle);
	};
	virtual int set_option(int level, int option, void* optval, int optlen) const
	{
		return stream_.set_option(level, option, optval, optlen);
	};
	virtual int get_option(int level, int option, void* optval, int* optlen) const
	{
		return stream_.get_option(level, option, optval, optlen);
	};
	virtual int get_local_addr(ACE_Addr& addr) const
	{
		return stream_.get_local_addr(addr);
	};
	virtual int get_remote_addr(ACE_Addr& addr) const
	{
		return stream_.get_remote_addr(addr);
	};
	virtual ssize_t recv(void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const
	{
		return stream_.recv(buf, len, flags, timeout);
	}
	virtual ssize_t recv_n(void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const
	{
		return stream_.recv_n(buf, len, flags, timeout, bytes_transferred);
	}
	virtual ssize_t send(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const
	{
		return stream_.send(buf, len, flags, timeout);
	}
	virtual ssize_t send_n(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const
	{
		return stream_.send_n(buf, len, flags, timeout, bytes_transferred);
	}
	virtual int close()
	{
		return stream_.close();
	}

public:
	ACE_SOCK_Stream& stream() { return stream_; }; // explicit conversion
	operator ACE_SOCK_Stream& () { return stream_; }; // implicit conversion

public:
	int start_ssl(SSL_Socket_Stream& stream, int server = 0);

public:
	ACE_SOCK_Stream stream_;
};

class SSL_Socket_Stream : public SOCK_Stream
{
public:
	virtual ~SSL_Socket_Stream() {};

public:
	virtual int control(int cmd, void* arg) const
	{
		return stream_.control(cmd, arg);
	};
	virtual ACE_HANDLE get_handle(void) const
	{
		return stream_.get_handle();
	};
	virtual void set_handle(ACE_HANDLE handle)
	{
		return stream_.set_handle(handle);
	};
	virtual int set_option(int level, int option, void* optval, int optlen) const
	{
		return stream_.set_option(level, option, optval, optlen);
	};
	virtual int get_option(int level, int option, void* optval, int* optlen) const
	{
		return stream_.get_option(level, option, optval, optlen);
	};
	virtual int get_local_addr(ACE_Addr& addr) const
	{
		return stream_.get_local_addr(addr);
	};
	virtual int get_remote_addr(ACE_Addr& addr) const
	{
		return stream_.get_remote_addr(addr);
	};
	virtual ssize_t recv(void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const
	{
		return stream_.recv(buf, len, flags, timeout);
	}
	virtual ssize_t recv_n(void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const
	{
		return stream_.recv_n(buf, len, flags, timeout, bytes_transferred);
	}
	virtual ssize_t send(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout) const
	{
		return stream_.send(buf, len, flags, timeout);
	}
	virtual ssize_t send_n(const void* buf, size_t len, int flags, const ACE_Time_Value* timeout, size_t* bytes_transferred = 0) const
	{
		return stream_.send_n(buf, len, flags, timeout, bytes_transferred);
	}
	virtual int close()
	{
		return stream_.close();
	}

public:
	ACE_SSL_SOCK_Stream& stream() { return stream_; }; // explicit conversion
	operator ACE_SSL_SOCK_Stream& () { return stream_; }; // implicit conversion

public:
	SSL* ssl() { return stream_.ssl(); };
	int stop_ssl(Socket_Stream& stream);

public:
	ACE_SSL_SOCK_Stream stream_;
};

///*
//What you can try doing (I know it works, I've tried it) is connect 
//using a simple ACE_SOCK_Stream and then use set_handle on an 
//ACE_SOCK_SSL_Stream to use the same TCP connection (and of course make 
//sure to invalidate the orignal stream class).
//
//You will still need to handle yourself making the new class do the SSL 
//handshake, if I remember correctly (not hard, you can use SSL_connect 
//or SSL_accept using the SSL object).
//
//int SSL_accept(SSL *ssl); // ssl.h
//int SSL_connect(SSL *ssl); // ssl.h
//SSL* ACE_SSL_SOCK_Stream::ssl() const;

//?? change these two function to be static?
inline int
Socket_Stream::start_ssl(SSL_Socket_Stream& stream, int server)
{
	stream.set_handle(this->get_handle());
	this->set_handle(ACE_INVALID_HANDLE);
	
	// SSL handshakes
	int rc = 0;
	SSL* ssl = stream.ssl();
	if ( server )
	{
		// refer to ACE_SSL_SOCK_Acceptor::ssl_accept()
		if ( !SSL_in_accept_init(ssl) )
			::SSL_set_accept_state(ssl);

		rc = ::SSL_accept(ssl);
		rc = (::SSL_get_error(ssl, rc) == SSL_ERROR_NONE)?0:-1;
	}
	else
	{
		rc = ::SSL_connect(stream.ssl());
		rc = (::SSL_get_error(ssl, rc) == SSL_ERROR_NONE)?0:-1;
	}

	return rc;
};

inline int
SSL_Socket_Stream::stop_ssl(Socket_Stream& stream)
{
	stream.set_handle(this->get_handle());
	this->set_handle(ACE_INVALID_HANDLE);

	return 0;
};
//*/

} // namespace aos

#endif // _SOCK_STREAM_H_
