#ifndef _HTTP_H_
#define _HTTP_H_

#include "ace/OS.h"
#include "ace/Task.h"

#include "aos/String.h"
using namespace std;
using namespace aos;

namespace HTTP {

namespace Header {
static const char* _Request_ = "%s %s HTTP/%.1f\r\n"; // rename to HTTP_Request
static const char* _Response_ = "HTTP/%.1f %d %s\r\n"; // rename to HTTP_Response
static const char* _Header_ = "%s: %s\r\n"; // User-defined Header // rename to HTTP_Header
static const char* _End_ = "\r\n"; // End of Header
static const char* _CRLF_ = "\r\n"; // CRLF = End of Header/Content // rename to CRLF

static const char* Connection = "Connection: %s\r\n";
static const char* Content_Length = "Content-Length: %s\r\n";
static const char* Content_Type = "Content-Type: %s\r\n";
static const char* Date = "Date: %s\r\n";
static const char* Host = "Host: %s\r\n";
static const char* Last_Modified = "Last-Modified: %s\r\n";
static const char* Location = "Location: %s\r\n";
static const char* Range = "Range: %s\r\n";
static const char* Server = "Server: %s\r\n";
} // namespace Header

namespace Response {

enum
{
	// Range:100-199 Defined:100-101 Category:Informational
	Continue = 100, // An initial part of the request was received, and the client should continue.
	Switching_Protocols = 101, // The server is changing protocols, as specified by the client, to one listed in the Upgrade header.

	// Range:200-299 Defined:200-206 Category:Successful
	OK = 200, // The request is okay.
	Created = 201, // The resource was created (for requests that create server objects).
	Accepted = 202, // The request was accepted, but the server has not yet performed any action with it.
	Non_Authoritative_Information = 203, // The transaction was okay, except the information contained in the entity headers was not from the origin server, but from a copy of the resource.
	No_Content = 204, // The response message contains headers and a status line, but no entity body.
	Reset_Content = 205, //  Another code primarily for browsers; basically means that the browser should clear any HTML form elements on the current page.
	Partial_Content = 206, //  A partial request was successful.

	// Range:300-399 Defined:300-305 Category:Redirection
	Multiple_Choices = 300, // A client has requested a URL that actually refers to multiple resources. This code is returned along with a list of options; the user can then select which one he wants.
	Moved_Permanently = 301, // The requested URL has been moved. The response should contain a Location URL indicating where the resource now resides.
	Found = 302, //  Like the 301 status code, but the move is temporary. The client should use the URL given in the Location header to locate the resource temporarily.
	See_Other = 303, // Tells the client that the resource should be fetched using a different URL. This new URL is in the Location header of the response message.
	Not_Modified = 304, // Clients can make their requests conditional by the request headers they include. This code indicates that the resource has not changed.
	Use_Proxy = 305, // The resource must be accessed through a proxy, the location of the proxy is given in the Location header.
	Unused = 306, // This status code currently is not used.
	Temporary_Redirect = 307, // Like the 301 status code; however, the client should use the URL given in the Location header to locate the resource temporarily.

	// Range:400-499 Defined:400-415 Category:Client error
	Bad_Request = 400, // Tells the client that it sent a malformed request.
	Unauthorized = 401, // Returned along with appropriate headers that ask the client to authenticate itself before it can gain access to the resource.
	Payment_Required = 402, // Currently this status code is not used, but it has been set aside for future use.
	Forbidden = 403, // The request was refused by the server.
	Not_Found = 404, // The server cannot find the requested URL.
	Method_Not_Allowed = 405, // A request was made with a method that is not supported for the requested URL. The Allow header should be included in the response to tell the client what methods are allowed on the requested resource.
	Not_Acceptable = 406, // Clients can specify parameters about what types of entities they are willing to accept. This code is used when the server has no resource matching the URL that is acceptable for the client.
	Proxy_Authentication_Required = 407, // Like the 401 status code, but used for proxy servers that require authentication for a resource.
	Request_Timeout = 408, // If a client takes too long to complete its request, a server can send back this status code and close down the connection.
	Conflict = 409, // The request is causing some conflict on a resource.
	Gone = 410, // Like the 404 status code, except that the server once held the resource.
	Length_Required = 411, // Servers use this code when they require a Content-Length header in the request message. The server will not accept requests for the resource without the Content-Length header.
	Precondition_Failed = 412, // If a client makes a conditional request and one of the conditions fails, this response code is returned.
	Request_Entity_Too_Large = 413, // The client sent an entity body that is larger than the server can or wants to process.
	Request_URI_Too_Long = 414, // The client sent a request with a request URL that is larger than what the server can or wants to process.
	Unsupported_Media_Type = 415, // The client sent an entity of a content type that the server does not understand or support.
	Requested_Range_Not_Satisfiable = 416, // The request message requested a range of a given resource, and that range either was invalid or could not be met.
	Expectation_Failed = 417, // The request contained an expectation in the Expect request header that could not be satisfied by the server.

