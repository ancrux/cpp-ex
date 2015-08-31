#include "HTTP_Session.h"
#include "HTTP.h"

// Server

HTTP_Server_Session::HTTP_Server_Session()
:
AIO_Session()
{
}

HTTP_Server_Session::~HTTP_Server_Session()
{
}

int
HTTP_Server_Session::on_open(ACE_Message_Block& mb_open)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session open(this: %@)\n"), this));//@

	ACE_Message_Block* mb = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb ) return -1;

	if ( read(*mb) != 0 ) return -1;

	return 0;
}

int
HTTP_Server_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	//ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	HTTP::Responser http;

	if ( mb.space() == 0 || (mb.length() > 4 && ACE_OS::strncmp(mb.wr_ptr()-4, "\r\n\r\n", 4) == 0) )
	{
		//ACE_Time_Value tv(0, 1 * 1000);
		//timespec_t t  = (timespec_t) tv;
		//ACE_OS::nanosleep(&t);

		int n_err = 0;
		if ( mb.space() == 0 ) n_err = HTTP::Response::Request_Entity_Too_Large;
		
		ACE_Message_Block* out = new (std::nothrow) ACE_Message_Block(BUFSIZE);
		int n_res = http.parse_header(&mb, out, n_err);

		write(*out);
	}
	else
	{
		read(mb);
		return 1;
	}

	return 0;
}

int
HTTP_Server_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@
	return 0;
}

// SSL Server

HTTP_SSL_Server_Session::HTTP_SSL_Server_Session()
:
AIO_SSL_Server_Session()
{
}

HTTP_SSL_Server_Session::~HTTP_SSL_Server_Session()
{
}

int
HTTP_SSL_Server_Session::on_open(ACE_Message_Block& mb_open)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session open(this: %@)\n"), this));//@

	ACE_Message_Block* mb = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb ) return -1;

	if ( read(*mb) != 0 ) return -1;

	return 0;
}

int
HTTP_SSL_Server_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	//ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	HTTP::Responser http;

	if ( mb.space() == 0 || (mb.length() > 4 && ACE_OS::strncmp(mb.wr_ptr()-4, "\r\n\r\n", 4) == 0) )
	{
		//ACE_Time_Value tv(0, 1 * 1000);
		//timespec_t t  = (timespec_t) tv;
		//ACE_OS::nanosleep(&t);

		int n_err = 0;
		if ( mb.space() == 0 ) n_err = HTTP::Response::Request_Entity_Too_Large;
		
		ACE_Message_Block* out = new (std::nothrow) ACE_Message_Block(BUFSIZE);
		int n_res = http.parse_header(&mb, out, n_err);

		write(*out);
	}
	else
	{
		read(mb);
		return 1;
	}

	return 0;
}

int
HTTP_SSL_Server_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@
	return 0;
}

/// Client

HTTP_Client_Session::HTTP_Client_Session()
:
AIO_Session()
{
}

HTTP_Client_Session::~HTTP_Client_Session()
{
}

int
HTTP_Client_Session::on_open(ACE_Message_Block& mb_open)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session open(this: %@)\n"), this));//@

	ACE_Message_Block* mb = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb ) return -1;

	int n = 0;
	n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::_Request_, "GET", "/no_content.htm", 1.0); mb->wr_ptr(n);
	//n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::Connection, "keep-alive"); mb->wr_ptr(n);
	//n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::Host, "localhost"); mb->wr_ptr(n);
	n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::_End_); mb->wr_ptr(n);

	//ACE_OS::write_n(ACE_STDOUT, mb->rd_ptr(), mb->length()); //@

	if ( write(*mb) != 0 ) return -1;

	return 0;
}

int
HTTP_Client_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	//ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	//static int i = 0;
	//::printf("%d, ", i++);

	return 0;
}

int
HTTP_Client_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	//ACE_Time_Value tv(0, 10 * 1000);
	//timespec_t t  = (timespec_t) tv;
	//ACE_OS::nanosleep(&t);

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	
	if ( read(*mb_read) != 0 ) return -1;

	return 0;
}

/// SSL Client

HTTP_SSL_Client_Session::HTTP_SSL_Client_Session()
:
AIO_SSL_Client_Session()
{
}

HTTP_SSL_Client_Session::~HTTP_SSL_Client_Session()
{
}

int
HTTP_SSL_Client_Session::on_open(ACE_Message_Block& mb_open)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session open(this: %@)\n"), this));//@

	ACE_Message_Block* mb = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb ) return -1;

	int n = 0;
	n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::_Request_, "GET", "/admin/xjs_loader.htm", 1.0); mb->wr_ptr(n);
	//n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::Connection, "keep-alive"); mb->wr_ptr(n);
	//n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::Host, "localhost"); mb->wr_ptr(n);
	n = ACE_OS::snprintf(mb->wr_ptr(), mb->space(), HTTP::Header::_End_); mb->wr_ptr(n);

	//ACE_OS::write_n(ACE_STDOUT, mb->rd_ptr(), mb->length()); //@

	if ( write(*mb) != 0 ) return -1;

	return 0;

	//start_ssl();
	//return 0;
}

int
HTTP_SSL_Client_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	//static int i = 0;
	//::printf("%d, ", i++);

	return 0;
}

int
HTTP_SSL_Client_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	//ACE_Time_Value tv(0, 10 * 1000);
	//timespec_t t  = (timespec_t) tv;
	//ACE_OS::nanosleep(&t);

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	
	if ( read(*mb_read) != 0 ) return -1;

	return 0;
}


