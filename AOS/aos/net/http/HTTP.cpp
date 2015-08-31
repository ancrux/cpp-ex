#include "HTTP.h"

#include <sstream>

namespace HTTP
{

char* Response::Text::msg_[Response::MAX];

Response::Text Responser::text_;

int
Responser::parse_header(ACE_Message_Block* in, ACE_Message_Block* out, int n_err)
{
	int n_code = n_err; // response code
	char date[128]; // date buffer

	if ( n_err < Response::Bad_Request )
	{
		n_code = Response::Bad_Request;

		Tokenizer tok(in->base(), in->length());
		tok.set_separator(" \t");

		//tok.c_str("GET /sab.txt H"); //@

		int res = tok.next(); // Get Request Type
		// GET
		if ( aos::strnicmp("GET", tok.token(), tok.size()) == 0 )
		{
			string url;

			res = tok.next();
			if ( res > Tokenizer::Last )
			{
				url.assign(tok.token(), tok.size());

				// make sure first line ended with "HTTP/1.0\r\n"
				tok.set_separator("\r\n");
				res = tok.next();
				if ( res > Tokenizer::Last )
					n_code = Response::OK;
			}

			if ( out && n_code == Response::OK )
			{
				int n;
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_Response_, this->version(), n_code, this->get_response_text(n_code)); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Content_Type, "text/plain"); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Date, this->get_gmt_time(ACE_OS::gettimeofday(), date, 128)); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Server, this->get_server_name()); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Connection, "close"); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_End_); out->wr_ptr(n);

				for(int i = 0; i < 10; ++i)
				{
					n = ACE_OS::snprintf(out->wr_ptr(), out->space(), "%s\r\n", url.c_str()); out->wr_ptr(n);
				}
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_CRLF_); out->wr_ptr(n);
			}
		}
		// POST
		else if ( aos::strnicmp("POST", tok.token(), tok.size()) == 0 )
		{
			string url;
			stringstream sstr;
			sstr << "POST\r\n";

			tok.set_separator(" \t\r\n");
			while( (res = tok.next()) > Tokenizer::End )
			{
				string token(tok.token(), tok.size());
				sstr << "<" << token << ">(" << res << ")\r\n";
			}

			n_code = Response::OK;

			if ( out && n_code == Response::OK )
			{
				int n;
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_Response_, this->version(), n_code, this->get_response_text(n_code)); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Content_Type, "text/plain"); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Date, this->get_gmt_time(ACE_OS::gettimeofday(), date, 128)); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Server, this->get_server_name()); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_End_); out->wr_ptr(n);

				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), "%s", sstr.str().c_str()); out->wr_ptr(n);
				n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_CRLF_); out->wr_ptr(n);
			}
		}
		// HEAD
		else if ( aos::strnicmp("HEAD", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}
		// PUT
		else if ( aos::strnicmp("PUT", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}
		// DELETE
		else if ( aos::strnicmp("DELETE", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}
		// TRACE
		else if ( aos::strnicmp("TRACE", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}
		// OPTIONS
		else if ( aos::strnicmp("OPTIONS", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}
		// CONNECT
		else if ( aos::strnicmp("CONNECT", tok.token(), tok.size()) == 0 )
		{
			n_code = Response::Not_Implemented;
		}

	} // n_err == 0

	// Handle Error Resquest ( n_code >= 400 )
	if ( out && n_code >= Response::Bad_Request )
	{
		int n;
		n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_Response_, this->version(), n_code, this->get_response_text(n_code)); out->wr_ptr(n);
		n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Date, this->get_gmt_time(ACE_OS::gettimeofday(), date, 128)); out->wr_ptr(n);
		n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::Server, this->get_server_name()); out->wr_ptr(n);
		n = ACE_OS::snprintf(out->wr_ptr(), out->space(), HTTP::Header::_End_); out->wr_ptr(n);
	}

	return n_code;
}

} // namespace HTTP
