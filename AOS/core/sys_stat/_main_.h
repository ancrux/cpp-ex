
#include "_test_pdh_enum_.h"
#include "_test_cpu_usage_.h"
#include "_test_vm_stats_.h"
#include "_test_os_info_.h"
#include "_test_load_stats_.h" // NOT SUPPORTED ON WIN32
#include "_test_process_stats_.h"
#include "_test_process_snapshot_.h" // NOT SUPPORTED ON WIN32
#include "_test_page_stats_.h"
#include "_test_user_list_.h"
#include "_test_disk_traffic_.h"
#include "_test_network_iface_stats_.h"
#include "_test_network_traffic_.h"
#include "_test_fs_stats_.h"

#define __SVC_VER__ "0.1.3 (alpha)"

int run_svc_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	Service_Process svc(argc, argv, __SVC_VER__, __DATE__ " " __TIME__);
	ACE_OS::printf("service: %s\n", svc.name());

	// start local_acceptor
	aos::ipc::Local_Acceptor acceptor;
	ACE_Time_Value timeout(1);

	if ( acceptor.open(svc.name()) == -1 )
	{
		ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
		return -1;
	}

	// start service
	System_Stat sys;
	//Cello_Directory_Updater updater(sys);

	//updater.start(); //ACE_THR_FUNC

	int stop = 0;
	while(!stop)
	{
		aos::ipc::Local_Stream stream;
		if ( acceptor.accept(stream) == -1 )
		{
			//ACE_OS::printf("accept() == -1\n"); //@
			continue;
		}

		//ACE_OS::printf("client accepted!\n"); //@

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		n_recv = stream.recv_cstr();
		//ACE_OS::printf("command: %s\n", stream.buf().c_str()); //@

		if ( (n_send = svc.handle_predefined_command(stream)) > 0 )
		{
			// do nothing
		}
		// read "stop" command, stop service.
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "stop") == 0 )
		{
			char buf[256];
			int n = ACE_OS::snprintf(buf, 255, "+%d\t[%s]\tservice is stopping...", svc.pid(), svc.name());
			n_send = stream.send_cstr(buf, n+1); // include '\0'
			
			if ( n_send > 0 )
			{
				stop = 1;
				// send stop signal, and wait for service to stop
				//updater.stop();
			}
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "cpu_info") == 0 )
		{
			char buf[4096+1];
			std::string cpu;
			int n_cpu = sys.get_cpu_info(cpu);

			int n = ACE_OS::snprintf(buf, 4096,
				"+%d\tOK\n%s",
				((n_cpu>0)?n_cpu:0),
				cpu.c_str()
				);
			if ( n )
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			else
			{
				const char* msg = "-1\tinsufficient buffer!\n";
				size_t n_msg = ACE_OS::strlen(msg);
				n_send = stream.send_cstr(msg, n_msg+1); // include '\0'
			}
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "cpu") == 0 )
		{
			char buf[4096+1];

			float user, kernel, iowait, idle;
			sys.get_cpu_stats(user, kernel, iowait, idle);

			int n = ACE_OS::snprintf(buf, 4096,
				"+0\tOK\n"
				"user\tkernel\tiowait\tidle\n"
				"%f\t%f\t%f\t%f\n",
				user, kernel, iowait, idle
				);
			n_send = stream.send_cstr(buf, n+1); // include '\0'
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "mem") == 0 )
		{
			char buf[4096+1];

			long long total, used, free;
			sys.get_mem_stats(total, used, free);

			int n = ACE_OS::snprintf(buf, 4096,
				"+0\tOK\n"
				"total\tused\tfree\n"
				"%lld\t%lld\t%lld\n",
				total, used, free
				);
			n_send = stream.send_cstr(buf, n+1); // include '\0'
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "swap") == 0 )
		{
			char buf[4096+1];

			long long total, used, free;
			sys.get_swap_stats(total, used, free);

			int n = ACE_OS::snprintf(buf, 4096,
				"+0\tOK\n"
				"total\tused\tfree\n"
				"%lld\t%lld\t%lld\n",
				total, used, free
				);
			n_send = stream.send_cstr(buf, n+1); // include '\0'
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "fs") == 0 )
		{
			char buf[4096+1];
			std::string fs;
			int n_fs = sys.get_fs_stats(fs);

			//int n = ACE_OS::snprintf(buf, 4096, "%s%d\tOK\n%s", ((n_fs>0)?"+":""), n_fs, fs.c_str());
			int n = ACE_OS::snprintf(buf, 4096,
				"+%d\tOK\n%s",
				((n_fs>0)?n_fs:0),
				fs.c_str()
				);
			if ( n )
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			else
			{
				const char* msg = "-1\tinsufficient buffer!\n";
				size_t n_msg = ACE_OS::strlen(msg);
				n_send = stream.send_cstr(msg, n_msg+1); // include '\0'
			}
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "ps") == 0 )
		{
			char buf[32768+1];
			std::string ps;
			int n_ps = sys.get_ps_stats(ps);

			int n = ACE_OS::snprintf(buf, 32768,
				"+%d\tOK\n%s",
				((n_ps>0)?n_ps:0),
				ps.c_str()
				);
			if ( n )
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			else
			{
				const char* msg = "-1\tinsufficient buffer!\n";
				size_t n_msg = ACE_OS::strlen(msg);
				n_send = stream.send_cstr(msg, n_msg+1); // include '\0'
			}
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "os") == 0 )
		{
			char buf[4096+1];
			std::string os;
			int n_os = sys.get_os_stats(os);

			int n = ACE_OS::snprintf(buf, 4096,
				"+%d\tOK\n%s",
				((n_os>0)?n_os:0),
				os.c_str()
				);
			if ( n )
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			else
			{
				const char* msg = "-1\tinsufficient buffer!\n";
				size_t n_msg = ACE_OS::strlen(msg);
				n_send = stream.send_cstr(msg, n_msg+1); // include '\0'
			}
		}
		else if ( ACE_OS::strcasecmp(stream.buf().c_str(), "net_if") == 0 )
		{
			char buf[4096+1];
			std::string net;
			int n_net_if = sys.get_net_if_stats(net);

			int n = ACE_OS::snprintf(buf, 4096,
				"+%d\tOK\n%s",
				((n_net_if>0)?n_net_if:0),
				net.c_str()
				);
			if ( n )
				n_send = stream.send_cstr(buf, n+1); // include '\0'
			else
			{
				const char* msg = "-1\tinsufficient buffer!\n";
				size_t n_msg = ACE_OS::strlen(msg);
				n_send = stream.send_cstr(msg, n_msg+1); // include '\0'
			}
		}
		else
		{
			n_send = svc.handle_unknown_command(stream);
		}
		stream.close();
	}
	acceptor.close();

	//ACE_OS::printf("stop sleep...\n");
	//ACE_OS::sleep(5);
	//ACE_OS::printf("exit!\n");

	return rc;
}

