
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_process_stats_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	int delay = 1;
	sg_process_count *process_stat;

	//extern char *optarg;
	//int c;
	//while ((c = getopt(argc, argv, "d:")) != -1){
	//	switch (c){
	//		case 'd':
	//			delay = atoi(optarg);
	//			break;
	//	}
	//}

#ifdef WIN32
	delay = delay * 1000;
#endif

	/* Initialise statgrab */
	sg_init();

	/* Drop setuid/setgid privileges. */
	if (sg_drop_privileges() != 0) {
		perror("Error. Failed to drop privileges");
		return 1;
	}

	int n_loop = 0;
	while( (process_stat = sg_get_process_count()) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;

		printf("Running:%3d \t", process_stat->running);
		printf("Sleeping:%5d \t",process_stat->sleeping);
		printf("Stopped:%4d \t", process_stat->stopped);
		printf("Zombie:%4d \t", process_stat->zombie);
		printf("Total:%5d\n", process_stat->total);
#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif
	}

	return rc;
}
