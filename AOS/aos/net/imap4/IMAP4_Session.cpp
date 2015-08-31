#include "IMAP4_Session.h"

/// SSL Client

IMAP4_SSL_Client_Session::IMAP4_SSL_Client_Session()
:
AIO_SSL_Client_Session(),
step_(0)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) ctor(%@):IMAP4_SSL_Client_Session\n"), this)); //@
	io_.open("A001");
}

IMAP4_SSL_Client_Session::~IMAP4_SSL_Client_Session()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) dtor(%@):IMAP4_SSL_Client_Session\n"), this)); //@
	io_.close();
}

int
IMAP4_SSL_Client_Session::on_open(ACE_Message_Block& mb_open)
{

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	if ( read(*mb_read) != 0 ) return -1;

	// initialize a new read
	io_.read_reset();

	//start_ssl();

	return 0;
}

int
IMAP4_SSL_Client_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	//ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	if ( step_ == 0 )
	{
		// read greetings
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_greeting(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: login
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			if ( io_.cmd_login(*mb_write, "ntut", "111111") != IMAP4_Client_IO::WR_OK ) 
			{
				mb_write->release();
				return -1;
			}
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else if ( step_ == 1 )
	{
		// read response
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_line(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: capability
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			if ( io_.cmd_capability(*mb_write) != IMAP4_Client_IO::WR_OK ) 
			{
				mb_write->release();
				return -1;
			}
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else if ( step_ == 2 )
	{
		// read response
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_line(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: user command
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			mb_write->copy("A001 SELECT INBOX\r\n");
			mb_write->wr_ptr(-1); // take '\0' out!
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else if ( step_ == 3 )
	{
		// read response
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_line(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: logout
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			if ( io_.cmd_noop(*mb_write) != IMAP4_Client_IO::WR_OK ) 
			{
				mb_write->release();
				return -1;
			}
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else if ( step_ == 4 )
	{
		// read response
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_line(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: logout
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			mb_write->copy("A001 FETCH 1 BODY[]\r\n");
			mb_write->wr_ptr(-1); // take '\0' out!
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else if ( step_ == 5 )
	{
		// read response
		while( mb.length() > 0 )
		{
			if ( io_.read_state() != IMAP4_Client_IO::RD_DATA )
			{
				aos::bcstr line = io_.read_line(mb);
				ACE_OS::printf("%s", io_.buf().c_str());
			}
			else
			{
				aos::bcstr data = io_.read_data(mb);
				std::string in_data(data.buf, data.len);
				ACE_OS::printf("%s", in_data.c_str());
			}
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		// write command: logout
		else
		{
			ACE_Message_Block* mb_write = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_write ) return -1;
			if ( io_.cmd_logout(*mb_write) != IMAP4_Client_IO::WR_OK ) 
			{
				mb_write->release();
				return -1;
			}
			if ( write(*mb_write) != 0 ) return -1;
		}
	}
	else
	{
		// read response
		while( mb.length() > 0 )
		{
			aos::bcstr line = io_.read_line(mb);
			ACE_OS::printf("%s", io_.buf().c_str());
		}
		// keep reading
		if ( io_.read_state() != IMAP4_Client_IO::RD_OK )
		{
			ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
			if ( !mb_read ) return -1;
			if ( read(*mb_read) != 0 ) return -1;
		}
		return -1;
	}

	//+ if finish reading, get next command and write command...

	return 0;
}

int
IMAP4_SSL_Client_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	//ACE_Time_Value tv(0, 10 * 1000);
	//timespec_t t  = (timespec_t) tv;
	//ACE_OS::nanosleep(&t);

	ACE_OS::write_n(ACE_STDOUT, mb.rd_ptr(), mb.length()); //@

	ACE_Message_Block* mb_read = new (std::nothrow) ACE_Message_Block(BUFSIZE+1);
	if ( !mb_read ) return -1;
	if ( read(*mb_read) != 0 ) return -1;

	// initialize a new read
	++step_;
	io_.read_reset();

	return 0;
}


