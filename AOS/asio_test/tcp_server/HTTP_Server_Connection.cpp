#include "HTTP_Server_Connection.h"

namespace asio {

HTTP_Server_Connection::HTTP_Server_Connection(TCP_Server& server, long timeout, size_t buf_size)
:
TCP_Server_Connection(server, timeout, buf_size)
{
}

HTTP_Server_Connection::~HTTP_Server_Connection()
{
	//ACE_OS::printf("http_dtor=%p, ssl=%d\n", this, this->is_ssl_); //@
}

void
HTTP_Server_Connection::on_open(ACE_Message_Block& mb)
{
	this->read(mb);
}

void
HTTP_Server_Connection::on_read_some(ACE_Message_Block& mb, size_t bytes_transferred)
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
		std::string response = "HTTP/1.1 200 OK\r\n" \
			"Content-Type: text/plain\r\n" \
			"Connection: close\r\n" \
			"\r\n";

		response += buf_;
		buf_.resize(0);

		mb.reset();
		mb.copy(response.c_str(), response.size());

		this->write(mb);
	}
}

void
HTTP_Server_Connection::on_write_some(ACE_Message_Block& mb, size_t bytes_transferred)
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
		this->close();
	}	
}

void
HTTP_Server_Connection::on_socket_timeout(int pending_operation, long timeout)
{
	//ACE_OS::printf("socket timeout! op_code=%d timeout=%d\n", pending_operation, timeout); //@
}

} // namespace asio


