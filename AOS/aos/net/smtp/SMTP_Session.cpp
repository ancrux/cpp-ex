#include "SMTP_Session.h"

/// SSL Client

SMTP_SSL_Client_Session::SMTP_SSL_Client_Session()
:
AIO_SSL_Client_Session(),
step_(0)
{
}

SMTP_SSL_Client_Session::~SMTP_SSL_Client_Session()
{
}

int
SMTP_SSL_Client_Session::on_open(ACE_Message_Block& mb_open)
{

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	
	if ( read(*mb_read) != 0 ) return -1;

	//start_ssl();

	return 0;
}

int
SMTP_SSL_Client_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	++step_;

	ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_write ) return -1;

	int n = 0;

	if ( step_ == 1 )
	{
		//n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "HELO 127.0.0.1\r\n"); mb_write->wr_ptr(n);
		n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "EHLO 127.0.0.1\r\n"); mb_write->wr_ptr(n);
	}
	else if ( step_ == 2 )
	{
		//ACE_OS::sleep(1);
		n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "STARTTLS\r\n"); mb_write->wr_ptr(n);
		//n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "QUIT\r\n"); mb_write->wr_ptr(n);
	}
	else if ( step_ == 3 )
	{
		start_ssl();
		n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "EHLO 127.0.0.1\r\n"); mb_write->wr_ptr(n);
	}
	else if ( step_ == 4 )
	{
		n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "MAIL FROM: angus@email-home.com\r\n"); mb_write->wr_ptr(n);
	}
	else if ( step_ == 5 )
	{
		ACE_OS::sleep(20);
		n = ACE_OS::snprintf(mb_write->wr_ptr(), mb_write->space(), "QUIT\r\n"); mb_write->wr_ptr(n);
	}
	else
	{
		return -1;
	}

	if ( write(*mb_write) != 0 ) return -1;


	return 0;
}

int
SMTP_SSL_Client_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	//ACE_Time_Value tv(0, 10 * 1000);
	//timespec_t t  = (timespec_t) tv;
	//ACE_OS::nanosleep(&t);

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	
	if ( read(*mb_read) != 0 ) return -1;

	return 0;
}


