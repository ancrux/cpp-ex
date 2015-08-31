#include "HTTP_Client.h"

#include "HTTP_Client_Connection.h"

namespace asio {

HTTP_Client::HTTP_Client()
{
}

HTTP_Client::~HTTP_Client()
{
	//ACE_OS::printf("(dtor) http_server=%p\n", this);
}

TCP_Client_Connection*
HTTP_Client::make_connection()
{
	TCP_Client_Connection* conn = new HTTP_Client_Connection(*this);
	//ACE_OS::printf("make_conn=%p\n", conn); //@
	return conn;
}

} // namespace asio

