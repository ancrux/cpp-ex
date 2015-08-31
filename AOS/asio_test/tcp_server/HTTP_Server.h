#ifndef _ASIO_HTTP_SERVER_H_
#define _ASIO_HTTP_SERVER_H_

#include "aos/TCP_Server.h"

namespace asio {

class HTTP_Server : public TCP_Server
{
public:
	HTTP_Server();
	virtual ~HTTP_Server();

public:
	virtual TCP_Server_Connection* make_connection();
};

} // namespace asio

#endif // _ASIO_HTTP_SERVER_H_

