
#include <stdio.h>
#include "statgrab.h"
#include <stdlib.h>
#ifdef WIN32
//@
#else
#include <unistd.h>
#endif

int run_load_stats_test(int argc, ACE_TCHAR* argv[])
{	
	int rc = 0;

	int delay = 1;
	sg_load_stats *load_stat;

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
	while( (load_stat = sg_get_load_stats()) != NULL){
		++n_loop;
		if ( n_loop > 5 )
			break;

		printf("Load 1 : %5.2f\t Load 5 : %5.2f\t Load 15 : %5.2f\n", load_stat->min1, load_stat->min5, load_stat->min15);
#ifdef WIN32
		::Sleep(delay);
#else
		sleep(delay);
#endif
	}

	return rc;
}
