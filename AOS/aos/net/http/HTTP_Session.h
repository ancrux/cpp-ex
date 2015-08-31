#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_

#include "aos/net/AIO_Session.h"
#include "aos/net/AIO_SSL_Server_Session.h"
#include "aos/net/AIO_SSL_Client_Session.h"

class HTTP_Server_Session : public AIO_Session
{
public:
	HTTP_Server_Session();
	virtual ~HTTP_Server_Session();
	virtual int on_open(ACE_Message_Block& mb_open);
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);
};

class HTTP_SSL_Server_Session : public AIO_SSL_Server_Session
{
public:
	HTTP_SSL_Server_Session();
	virtual ~HTTP_SSL_Server_Session();
	virtual int on_open(ACE_Message_Block& mb_open);
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);
};

class HTTP_Client_Session : public AIO_Session
{
public:
	HTTP_Client_Session();
	virtual ~HTTP_Client_Session();
	virtual int on_open(ACE_Message_Block& mb_open);
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);
};

class HTTP_SSL_Client_Session : public AIO_SSL_Client_Session
{
public:
	HTTP_SSL_Client_Session();
	virtual ~HTTP_SSL_Client_Session();
	virtual int on_open(ACE_Message_Block& mb_open);
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);
};

#endif
