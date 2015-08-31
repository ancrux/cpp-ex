#ifndef _ASIO_HTTP_CLIENT_CONNECTION_H_
#define _ASIO_HTTP_CLIENT_CONNECTION_H_

#include "aos/TCP_Client_Connection.h"

#include "HTTP_Client.h"

namespace asio {

class HTTP_Client_Connection : public TCP_Client_Connection
{
public:
	HTTP_Client_Connection(TCP_Client& client);
	virtual ~HTTP_Client_Connection();

public:
	virtual void on_open(ACE_Message_Block& mb); // connection starting point
	virtual void on_read_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_write_some(ACE_Message_Block& mb, size_t bytes_transferred);
	virtual void on_socket_timeout(int pending_operation, long timeout);

public:
	virtual void on_connect_error(const error_code& error);
	virtual void on_ssl_handshake_error(const error_code& error);
	virtual void on_read_error(const error_code& error);
	virtual void on_write_error(const error_code& error);

public: // the_client()
	HTTP_Client& get_client() { return (HTTP_Client&) client(); };

protected:
	std::string buf_;
};

} // namespace asio

#endif // _ASIO_HTTP_CLIENT_CONNECTION_H_

