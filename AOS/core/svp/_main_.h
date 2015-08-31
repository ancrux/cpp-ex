
#include "ace/Atomic_Op.h"
#include "ace/Task.h"
#include "ace/Process_Manager.h"

#include "aos/IPC.h"

class IPC_Handler : public ACE_Task_Base
{
public:
	IPC_Handler(aos::ipc::Service_Process& sp, aos::ipc::Local_Acceptor& acceptor, const std::string& stop_cmd);
	~IPC_Handler();

public:
	void start();
	void stop();

public:
	virtual int svc();

protected:
	aos::ipc::Service_Process& sp_;
	aos::ipc::Local_Acceptor& acceptor_;
	std::string stop_cmd_;

	ACE_Atomic_Op<ACE_Thread_Mutex, long> stop_;
};

IPC_Handler::IPC_Handler(aos::ipc::Service_Process& sp, aos::ipc::Local_Acceptor& acceptor, const std::string& stop_cmd)
:
sp_(sp),
acceptor_(acceptor),
stop_cmd_(stop_cmd)
{
}

IPC_Handler::~IPC_Handler()
{
}

void
IPC_Handler::start()
{
	stop_ = 0;
	if ( this->activate() != -1 )
	{
		// wait till thread is created
		while( !this->thr_count() )
			continue;
	}
}

void
IPC_Handler::stop()
{
	stop_ = 1;
	this->wait();
}

int
IPC_Handler::svc()
{
	while( !stop_.value() )
	{
		ACE_Time_Value accept_timeout; accept_timeout.set(0.1);
		aos::ipc::Local_Stream stream;
		if ( acceptor_.accept(stream, 0, &accept_timeout) == -1 )
		{
			//ACE_OS::printf("accept() == -1\n"); //@
			continue;
		}

		//ACE_OS::printf("client accepted!\n"); //@

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		n_recv = stream.recv_cstr();
		//ACE_OS::printf("command: %s\n", stream.buf().c_str()); //@

		// read "stop" command, stop service.
		int n = 0;
		if ( (n_send = sp_.handle_predefined_command(stream)) > 0 )
		{
			// do nothing
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "stop") == 0 )
		{
			char buf[256];
			n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\tservice is stopping...", sp_.pid(), sp_.name());
			n_send = stream.send_cstr(buf, n+1); // include '\0'

			if ( n_send > 0 )
			{
				stop_ = 1;

#ifdef ACE_WIN32
				int stop_signal = SIGINT; // SIGINT(2), SIGQUIT(3), SIGKILL(9), SIGTERM(15)
#else
				int stop_signal = SIGKILL; // SIGINT(2), SIGQUIT(3), SIGKILL(9), SIGTERM(15)
#endif

				//+ if stop_command_ is empty or digits, send stop signal
				//+ else exec stop_command_
				if ( stop_cmd_.empty() || (stop_cmd_[0] >= '0' && stop_cmd_[0] <= '9') )
				{
					if ( stop_cmd_.size() )
						stop_signal = ACE_OS::atoi(stop_cmd_.c_str());

					// send stop signal 
					// Win32:
					// For a GUI App, the "normal" way to handle this in Windows development would be to send a WM_CLOSE message to the process's main window.
					// For a console app, you need to use SetConsoleCtrlHandler to add a CTRL_C_EVENT.
					// If the application doesn't honor that, you could call TerminateProcess.
					// POSIX:
					// kill -signal pid
#ifdef ACE_WIN32
					//::GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0); // won't work if process is created detached
					ACE::terminate_process(sp_.pid());
#else
					ACE_OS::kill(sp_.pid(), stop_signal);
#endif
				}
				else
				{
					ACE_Process_Options opt;
					opt.command_line(stop_cmd_.c_str());
					char* work_dir = ACE_OS::realpath(opt.process_name(), buf);
					if ( work_dir )
						opt.working_directory(ACE::dirname(buf));

#ifdef ACE_WIN32
					opt.creation_flags(DETACHED_PROCESS); //Win32 use DETACHED_PROCESS, or CREATE_NEW_CONSOLE
#endif

					///*
					ACE_Process p;
					pid_t pid = p.spawn(opt);

					ACE_exitcode ret = 0;
					if ( pid > 0 )
						pid = p.wait(&ret);

					if ( pid <= 0 || ret != 0 )
						ACE::terminate_process(sp_.pid());
					//*/

					/*
					int ret = ACE_OS::system(opt.command_line_buf());
					if ( ret != 0 )
					{
					//::printf("ret:%d\n", ret);
					ACE::terminate_process(sp_.pid());
					}
					//*/
				}
			}
		}
		else
		{
			n_send = sp_.handle_unknown_command(stream);
		}
		stream.close();
	}
	acceptor_.close();

	return 0;
}
