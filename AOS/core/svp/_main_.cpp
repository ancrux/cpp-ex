#include "ace/OS.h"
#include "ace/Get_Opt.h"
#include "ace/Log_Msg.h"
#include "ace/Process_Manager.h"

#include "aos/IPC.h"
using namespace aos::ipc;

#include <iostream>
using namespace std;

#include "_main_.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{	
	int rc = -1;

	char path[PATH_MAX+1];
	ACE_OS::chdir(ACE::dirname(ACE_OS::realpath(argv[0], path))); 

	std::string cmd; // command
	std::string ver; // version
	std::string bts; // build_timestamp
	int bts_arg = 0; // build_timestamp from command argv[i]
	std::string stop_cmd; // stop command
	std::string wait_pid; // wait for pid

	// Get_Opt
	ACE_Get_Opt get_opt(argc, argv, ACE_TEXT("c:s:p:v:b:"));

	int ch;
	while( (ch = get_opt()) != EOF )
	{
		switch(ch)
		{
		case 'c':
			cmd = get_opt.opt_arg();
			break;
		case 's':
			stop_cmd = get_opt.opt_arg();
			break;
		case 'p':
			wait_pid = get_opt.opt_arg();
			break;
		case 'v':
			ver = get_opt.opt_arg();
			break;
		case 'b':
			bts = get_opt.opt_arg();
			bts_arg = ACE_OS::atoi(get_opt.opt_arg());
			break;
		}
	}

	if ( cmd.empty() )
	{
		ACE_OS::printf("-c 'command' is required!\n");
		return rc;
	}

	//ACE_OS::printf("cmd: %s\n", cmd.c_str()); //@

	ACE_Process_Options opt;
	opt.command_line(cmd.c_str());
	char* work_dir = ACE_OS::realpath(opt.process_name(), path);
	if ( work_dir )
		opt.working_directory(ACE::dirname(path));

	if ( bts.empty() || (bts.size() && bts[0] >= '0' && bts[0] <= '9') )
	{
		// use realpath & ACE_stat to check argument's file time as build_timestamp
		std::string bts_file;
		for(int i = 0; opt.command_line_argv()[i] != 0; ++i)
		{
			if ( i == bts_arg )
			{
				//ACE_OS::printf("[%d]%s\n", i, opt.command_line_argv()[i]); //@
				bts_file = opt.command_line_argv()[i];
				break;
			}
		}
#ifdef ACE_WIN32
		if ( bts_arg == 0 && bts_file.size() > 4 && ACE_OS::strncasecmp(bts_file.c_str() + bts_file.size() - 4, ".exe", 4) != 0 )
			bts_file += ".exe";
#endif
		char* exist = ACE_OS::realpath(bts_file.c_str(), path);
		if ( exist )
		{
			ACE_stat stat;
			if ( ACE_OS::lstat(path, &stat) != -1 && (stat.st_mode & S_IFMT) == S_IFREG )
			{
				struct tm tm_val;
				ACE_OS::localtime_r(&stat.st_mtime, &tm_val);
				char date[64];
				size_t n_date = ACE_OS::strftime(date, 63, "%b %d %Y %H:%M:%S", &tm_val);
				bts.assign(date, n_date);
			}
		}
	}

	Service_Process_Proxy sp(argc, argv, ver.size()?ver.c_str():0, bts.size()?bts.c_str():0);

	// initialize local_acceptor
	aos::ipc::Local_Acceptor acceptor;
	if ( acceptor.open(sp.name()) == -1 )
	{
		ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
		return rc;
	}

#ifdef ACE_WIN32
	opt.creation_flags(DETACHED_PROCESS); //Win32 use DETACHED_PROCESS, or CREATE_NEW_CONSOLE
	//opt.creation_flags(CREATE_NO_WINDOW);
#endif

	//ACE_Process_Manager* pm = ACE_Process_Manager::instance();
	//pid_t pid = pm->spawn(opt);
	ACE_Process p;
	pid_t pid = p.spawn(opt);

	if ( pid > 0 )
	{
		sp.proxy_pid(pid);
		ACE_OS::printf("service: %s\n", sp.name());

		/*
		// initialize local_acceptor
		aos::ipc::Local_Acceptor acceptor;
		if ( acceptor.open(sp.name()) == -1 )
		{
			ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
			
			//? if cannot acceptor.open() failed, should terminate process immediately!
			// but now, keep waiting for process to terminate
			ACE_exitcode ret = 0;
			//pm->wait(pid, &ret);
			pid = p.wait(&ret);

			return rc;
		}
		//*/

		IPC_Handler handler(sp, acceptor, stop_cmd);
		handler.start();

		//ACE_OS::printf("start p.wait()\n"); //@

		ACE_exitcode ret = 0;
		//pm->wait(pid, &ret);
		pid = p.wait(&ret);
		rc = (int) ret;

		//ACE_OS::printf("stop p.wait()\n"); //@

		// if '-p <pid>' is set, wait for pid
		if ( !wait_pid.empty() )
		{
			pid_t pid_to_wait = 0;
			if ( wait_pid[0] >= '0' && wait_pid[0] <= '9' )
			{
				pid_to_wait = (pid_t) ACE_OS::atoi(wait_pid.c_str());
			}
			else
			{
				FILE* fp = ACE_OS::fopen(wait_pid.c_str(), "rb");
				if ( fp )
				{
					static const int LINE = 32;
					char line[LINE+1];
					ACE_OS::memset(line, 0, LINE+1);

					ACE_OS::fgets(line, LINE, fp);
					pid_to_wait = (pid_t) ACE_OS::atoi(line);
					ACE_OS::fclose(fp);
				}
			}

			if ( pid_to_wait > 0 )
			{
				//? use suspend()/resume() to guard thread safety on proxy_pid
				handler.suspend(); //@
				sp.proxy_pid(pid_to_wait);
				handler.resume(); //@

				ACE_OS::waitpid(pid_to_wait, &ret);	
			}
		}

		handler.stop();

		rc = (int) ret;
	}
	else
	{
		ACE_OS::printf("cannot execute '%s'!\n", opt.command_line_buf());
	}

	//ACE_OS::printf("rc: %d\n", rc); //@

	return rc;
}
