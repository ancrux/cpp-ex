#include "HTTP_Client_Connection.h"

namespace asio {

HTTP_Client_Connection::HTTP_Client_Connection(TCP_Client& client)
:
TCP_Client_Connection(client)
{
}

HTTP_Client_Connection::~HTTP_Client_Connection()
{
	//ACE_OS::printf("http_dtor=%p, ssl=%d\n", this, this->is_ssl_); //@
}

void
HTTP_Client_Connection::on_open(ACE_Message_Block& mb)
{
	std::string request = "GET / HTTP/1.1\r\n" \
		"Host: localhost\r\n" \
		"\r\n";
	mb.copy(request.c_str(), request.size());

	this->write(mb);
}

void
HTTP_Client_Connection::on_read_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	//ACE_Time_Value sleep_tv; sleep_tv.set(0.01);
	//ACE_OS::sleep(sleep_tv);

	// copy mb to buf
	buf_.append(mb.rd_ptr(), mb.length());

	if ( buf_.rfind("\r\n\r\n") == std::string::npos )
	{
		mb.reset();
		this->read(mb);
	}
	else
	{
		//ACE_OS::printf("%s", buf_.c_str()); //@
		//ACE_OS::printf("+");
		++get_client().n_ok;

		this->close();
	}
}

void
HTTP_Client_Connection::on_write_some(ACE_Message_Block& mb, size_t bytes_transferred)
{
	// keep writing
	if ( mb.length() > 0 )
	{
		this->write(mb);
		return;
	}
	// write completed
	else
	{
		buf_.resize(0);

		mb.reset();
		this->read(mb);
	}	
}

void
HTTP_Client_Connection::on_socket_timeout(int pending_operation, long timeout)
{
	//ACE_OS::printf("socket timeout! op_code=%d timeout=%d\n", pending_operation, timeout); //@
	//ACE_OS::printf("T");
	if ( pending_operation == CONNECT )
		++get_client().n_ct;
	else
		++get_client().n_st;
}

void
HTTP_Client_Connection::on_connect_error(const error_code& error)
{
	//ACE_OS::printf("connect error:%s\n", error.message().c_str()); //@
	//ACE_OS::printf("C");
	++get_client().n_ce;
}

void
HTTP_Client_Connection::on_ssl_handshake_error(const error_code& error)
{
	//ACE_OS::printf("ssl_handshake error:%s\n", error.message().c_str()); //@
	//ACE_OS::printf("H");
	++get_client().n_he;
}

void
HTTP_Client_Connection::on_read_error(const error_code& error)
{
	//ACE_OS::printf("read error:%s\n", error.message().c_str()); //@
	//ACE_OS::printf("R");
	++get_client().n_re;
}

void
HTTP_Client_Connection::on_write_error(const error_code& error)
{
	//ACE_OS::printf("write error:%s\n", error.message().c_str()); //@
	//ACE_OS::printf("W");
	++get_client().n_we;
}


} // namespace asio


