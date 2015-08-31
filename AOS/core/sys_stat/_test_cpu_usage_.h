
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_cpu_usage_test(int argc, ACE_TCHAR* argv[])
{
	int rc = 0;

	int delay = 1;
	sg_cpu_percents *cpu_percent;

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

	/* Throw away the first reading as thats averaged over the machines uptime */
	sg_snapshot();
	cpu_percent = sg_get_cpu_percents();

	/* Clear the screen ready for display the cpu usage */
	//printf("\033[2J");

	int n_loop = 0;
	while( (cpu_percent = sg_get_cpu_percents()) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;
		
		sg_snapshot();
		//printf("\033[2;2H%-12s : %6.2f", "User CPU", cpu_percent->user);
		//printf("\033[3;2H%-12s : %6.2f", "Kernel CPU", cpu_percent->kernel);
		//printf("\033[4;2H%-12s : %6.2f", "IOWait CPU", cpu_percent->iowait);
		//printf("\033[5;2H%-12s : %6.2f", "Swap CPU", cpu_percent->swap);
		//printf("\033[6;2H%-12s : %6.2f", "Nice CPU", cpu_percent->nice);
		//printf("\033[7;2H%-12s : %6.2f", "Idle CPU", cpu_percent->idle);
		printf("%-12s : %6.2f\n", "User CPU", cpu_percent->user);
		printf("%-12s : %6.2f\n", "Kernel CPU", cpu_percent->kernel);
		printf("%-12s : %6.2f\n", "IOWait CPU", cpu_percent->iowait);
		printf("%-12s : %6.2f\n", "Swap CPU", cpu_percent->swap);
		printf("%-12s : %6.2f\n", "Nice CPU", cpu_percent->nice);
		printf("%-12s : %6.2f\n", "Idle CPU", cpu_percent->idle);
		printf("\n");
		fflush(stdout);
#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif
	}
	sg_shutdown();

	return rc;
}
