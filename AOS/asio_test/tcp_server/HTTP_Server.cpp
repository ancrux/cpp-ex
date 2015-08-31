#include "HTTP_Server.h"

#include "HTTP_Server_Connection.h"

namespace asio {

HTTP_Server::HTTP_Server()
{
}

HTTP_Server::~HTTP_Server()
{
	//ACE_OS::printf("(dtor) http_server=%p\n", this);
}

TCP_Server_Connection*
HTTP_Server::make_connection()
{
	///*
	TCP_Server_Connection* conn = new HTTP_Server_Connection(*this);
	//ACE_OS::printf("make_conn=%p\n", conn); //@
	return conn;
	/**/
}

} // namespace asio

