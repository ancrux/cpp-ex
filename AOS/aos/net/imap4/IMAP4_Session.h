#ifndef _IMAP4_SESSION_H_
#define _IMAP4_SESSION_H_

#include "aos/net/AIO_Session.h"
#include "aos/net/AIO_SSL_Server_Session.h"
#include "aos/net/AIO_SSL_Client_Session.h"
#include "aos/net/imap4/IMAP4_Client_IO.h"

class IMAP4_SSL_Client_Session : public AIO_SSL_Client_Session
{
public:
	IMAP4_SSL_Client_Session();
	virtual ~IMAP4_SSL_Client_Session();

	virtual int on_open(ACE_Message_Block& mb_open);
	virtual int on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result);
	virtual int on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result);

protected:
	IMAP4_Client_IO io_;
	int step_;
};

#endif