int run_ipc_io_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	Service_Process svc(argc, argv, __SVC_VER__, __DATE__ " " __TIME__);
	ACE_OS::printf("service: %s\n", svc.name());

	// start local_acceptor
	aos::ipc::Local_Acceptor acceptor;
	ACE_Time_Value timeout(1);

	if ( acceptor.open(svc.name()) == -1 )
	{
		ACE_OS::printf("open() failed: %s\n", ACE_OS::strerror(ACE_OS::last_error())); // ACE_OS::perror("open()");
		return -1;
	}

	// start service
	System_Stat sys;

	int stop = 0;
	while(!stop)
	{
		aos::ipc::Local_Stream stream;
		if ( acceptor.accept(stream) == -1 )
		{
			//ACE_OS::printf("accept() == -1\n"); //@
			continue;
		}

		//ACE_OS::printf("client accepted!\n"); //@

		ssize_t n_recv = -1;
		ssize_t n_send = -1;

		static const int bsize = 4095;
		do
		{
			// recv
			n_recv = stream.recv_cstr();
			if ( n_recv < 1 )
				break;

			// send
			char buf[bsize+1];
			memset(buf, 'A', bsize);
			buf[bsize] = '\0';
			n_send = stream.send_cstr(buf, bsize+1); // include '\0'
		}
		while( n_send > 0);

		stream.close();
	}
	acceptor.close();

	//ACE_OS::printf("stop sleep...\n");
	//ACE_OS::sleep(5);
	//ACE_OS::printf("exit!\n");

	return rc;
}

int run_all_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	//rc = run_fs_stats_test(argc, argv); // LOOP
	//rc = run_network_traffic_test(argc, argv); // LOOP
	//rc = run_network_iface_stats_test(argc, argv);
	//rc = run_disk_traffic_test(argc, argv); // LOOP
	//rc = run_user_list_test(argc, argv);
	//rc = run_page_stats_test(argc, argv);
	//rc = run_process_snapshot_test(argc, argv); // NOT SUPPORTED on Win32
	//rc = run_process_stats_test(argc, argv);
	//rc = run_load_stats_test(argc, argv); // NOT SUPPORTED on Win32
	//rc = run_os_info_test(argc, argv);
	//rc = run_vm_stats_test(argc, argv);
	rc = run_cpu_usage_test(argc, argv); // LOOP

	//rc = run_pdh_enum_test(argc, argv); // Win32 only
	//rc = run_pdh_enum_test2(argc, argv); // Win32 only

	return rc;
}

