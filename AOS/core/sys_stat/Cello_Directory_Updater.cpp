#include "Cello_Directory_Updater.h"

#include "ace/INET_Addr.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Connector.h"

#include <string>

Cello_Directory_Updater::Cello_Directory_Updater(System_Stat& sys)
:
sys_(sys)
{
}

Cello_Directory_Updater::~Cello_Directory_Updater()
{
}

void
Cello_Directory_Updater::start()
{
	stop_ = 0;
	this->activate();
}

void
Cello_Directory_Updater::stop()
{
	stop_ = 1;
	this->wait();
}

int
Cello_Directory_Updater::svc(void)
{
	ACE_INET_Addr server_addr(7878, ACE_LOCALHOST);
	ACE_SOCK_Connector connector;

	while( stop_ == 0 )
	{
		ACE_SOCK_Stream stream;
		ACE_Time_Value conn_timeout(1); //conn_timeout.set(0.5);
		
		if ( connector.connect(stream, server_addr, &conn_timeout) == -1 )
		{
			//ACE_OS::printf("connect() == -1, error(%d): %s\n", ACE_OS::last_error(), ACE_OS::strerror(ACE_OS::last_error()));
			if ( stop_ == 0 && ACE_OS::last_error() != ETIMEDOUT )
				ACE_OS::sleep(conn_timeout);
			continue;
		}

		ACE_Message_Block mb(4096); // read/write buffer
		ACE_Time_Value timeout(10);
		int flags = 0;
		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		std::string response;

		int c = 0;
		while( stop_ == 0 )
		{
			// set cpu
			float user, kernel, iowait, idle;
			sys_.get_cpu_stats(user, kernel, iowait, idle);

			mb.reset();
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "set\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "dn:/tmp/monitoring//cpu\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "oc:cpu_usage\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "cpu_user:%f\r\n", user));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "cpu_kernel:%f\r\n", kernel));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "cpu_iowait:%f\r\n", iowait));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "cpu_idle:%f\r\n", idle));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
			if ( n_send < 1 ) break; // write failed, close connection

			response.resize(0);
			while(true)
			{
				mb.reset();
				n_recv = stream.recv(mb.base(), mb.size(), flags, &timeout);
				if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				else
					response.append(mb.base(), n_recv);
				// process buffer
				if ( response.size() > 32768 )
				{
					// response is too long!
					n_recv = 0 - ((ssize_t) response.size());
					break;
				}
				if ( response.size() >= 4 )
				{
					const char* end = response.c_str() + response.size() - 4;
					if ( ACE_OS::strncmp(end, "\r\n\r\n", 4) == 0 )
						break;
				}
			}
			if ( n_send < 1 || n_recv < 1 )
				break;

			// for mem & swap
			long long total, used, free;

			// set mem
			sys_.get_mem_stats(total, used, free);

			mb.reset();
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "set\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "dn:/tmp/monitoring//mem\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "oc:space_usage\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_total:%lld\r\n", total));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_used:%lld\r\n", used));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_free:%lld\r\n", free));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
			if ( n_send < 1 ) break; // write failed, close connection

			response.resize(0);
			while(true)
			{
				mb.reset();
				n_recv = stream.recv(mb.base(), mb.size(), flags, &timeout);
				if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				else
					response.append(mb.base(), n_recv);
				// process buffer
				if ( response.size() > 32768 )
				{
					// response is too long!
					n_recv = 0 - ((ssize_t) response.size());
					break;
				}
				if ( response.size() >= 4 )
				{
					const char* end = response.c_str() + response.size() - 4;
					if ( ACE_OS::strncmp(end, "\r\n\r\n", 4) == 0 )
						break;
				}
			}
			if ( n_send < 1 || n_recv < 1 )
				break;

			// set swap
			sys_.get_swap_stats(total, used, free);

			mb.reset();
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "set\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "dn:/tmp/monitoring//swap\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "oc:space_usage\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_total:%lld\r\n", total));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_used:%lld\r\n", used));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_free:%lld\r\n", free));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
			if ( n_send < 1 ) break; // write failed, close connection

			response.resize(0);
			while(true)
			{
				mb.reset();
				n_recv = stream.recv(mb.base(), mb.size(), flags, &timeout);
				if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				else
					response.append(mb.base(), n_recv);
				// process buffer
				if ( response.size() > 32768 )
				{
					// response is too long!
					n_recv = 0 - ((ssize_t) response.size());
					break;
				}
				if ( response.size() >= 4 )
				{
					const char* end = response.c_str() + response.size() - 4;
					if ( ACE_OS::strncmp(end, "\r\n\r\n", 4) == 0 )
						break;
				}
			}
			if ( n_send < 1 || n_recv < 1 )
				break;

			// set fs
			std::string fs;
			int n_fs = sys_.get_fs_stats(fs);

			std::string devices, types, mounts, totals, useds, frees;
			aos::Tokenizer toker(fs.c_str(), fs.size());
			toker.set_separator("\t\n");
			int ch;
			for(int i = 0; i <= n_fs; ++i)
			{
				// the first line is column names, skip it!
				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { devices.append(toker.token(), toker.size()); devices += ","; }

				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { types.append(toker.token(), toker.size()); types += ","; }

				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { mounts.append(toker.token(), toker.size()); mounts += ","; }

				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { totals.append(toker.token(), toker.size()); totals += ","; }

				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { useds.append(toker.token(), toker.size()); useds += ","; }

				if ( (ch = toker.next()) <= aos::Tokenizer::End ) break;
				if (i) { frees.append(toker.token(), toker.size()); frees += ","; }
			}

			mb.reset();
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "set\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "dn:/tmp/monitoring//fs\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "oc:fs_info\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "oc:space_usage\r\n"));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "dev_name:%s\r\n", devices.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "fs_type:%s\r\n", types.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "mnt_point:%s\r\n", mounts.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_total:%s\r\n", totals.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_used:%s\r\n", useds.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "space_free:%s\r\n", frees.c_str()));
			mb.wr_ptr(ACE_OS::snprintf(mb.wr_ptr(), mb.space(), "\r\n"));
			n_send = stream.send_n(mb.rd_ptr(), mb.length(), flags, &timeout);
			if ( n_send < 1 ) break; // write failed, close connection

			response.resize(0);
			while(true)
			{
				mb.reset();
				n_recv = stream.recv(mb.base(), mb.size(), flags, &timeout);
				if ( n_recv < 1 && ACE_OS::last_error() != EWOULDBLOCK )
					break;
				else
					response.append(mb.base(), n_recv);
				// process buffer
				if ( response.size() > 32768 )
				{
					// response is too long!
					n_recv = 0 - ((ssize_t) response.size());
					break;
				}
				if ( response.size() >= 4 )
				{
					const char* end = response.c_str() + response.size() - 4;
					if ( ACE_OS::strncmp(end, "\r\n\r\n", 4) == 0 )
						break;
				}
			}
			if ( n_send < 1 || n_recv < 1 )
				break;

			//ACE_OS::printf("%s", response.c_str()); //@
			//ACE_OS::printf("count: %d\n", count_); //@

			ACE_OS::sleep(1);
			++c;
		}
		stream.close();
	}

	return 0;
}