	// Range:500-599 Defined:500-505 Category:Server error
	Internal_Server_Error = 500, // The server encountered an error that prevented it from servicing the request.
	Not_Implemented = 501, // The client made a request that is beyond the server's capabilities.
	Bad_Gateway = 502, //  A server acting as a proxy or gateway encountered a bogus response from the next link in the request response chain.
	Service_Unavailable = 503, // The server cannot currently service the request but will be able to in the future.
	Gateway_Timeout = 504, // Similar to the 408 status code, except that the response is coming from a gateway or proxy that has timed out waiting for a response to its request from another server.
	HTTP_Version_Not_Supported = 505, // The server received a request in a version of the protocol that it can't or won't support.
	
	MAX = 600
};

class Text
{
public:
	Text()
	{
		::memset(msg_, 0, sizeof(msg_));

		msg_[Response::Continue] = "Continue";
		msg_[Response::Switching_Protocols] = "Switching Protocol";

		msg_[Response::OK] = "OK";
		msg_[Response::Created] = "Created";
		msg_[Response::Accepted] = "Accepted";
		msg_[Response::Non_Authoritative_Information] = "Non Authoritative Information";
		msg_[Response::No_Content] = "No Content";
		msg_[Response::Reset_Content] = "Reset Content";
		msg_[Response::Partial_Content] = "Partial Content";

		msg_[Response::Multiple_Choices] = "Multiple Choices";
		msg_[Response::Moved_Permanently] = "Moved Permanently";
		msg_[Response::Found] = "Found";
		msg_[Response::See_Other] = "See Other";
		msg_[Response::Not_Modified] = "Not Modified";
		msg_[Response::Use_Proxy] = "Use Proxy";
		msg_[Response::Unused] = "Unused";
		msg_[Response::Temporary_Redirect] = "Temporary Redirect";

		msg_[Response::Bad_Request] = "Bad Request";
		msg_[Response::Unauthorized] = "Unauthorized";
		msg_[Response::Payment_Required] = "Payment Required";
		msg_[Response::Forbidden] = "Forbidden";
		msg_[Response::Not_Found] = "Not Found";
		msg_[Response::Method_Not_Allowed] = "Method Not Allowed";
		msg_[Response::Not_Acceptable] = "Not Acceptable";
		msg_[Response::Proxy_Authentication_Required] = "Proxy Authentication Required";
		msg_[Response::Request_Timeout] = "Request Timeout";
		msg_[Response::Conflict] = "Conflict";
		msg_[Response::Gone] = "Gone";
		msg_[Response::Length_Required] = "Length Required";
		msg_[Response::Precondition_Failed] = "Precondition Failed";
		msg_[Response::Request_Entity_Too_Large] = "Request Entity Too Large";
		msg_[Response::Request_URI_Too_Long] = "Request URI Too Long";
		msg_[Response::Unsupported_Media_Type] = "Unsupported Media Type";
		msg_[Response::Requested_Range_Not_Satisfiable] = "Requested Range Not Satisfiable";
		msg_[Response::Expectation_Failed] = "Expectation Failed";

		msg_[Response::Internal_Server_Error] = "Internal Server Error";
		msg_[Response::Not_Implemented] = "Not Implemented";
		msg_[Response::Bad_Gateway] = "Bad Gateway";
		msg_[Response::Service_Unavailable] = "Service Unavailable";
		msg_[Response::Gateway_Timeout] = "Gateway Timeout";
		msg_[Response::HTTP_Version_Not_Supported] = "HTTP Version Not Supported";

		//::printf("%p:ctor\r\n", this); //@
	};
	~Text()
	{
		//::printf("%p:dtor\r\n", this); //@
	};
	const char* operator[](int index) const
	{
		return msg_[index];
	};

protected:
	static char* msg_[Response::MAX];
};

} // namespace Response

class Responser
{
public:
	inline const char* get_response_text(int code)
	{
		return text_[code];
	};
	inline const char* get_server_name()
	{
		return "Mini (Angus) v0.1";
	};
	inline const char* get_gmt_time(const ACE_Time_Value& time, char* buf, size_t size)
	{
		time_t t = time.sec();
		struct tm* tm_t = ACE_OS::gmtime(&t);
		ACE_OS::strftime(buf, size, "%a, %d %b %Y %H:%M:%S GMT", tm_t);

		return buf;
	}
	inline double version() const { return 1.0; };

public:
	int parse_header(ACE_Message_Block* in, ACE_Message_Block* out, int n_err = 0);

protected:
	static Response::Text text_;
};

} // namespace HTTP

#endif // _HTTP_H_
