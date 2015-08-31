#ifndef _AIO_SSL_CLIENT_SESSION_H_
#define _AIO_SSL_CLIENT_SESSION_H_

#include "AIO_Session.h"
#include "TProactor/SSL/SSL_Asynch_Stream.h"

class AIO_SSL_Client_Session : public AIO_Session
{
//friend class AIO_Session_Manager;
//friend class AIO_Acceptor;
//friend class AIO_Connector;

public:
	AIO_SSL_Client_Session(ACE_SSL_Context* context = 0, int start_ssl = 1);
	virtual ~AIO_SSL_Client_Session();

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

/*
---------- Forwarded message ----------
From: Alexander Libman <libman@terabit.com.au>
Date: 2008/7/1
Subject: RE: Hi, just a few question about TProactor
To: Angus Liu <angus@email-home.com>
Cc: Support Terabit <support@terabit.com.au>


Hi Angus,
I think there is a quite simple solution for  StartTLS.
I will try to explain the idea  with pseudo-code.


Here is the classic way of doing AIO with ACE (details are omitted)

class MyHandler : public ACE_Service_Handler
{
     ACE_Asynch_Read_Stream  rs_;
     ACE_Asynch_Write_Stream ws_;
       ACE_Handle              h_;
     ACE_Atomic<int,MUTEX>   pending_count_;

     int start_read ()
       {
               ++pending_count_;
               return rs_.read (...);
       }

     int start_write ()
       {
               ++pending_count_;
               return ws_.write (...);
       }

       void asynch_close ()
       {
               rs_.cancel ();
               ws_.cancel ();
               closesocket (h_);
             h_ = ACE_INVALID_HANDLE;
       }

       virtual
     void  open (ACE_HANDLE handle, ACE_Message_Block &)
     {
             h_ = handle;
           rs_.open (*this, h_, completion_key, proactor);
           w_.open (*this, h_, completion_key, proactor);

           start_read  (...) ;  // start asynch read
           start_write (...) ;  // start asynch write
     }

     virtual
     void handle_read_stream (const ACEAsynch_Read_Stream::Result &result)
     {
               // process result

       if  ( all is OK and  we want read more )
               start_read ();

               if  (all is OK and  we want write more )
               start_write ();

               if ( something wrong or we want to shutdown)
                       asynch_close ();

               if (--pending_count_ == 0)
                       delete this;
       }

       virtual
     void handle_write_stream (const ACE_Asynch_Write_Stream::Result &)
     {
               // same as handle_read_stream
       }
};


Now let convert this pattern to the mixed one with SSL.

class MyHandler : public ACE_Service_Handler
{
     ACE_Asynch_Read_Stream  rs_;
     ACE_Asynch_Write_Stream ws_;
       ACE_Handle              h_;
     ACE_SSL_Asynch_Stream   ssl_rws_;  // SSL read & write
     bool                    mode_;     // true if SSL mode
     ACE_Atomic<int,MUTEX>   pending_count_;

     int start_read ()
       {
               ++pending_count_;
               return (mode_)
                       ? ssl_rws_.read (...)
                 : rs_.read (...)
       }

     int start_write ()
       {
               ++pending_count_;
               return (mode_)
                       ? ssl_rws_.write (...)
                 : ws_.write (...)
       }

       void asynch_close ()
       {
               if (mode_)
               {
                       ssl_rws_.cancel ();
                       ssl_rws_.close ();
               }
               else
               {
                       rs_.cancel ();
                       ws_.cancel ();
                       closesocket (h_);
               h_ = ACE_INVALID_HANDLE;
               }
       }

       void start_ssl ()
       {
               if (mode_)
                       return ;  //already in SSL

               mode_ = true;
               ssl_rws_.open (*this,
                          h_,
                          completion_key,
                                  proactor,
                                  1);  // pass handle ownership !!!

               this->h_ = ACE_INVALID_HANDLE;
       }

       virtual
     void  open (ACE_HANDLE handle, ACE_Message_Block &)
     {
             h_ = handle;
           rs_.open (*this, h_, completion_key, proactor);
           w_.open (*this, h_, completion_key, proactor);

           start_read  (...) ;  // start asynch read
           start_write (...) ;  // start asynch write
     }

     virtual
     void handle_read_stream (const ACE_Asynch_Read_Stream::Result &result)
     {
               // process result

               if ( all is OK and we want to start SSL/TLS)
                       start_ssl ();

       if  ( all is OK and  we want read more )
               start_read ();

               if  (all is OK and  we want write more )
               start_write ();

               if ( something wrong or we want to shutdown)
                       asynch_close ();

               if (--pending_count_ == 0)
                       delete this;
       }

       virtual
     void handle_write_stream (const ACE_Asynch_Write_Stream::Result &)
     {
               // same as handle_read_stream
       }
};

Such solution should work for SMTP client with startTLS feature.
In most cases it should work for servers as well.
But there are some cases when it does not work for servers. Example:
your FTP server that should recognize transparent or SSL mode without any
negotiations with client.

Problem: server has already read the initial SSL packet
that must be handled by SSL (SSL_Asynch_Read_Stream).

Solution: We have to inject this packet to SSL.
This can be done by extention of SSL_Asynch_Stream, so
that initial data given in ACE_Message_Block would be passed to SSL BIO.

Yes, it sounds reasonable to include such feature to TProactor framework.
Kind of ACE_SSL_Asynch_Stream_Ex or MixedSslChannel in IOTerabit ...

Thank you much,

Alex
//*/

#endif // _AIO_SSL_CLIENT_SESSION_H_
