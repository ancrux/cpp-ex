#include "HTTP_Server.h"

#include "HTTP_Server_Connection.h"

namespace asio {

HTTP_Server::HTTP_Server()
{
}

HTTP_Server::~HTTP_Server()
{
	ACE_OS::printf("(dtor) http_server=%p\n", this);
}

SSL_Server_Connection*
HTTP_Server::make_connection()
{
	return new HTTP_Server_Connection(*this);
}

} // namespace asio

