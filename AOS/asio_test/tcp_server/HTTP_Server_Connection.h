#ifndef _ASIO_HTTP_SERVER_CONNECTION_H_
#define _ASIO_HTTP_SERVER_CONNECTION_H_

#include "aos/TCP_Server_Connection.h"

namespace asio {

class HTTP_Server_Connection : public TCP_Server_Connection
{
public:
	HTTP_Server_Connection(TCP_Server& server, long timeout = 5, size_t buf_size = 0);
	virtual ~HTTP_Server_Connection();

public:
	virtual void on_open(ACE_Message_Block& mb); // connection starting point
	virtual void on_read_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_write_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_socket_timeout(int pending_operation, long timeout);

protected:
	std::string buf_;
};

} // namespace asio

#endif // _ASIO_HTTP_SERVER_CONNECTION_H_

